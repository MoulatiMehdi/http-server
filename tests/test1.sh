#!/bin/bash

# ============================================================
#  test_webserv.sh — curl test suite for webserv (HTTP/1.1)
#  Usage: ./test_webserv.sh [host] [port]
#  Defaults: localhost 8080
# ============================================================

HOST="${1:-localhost}"
PORT="${2:-8080}"
BASE="http://$HOST:$PORT"

# ---------- helpers -----------------------------------------

PASS=0
FAIL=0
TOTAL=0

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
RESET='\033[0m'

section() {
    echo ""
    echo -e "${CYAN}══════════════════════════════════════════${RESET}"
    echo -e "${CYAN}  $1${RESET}"
    echo -e "${CYAN}══════════════════════════════════════════${RESET}"
}

# expect_status <label> <expected_code> <actual_code>
expect_status() {
    local label="$1"
    local expected="$2"
    local actual="$3"
    TOTAL=$((TOTAL + 1))
    if [ "$actual" = "$expected" ]; then
        echo -e "  ${GREEN}[PASS]${RESET} $label (HTTP $actual)"
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}[FAIL]${RESET} $label — expected HTTP $expected, got HTTP $actual"
        FAIL=$((FAIL + 1))
    fi
}

# expect_body <label> <pattern> <body>
expect_body() {
    local label="$1"
    local pattern="$2"
    local body="$3"
    TOTAL=$((TOTAL + 1))
    if echo "$body" | grep -q "$pattern"; then
        echo -e "  ${GREEN}[PASS]${RESET} $label (found: '$pattern')"
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}[FAIL]${RESET} $label — pattern '$pattern' not found in body"
        echo -e "         Body snippet: $(echo "$body" | head -5)"
        FAIL=$((FAIL + 1))
    fi
}

# expect_header <label> <pattern> <headers>
expect_header() {
    local label="$1"
    local pattern="$2"
    local headers="$3"
    TOTAL=$((TOTAL + 1))
    if echo "$headers" | grep -qi "$pattern"; then
        echo -e "  ${GREEN}[PASS]${RESET} $label (header: '$pattern')"
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}[FAIL]${RESET} $label — header '$pattern' not found"
        echo -e "         Headers: $(echo "$headers" | head -10)"
        FAIL=$((FAIL + 1))
    fi
}

# raw curl: returns "<status>|||<headers>|||<body>"
do_req() {
    # $@ = extra curl flags + URL (URL must be last)
    curl -s -o /tmp/ws_body \
         -D /tmp/ws_headers \
         -w "%{http_code}" \
         "$@"
}

# ============================================================
#  0. SANITY — server is up
# ============================================================

section "0. Sanity — server reachable"

code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 3 "$BASE/")
if [ "$code" = "000" ]; then
    echo -e "${RED}  Server not reachable at $BASE — aborting.${RESET}"
    exit 1
fi
echo -e "  ${GREEN}Server is up${RESET} at $BASE"

# ============================================================
#  1. STATIC FILE SERVING (GET)
# ============================================================

section "1. Static file serving — GET"

# 1a. Root index
code=$(do_req "$BASE/")
body=$(cat /tmp/ws_body)
expect_status "GET / returns 200" "200" "$code"
expect_body   "GET / body is HTML" "<html\|<!DOCTYPE\|<body" "$body"

# 1b. Explicit index.html
code=$(do_req "$BASE/index.html")
expect_status "GET /index.html returns 200" "200" "$code"

# 1c. File that doesn't exist → 404
code=$(do_req "$BASE/does_not_exist_xyz.html")
expect_status "GET missing file → 404" "404" "$code"

# 1d. Content-Type header for HTML
code=$(do_req "$BASE/index.html")
headers=$(cat /tmp/ws_headers)
expect_header "Content-Type: text/html present" "text/html" "$headers"

# ============================================================
#  2. DIRECTORY LISTING
# ============================================================

section "2. Directory listing — GET /listing"

code=$(do_req "$BASE/listing/")
body=$(cat /tmp/ws_body)
expect_status "GET /listing/ returns 200" "200" "$code"
# Your serveDir() builds an HTML page — it should contain anchor tags
expect_body "Listing body contains <a href" '<a href' "$body"

# ============================================================
#  3. FILE UPLOAD (POST)
# ============================================================

section "3. File upload — POST /upload"

UPLOAD_FILE="/tmp/ws_test_upload_$$.txt"
echo "webserv upload test — $(date)" > "$UPLOAD_FILE"

# 3a. Valid upload
code=$(do_req -X POST \
    -F "file=@$UPLOAD_FILE;type=text/plain" \
    "$BASE/upload")
expect_status "POST /upload file → 201" "201" "$code"

# 3b. Upload text/plain directly (raw body)
code=$(do_req -X POST \
    -H "Content-Type: text/plain" \
    --data-binary "hello from curl raw body" \
    "$BASE/upload")
expect_status "POST /upload raw text → 201" "201" "$code"

# 3c. Body exceeds client_max_body_size (10m → send 11MB)
BIG_FILE="/tmp/ws_big_$$.bin"
dd if=/dev/urandom of="$BIG_FILE" bs=1M count=11 2>/dev/null
code=$(do_req -X POST \
    -H "Content-Type: application/octet-stream" \
    --data-binary "@$BIG_FILE" \
    "$BASE/upload")
expect_status "POST oversized body → 413" "413" "$code"
rm -f "$BIG_FILE" "$UPLOAD_FILE"

# ============================================================
#  4. DELETE
# ============================================================

section "4. DELETE — /upload"

# First upload a file so we have something to delete
DELNAME="ws_delete_test_$$.txt"
DELPATH="/tmp/$DELNAME"
echo "delete me" > "$DELPATH"
curl -s -o /dev/null \
     -X POST \
     -F "file=@$DELPATH;filename=$DELNAME" \
     "$BASE/upload"

# Now delete it (your upload_store is ./www/upload/)
code=$(do_req -X DELETE "$BASE/upload/$DELNAME")
# 200 or 204 are both valid for successful DELETE
if [ "$code" = "200" ] || [ "$code" = "204" ] || [ "$code" = "204" ]; then
    echo -e "  ${GREEN}[PASS]${RESET} DELETE existing file → HTTP $code"
    PASS=$((PASS + 1))
else
    echo -e "  ${RED}[FAIL]${RESET} DELETE existing file — expected 200/204, got $code"
    FAIL=$((FAIL + 1))
fi
TOTAL=$((TOTAL + 1))
rm -f "$DELPATH"

# Delete non-existent file → 404
code=$(do_req -X DELETE "$BASE/upload/ghost_file_xyz.txt")
expect_status "DELETE non-existent file → 404" "404" "$code"

# ============================================================
#  5. METHOD NOT ALLOWED
# ============================================================

section "5. Method Not Allowed — 405"

# GET / only allows GET DELETE per config; POST should be 405
code=$(do_req -X POST "$BASE/")
expect_status "POST / (not in allowed methods) → 405" "405" "$code"

# POST /listing — only GET is allowed
code=$(do_req -X POST "$BASE/listing/")
expect_status "POST /listing → 405" "405" "$code"

# ============================================================
#  6. REDIRECT
# ============================================================

section "6. Redirect — GET /old"

# -L follows redirects; check without -L to see the 3xx
code=$(do_req "$BASE/old")
expect_status "GET /old returns 3xx" "301" "$code"
headers=$(cat /tmp/ws_headers)
expect_header "Location header points to /" "Location:" "$headers"

# With -L it should follow and land on 200
code=$(curl -s -o /dev/null -w "%{http_code}" -L "$BASE/old")
expect_status "GET /old followed → 200" "200" "$code"

# ============================================================
#  7. CGI
# ============================================================

section "7. CGI — GET /cgi-bin"

# 7a. Basic CGI GET (your cgi-bin needs a .py script)
code=$(do_req "$BASE/cgi-bin/hello.py")
body=$(cat /tmp/ws_body)
# Accept 200 or 404 (if no script exists in test env)
if [ "$code" = "200" ]; then
    expect_status "GET /cgi-bin/hello.py → 200" "200" "$code"
    expect_body   "CGI body not empty" "." "$body"
elif [ "$code" = "404" ]; then
    echo -e "  ${YELLOW}[SKIP]${RESET} CGI hello.py not found (place a test script in www/cgi-bin/)"
    TOTAL=$((TOTAL + 1))  # count skip as neutral
else
    expect_status "GET /cgi-bin/hello.py → 200 or 404" "200" "$code"
fi

# 7b. CGI with query string
code=$(do_req "$BASE/cgi-bin/hello.py?name=webserv&test=1")
if [ "$code" = "200" ]; then
    expect_status "CGI GET with query string → 200" "200" "$code"
elif [ "$code" = "404" ]; then
    echo -e "  ${YELLOW}[SKIP]${RESET} CGI script missing — skipping query string test"
    TOTAL=$((TOTAL + 1))
else
    expect_status "CGI GET with query string → 200" "200" "$code"
fi

# 7c. CGI POST with body
code=$(do_req -X POST \
    -H "Content-Type: application/x-www-form-urlencoded" \
    --data "field=hello&value=world" \
    "$BASE/cgi-bin/hello.py")
if [ "$code" = "200" ]; then
    expect_status "CGI POST with body → 200" "200" "$code"
elif [ "$code" = "404" ]; then
    echo -e "  ${YELLOW}[SKIP]${RESET} CGI script missing — skipping POST test"
    TOTAL=$((TOTAL + 1))
else
    expect_status "CGI POST → 200" "200" "$code"
fi

# ============================================================
#  8. SESSION MANAGEMENT
# ============================================================

section "8. Sessions — GET /session"

# 8a. First visit — no cookie → server creates session + sets Set-Cookie
code=$(do_req -c /tmp/ws_cookies_$$.txt "$BASE/session")
headers=$(cat /tmp/ws_headers)
body=$(cat /tmp/ws_body)
expect_status "GET /session (no cookie) → 200" "200" "$code"
expect_header "Set-Cookie: sid= sent on first visit" "Set-Cookie.*sid=" "$headers"
expect_body   "Session body shows visit count" "visit\|Visit\|session\|Session" "$body"

# 8b. Second visit — send cookie back → visits should increment
code=$(do_req -b /tmp/ws_cookies_$$.txt "$BASE/session")
body=$(cat /tmp/ws_body)
expect_status "GET /session (with cookie) → 200" "200" "$code"
# visits should be 2 now; check body contains "2" somewhere
expect_body "Second visit body contains '2'" "2" "$body"

# 8c. Confirm no new Set-Cookie on second visit (session already exists)
headers=$(cat /tmp/ws_headers)
# This is advisory — some implementations always resend; just log
if echo "$headers" | grep -qi "Set-Cookie.*sid="; then
    echo -e "  ${YELLOW}[INFO]${RESET} Server re-sent Set-Cookie on second visit (not wrong, just noted)"
fi

rm -f /tmp/ws_cookies_$$.txt

# ============================================================
#  9. CUSTOM ERROR PAGES
# ============================================================

section "9. Custom error pages"

# 9a. 404 — your config maps: error_page 404 ./www/errors/404.html
code=$(do_req "$BASE/nowhere_land_xyz")
body=$(cat /tmp/ws_body)
expect_status "Non-existent path → 404" "404" "$code"
# Should be HTML (custom page), not empty
expect_body "404 body is HTML" "<html\|<!DOCTYPE\|<body\|404" "$body"

# ============================================================
#  10. PERSISTENT CONNECTION BEHAVIOR
# ============================================================

section "10. Connection: close header"

# Your finalizeResponse() always sets Connection: close
code=$(do_req "$BASE/")
headers=$(cat /tmp/ws_headers)
expect_header "Response includes Connection: close" "Connection: close" "$headers"

# ============================================================
#  11. CONTENT-LENGTH
# ============================================================

section "11. Content-Length"

code=$(do_req "$BASE/index.html")
headers=$(cat /tmp/ws_headers)
expect_header "Response includes Content-Length" "Content-Length:" "$headers"

# Verify Content-Length matches actual body size
cl=$(grep -i "Content-Length:" /tmp/ws_headers | tr -d '\r' | awk '{print $2}')
actual=$(wc -c < /tmp/ws_body)
TOTAL=$((TOTAL + 1))
if [ "$cl" = "$actual" ]; then
    echo -e "  ${GREEN}[PASS]${RESET} Content-Length ($cl) matches actual body size ($actual)"
    PASS=$((PASS + 1))
else
    echo -e "  ${RED}[FAIL]${RESET} Content-Length mismatch: header=$cl, body=$actual"
    FAIL=$((FAIL + 1))
fi

# ============================================================
#  12. EDGE CASES
# ============================================================

section "12. Edge cases"

# 12a. Double slash in path
code=$(do_req "$BASE//index.html")
expect_status "Double slash path handled (200 or 400)" "200" "$code"

# 12b. URL-encoded path
code=$(do_req "$BASE/index%2Ehtml")
if [ "$code" = "200" ] || [ "$code" = "404" ]; then
    echo -e "  ${GREEN}[PASS]${RESET} URL-encoded path returns $code (server handles decode)"
    PASS=$((PASS + 1))
else
    echo -e "  ${RED}[FAIL]${RESET} URL-encoded path returned $code"
    FAIL=$((FAIL + 1))
fi
TOTAL=$((TOTAL + 1))

# 12c. Path traversal attempt — must NOT escape root
code=$(do_req "$BASE/../../../etc/passwd")
body=$(cat /tmp/ws_body)
TOTAL=$((TOTAL + 1))
if echo "$body" | grep -q "root:"; then
    echo -e "  ${RED}[FAIL]${RESET} PATH TRAVERSAL VULNERABILITY — /etc/passwd served!"
    FAIL=$((FAIL + 1))
else
    echo -e "  ${GREEN}[PASS]${RESET} Path traversal blocked (no /etc/passwd in body)"
    PASS=$((PASS + 1))
fi

# 12d. HEAD request — body must be empty, headers must exist
code=$(do_req -X HEAD "$BASE/index.html")
body=$(cat /tmp/ws_body)
headers=$(cat /tmp/ws_headers)
expect_status "HEAD /index.html returns 200" "200" "$code"
TOTAL=$((TOTAL + 1))
if [ -z "$body" ]; then
    echo -e "  ${GREEN}[PASS]${RESET} HEAD response body is empty"
    PASS=$((PASS + 1))
else
    echo -e "  ${RED}[FAIL]${RESET} HEAD response has a body (${#body} bytes) — RFC violation"
    FAIL=$((FAIL + 1))
fi

# 12e. Empty POST body to upload
code=$(do_req -X POST \
    -H "Content-Type: text/plain" \
    -H "Content-Length: 0" \
    --data "" \
    "$BASE/upload")
# Accept 201, 400, or 204 — just must not crash (000 = crash/timeout)
if [ "$code" = "000" ]; then
    echo -e "  ${RED}[FAIL]${RESET} Empty POST — server crashed or timed out"
    FAIL=$((FAIL + 1))
else
    echo -e "  ${GREEN}[PASS]${RESET} Empty POST handled gracefully (HTTP $code)"
    PASS=$((PASS + 1))
fi
TOTAL=$((TOTAL + 1))

# ============================================================
#  13. STRESS — concurrent connections
# ============================================================

section "13. Concurrent requests (basic)"

echo "  Firing 20 parallel GET / requests..."
STRESS_FAIL=0
for i in $(seq 1 20); do
    curl -s -o /dev/null -w "%{http_code}" "$BASE/" &
done
wait

# Just verify server is still alive after the burst
code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "$BASE/")
TOTAL=$((TOTAL + 1))
if [ "$code" = "200" ]; then
    echo -e "  ${GREEN}[PASS]${RESET} Server still alive after 20 concurrent requests"
    PASS=$((PASS + 1))
else
    echo -e "  ${RED}[FAIL]${RESET} Server unresponsive after concurrent load (HTTP $code)"
    FAIL=$((FAIL + 1))
fi

# ============================================================
#  SUMMARY
# ============================================================

echo ""
echo -e "${CYAN}══════════════════════════════════════════${RESET}"
echo -e "${CYAN}  RESULTS${RESET}"
echo -e "${CYAN}══════════════════════════════════════════${RESET}"
echo -e "  Total : $TOTAL"
echo -e "  ${GREEN}Passed: $PASS${RESET}"
if [ "$FAIL" -gt 0 ]; then
    echo -e "  ${RED}Failed: $FAIL${RESET}"
else
    echo -e "  ${GREEN}Failed: $FAIL${RESET}"
fi
echo ""

rm -rf www/*.txt
exit $FAIL
