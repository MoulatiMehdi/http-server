#!/bin/bash
# stress.sh — sends N requests to the server, mix of valid/invalid/silent
# Usage: ./tests/stress.sh [N] [HOST] [PORT]
# Example: ./tests/stress.sh 1000 localhost 8080

N=${1:-1000}
HOST=${2:-localhost}
PORT=${3:-8080}
BASE="http://$HOST:$PORT"

PASS=0
FAIL=0
SILENT=0
INVALID=0

# ── helpers ──────────────────────────────────────────────────────────────────

# valid GET request
do_valid() {
    local paths=("/" "/index.html" "/listing/" "/upload/")
    local path=${paths[$((RANDOM % ${#paths[@]}))]}
    local code
    code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "$BASE$path" 2>/dev/null)
    if [ "$code" -ge 200 ] && [ "$code" -lt 500 ]; then
        PASS=$((PASS + 1))
    else
        FAIL=$((FAIL + 1))
        echo "[FAIL] valid GET $path → $code"
    fi
}

# valid POST request
do_valid_post() {
    local code
    code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 \
        -X POST -d "stress=test" \
        "$BASE/post-only/" 2>/dev/null)
    if [ "$code" -ge 200 ] && [ "$code" -lt 500 ]; then
        PASS=$((PASS + 1))
    else
        FAIL=$((FAIL + 1))
        echo "[FAIL] valid POST /post-only/ → $code"
    fi
}

# send a malformed HTTP request via netcat
do_invalid() {
    local payloads=(
        "NOTAMETHOD / HTTP/1.1\r\nHost: $HOST\r\n\r\n"
        "GET\r\n\r\n"
        "GET / HTTP/9.9\r\nHost: $HOST\r\n\r\n"
        "GET / HTTP/1.1\r\n\r\n"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\r\n\r\n"
        "\r\n\r\n"
        "GET /../../../etc/passwd HTTP/1.1\r\nHost: $HOST\r\n\r\n"
        "GET / HTTP/1.1\r\nHost: $HOST\r\nContent-Length: 99999\r\n\r\n"
    )
    local payload=${payloads[$((RANDOM % ${#payloads[@]}))]}
    printf "$payload" | nc -q 1 -w 2 "$HOST" "$PORT" > /dev/null 2>&1
    INVALID=$((INVALID + 1))
}

# connect and immediately close — sends nothing
do_silent() {
    # open TCP connection, send nothing, close after 1s
    (sleep 1) | nc -w 1 "$HOST" "$PORT" > /dev/null 2>&1 &
    SILENT=$((SILENT + 1))
}

# send partial request and hang briefly
do_partial() {
    # sends request line but no headers — server should timeout
    printf "GET / HTTP/1.1\r\n" | nc -w 2 "$HOST" "$PORT" > /dev/null 2>&1
    INVALID=$((INVALID + 1))
}

# send a very large header
do_large_header() {
    local big
    big=$(python3 -c "print('X-Big: ' + 'A' * 8192)")
    printf "GET / HTTP/1.1\r\nHost: $HOST\r\n$big\r\n\r\n" \
        | nc -q 1 -w 2 "$HOST" "$PORT" > /dev/null 2>&1
    INVALID=$((INVALID + 1))
}

# ── main loop ─────────────────────────────────────────────────────────────────

echo "========================================"
echo " stress test — $N requests"
echo " target: $BASE"
echo "========================================"
echo ""

for i in $(seq 1 $N); do
    # distribution:
    #   40% valid GET
    #   15% valid POST
    #   20% malformed/invalid
    #   10% silent (connect + nothing)
    #   10% partial request
    #    5% large header

    r=$((RANDOM % 100))

    if   [ $r -lt 40 ]; then do_valid
    elif [ $r -lt 55 ]; then do_valid_post
    elif [ $r -lt 75 ]; then do_invalid
    elif [ $r -lt 85 ]; then do_silent
    elif [ $r -lt 95 ]; then do_partial
    else                     do_large_header
    fi

    # progress every 100 requests
    if [ $((i % 100)) -eq 0 ]; then
        echo "  progress: $i / $N"
    fi
done

# wait for background silent connections to finish
wait

echo ""
echo "========================================"
echo " RESULTS"
echo "========================================"
echo " valid responses (2xx/3xx/4xx) : $PASS"
echo " valid responses failed        : $FAIL"
echo " malformed/invalid sent        : $INVALID"
echo " silent connections            : $SILENT"
echo " total                         : $N"
echo ""
if [ $FAIL -eq 0 ]; then
    echo " server stayed alive — all valid requests handled"
else
    echo " $FAIL valid requests got unexpected responses"
fi
echo "========================================"
