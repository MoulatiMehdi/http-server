#!/bin/bash
# Usage: ./tests/test_basic.sh
# Run with server on localhost:8080 using config/default.conf

BASE="http://localhost:8080"
PASS=0
FAIL=0

check() {
	local desc="$1"
	local expected="$2"
	local actual="$3"
	if echo "$actual" | grep -q "$expected"; then
		echo "[PASS] $desc"
		PASS=$((PASS + 1))
	else
		echo "[FAIL] $desc"
		echo "       expected: $expected"
		echo "       got:      $actual"
		FAIL=$((FAIL + 1))
	fi
}

echo "================================================"
echo " BASIC HTTP TESTS — no CGI"
echo "================================================"

# ─── GET ──────────────────────────────────────────────────────────────────────

echo ""
echo "--- GET ---"

# expect 200, body contains something
R=$(curl -s -o /dev/null -w "%{http_code}" $BASE/)
check "GET / returns 200" "200" "$R"

# existing static file
R=$(curl -s -o /dev/null -w "%{http_code}" $BASE/index.html)
check "GET /index.html returns 200" "200" "$R"

# file that does not exist
R=$(curl -s -o /dev/null -w "%{http_code}" $BASE/doesnotexist.html)
check "GET missing file returns 404" "404" "$R"

# directory with listing enabled
R=$(curl -s -o /dev/null -w "%{http_code}" $BASE/listing/)
check "GET /listing/ with directory_listing on returns 200" "200" "$R"

# directory listing body contains filenames
R=$(curl -s $BASE/listing/)
check "GET /listing/ body contains file1.txt" "file1.txt" "$R"

# directory with listing disabled — expect 403 or redirect to index
R=$(curl -s -o /dev/null -w "%{http_code}" $BASE/no-listing/)
check "GET /no-listing/ with directory_listing off returns 403" "403" "$R"

# redirect
R=$(curl -s -o /dev/null -w "%{http_code}" $BASE/old/)
check "GET /old/ returns 301 redirect" "301" "$R"

# redirect location header points to /
R=$(curl -s -D - -o/dev/null $BASE/old/)
check "GET /old/ Location header is /" "Location" "$R"

# method not allowed — POST-only route
R=$(curl -s -o /dev/null -w "%{http_code}" $BASE/post-only/)
check "GET /post-only/ returns 405" "405" "$R"

# body size limit — send more than 1k to port 7070
R=$(BLA=$(python3 -c "print('x' * 2000)"); \
	BASE="http://localhost:8080"; curl -s -D - -o /dev/null \
	-w "%{http_code}" -X POST --data "$BLA" $BASE/post-only/)
check "POST body exceeding client_max_body_size returns 413" "413" "$R"



# # ─── POST ─────────────────────────────────────────────────────────────────────
#
# echo ""
# echo "--- POST ---"

# # basic POST to upload route
# R=$(curl -s -o /dev/null -w "%{http_code}" \
# 	-X POST \
# 	-F "file=@README.md" \
# 	$BASE/upload)
# 	check "POST /upload returns 201" "201" "$R"

# verify file actually landed on disk
# check "uploaded file exists on disk" "0" "$(test -f www/upload/README.md; echo $?)"
#
# POST to GET-only route returns 405
# R=$(curl -s -o /dev/null -w "%{http_code}" \
# 	-X POST -d "data=test" \
# 	$BASE/)
# 	check "POST / (GET-only route) returns 405" "405" "$R"

# POST with no body
# R=$(curl -s -o /dev/null -w "%{http_code}" \
# 	-X POST \
# 	-F "file=@README.md" \
# 	$BASE/upload)
# 	check "POST /upload with empty body still returns 2xx" "20" "$R"

# ─── DELETE ───────────────────────────────────────────────────────────────────
#
# echo ""
# echo "--- DELETE ---"
#
# # create a file to delete
# echo "temporary file" > www/upload/todelete.txt
#
# # delete it
# R=$(curl -s -o /dev/null -w "%{http_code}" \
# 	-X DELETE \
# 	$BASE/upload/todelete.txt)
# 	check "DELETE existing file returns 204" "204" "$R"
#
# # verify it's gone
# check "deleted file no longer exists" "1" "$(test -f www/upload/todelete.txt; echo $?)"
#
# # delete nonexistent file
# R=$(curl -s -o /dev/null -w "%{http_code}" \
# 	-X DELETE \
# 	$BASE/upload/ghost.txt)
# 	check "DELETE nonexistent file returns 404" "404" "$R"
#
# # delete on route that doesn't allow DELETE
# R=$(curl -s -o /dev/null -w "%{http_code}" \
# 	-X DELETE \
# 	$BASE/listing/file1.txt)
# 	check "DELETE on no-delete route returns 405" "405" "$R"
#
# ─── HEADERS ──────────────────────────────────────────────────────────────────

echo ""
echo "--- RESPONSE HEADERS ---"

# Content-Length must be present on static file
R=$(curl -s -I $BASE/index.html)
check "GET /index.html has Content-Length header" "Content-Length" "$R"

# Connection: close must be present (HTTP/1.0 or explicit)
R=$(curl -s -I $BASE/index.html)
check "GET /index.html has Connection header" "Connection" "$R"

# Content-Type must be present
R=$(curl -s -I $BASE/index.html)
check "GET /index.html has Content-Type header" "Content-Type" "$R"

# ─── SUMMARY ──────────────────────────────────────────────────────────────────

echo ""
echo "================================================"
echo " PASSED: $PASS"
echo " FAILED: $FAIL"
echo "================================================"
