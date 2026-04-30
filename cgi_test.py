#!/usr/bin/env python3
# CGI Test Script — webserv validation
# Each test is triggered by ?test=<name> query param
# Run with: curl "http://localhost:8080/cgi-bin/cgi_test.py?test=env"
#
# Available tests:
#   ?test=basic
#   ?test=env
#   ?test=query&foo=bar&x=1&x=2
#   ?test=post          (POST with body)
#   ?test=large         (POST with large body)
#   ?test=headers
#   ?test=status&code=404
#   ?test=pathinfo      (set PATH_INFO on your server config)
#   ?test=delay&secs=5
#   ?test=stderr
#   ?test=badheader

import os
import sys
import time

# ─── helpers ──────────────────────────────────────────────────────────────────

def parse_query_string(qs):
    """
    Manual query string parser.
    Returns dict of key -> list of values (handles repeated keys).
    """
    result = {}
    if not qs:
        return result
    for pair in qs.split('&'):
        if '=' in pair:
            k, v = pair.split('=', 1)
        else:
            k, v = pair, ''
        result.setdefault(k, []).append(v)
    return result

def send_headers(status="200 OK", content_type="text/plain", extra=None):
    sys.stdout.write("Status: {}\r\n".format(status))
    sys.stdout.write("Content-Type: {}\r\n".format(content_type))
    if extra:
        for k, v in extra.items():
            sys.stdout.write("{}: {}\r\n".format(k, v))
    sys.stdout.write("\r\n")
    sys.stdout.flush()

def section(name):
    return "\n=== {} ===\n".format(name)

# ─── test handlers ────────────────────────────────────────────────────────────

def test_basic():
    """
    SERVER SIDE: verifies that the server correctly forwards CGI stdout
    to the client, and that Content-Type header is parsed and forwarded.
    """
    send_headers()
    sys.stdout.write(section("BASIC OUTPUT TEST"))
    sys.stdout.write("hello from CGI\n")
    sys.stdout.write("if you see this, stdout pipe and header parsing work\n")

def test_env():
    """
    SERVER SIDE: verifies that the server sets all mandatory RFC 3875
    environment variables before execve(). Missing variables here mean
    your env setup in Cgi constructor is incomplete.
    """
    mandatory = [
        "REQUEST_METHOD",
        "QUERY_STRING",
        "SCRIPT_NAME",
        "SERVER_NAME",
        "SERVER_PORT",
        "SERVER_PROTOCOL",
        "GATEWAY_INTERFACE",
    ]
    optional = [
        "CONTENT_TYPE",
        "CONTENT_LENGTH",
        "PATH_INFO",
        "PATH_TRANSLATED",
        "SCRIPT_FILENAME",
        "HTTP_HOST",
        "HTTP_USER_AGENT",
        "HTTP_ACCEPT",
        "REMOTE_ADDR",
        "REMOTE_HOST",
    ]

    send_headers()
    sys.stdout.write(section("ENVIRONMENT VARIABLES TEST"))

    sys.stdout.write("--- MANDATORY ---\n")
    for var in mandatory:
        val = os.environ.get(var)
        if val is not None:
            sys.stdout.write("[PRESENT] {}={}\n".format(var, val))
        else:
            sys.stdout.write("[MISSING] {}\n".format(var))

    sys.stdout.write("\n--- OPTIONAL ---\n")
    for var in optional:
        val = os.environ.get(var)
        if val is not None:
            sys.stdout.write("[PRESENT] {}={}\n".format(var, val))
        else:
            sys.stdout.write("[ABSENT]  {}\n".format(var))

    sys.stdout.write("\n--- ALL ENV ---\n")
    for k, v in sorted(os.environ.items()):
        sys.stdout.write("{}={}\n".format(k, v))

def test_query():
    """
    SERVER SIDE: verifies QUERY_STRING is passed correctly and completely.
    Test with: ?test=query&foo=bar&x=1&x=2&empty=&novalue
    Edge cases: empty value, repeated key, key with no '='.
    """
    qs = os.environ.get("QUERY_STRING", "")
    parsed = parse_query_string(qs)

    send_headers()
    sys.stdout.write(section("QUERY STRING PARSING TEST"))
    sys.stdout.write("raw QUERY_STRING: {}\n".format(qs))
    sys.stdout.write("parsed:\n")
    for k, vals in parsed.items():
        for v in vals:
            sys.stdout.write("  [{}] = [{}]{}\n".format(
                k, v, " (empty value)" if v == '' else ""
            ))
    if not parsed:
        sys.stdout.write("  (empty — no query params)\n")

def test_post():
    """
    SERVER SIDE: verifies that:
    - CONTENT_LENGTH is set correctly
    - stdin pipe is connected and readable
    - server does not truncate or add bytes to body
    Read exactly CONTENT_LENGTH bytes — reading more would block forever
    if server closes stdin at the right time.
    """
    send_headers()
    sys.stdout.write(section("POST BODY TEST"))

    method = os.environ.get("REQUEST_METHOD", "")
    sys.stdout.write("REQUEST_METHOD: {}\n".format(method))

    if method != "POST":
        sys.stdout.write("not a POST request — send with: curl -X POST -d 'key=value' ...\n")
        return

    length_str = os.environ.get("CONTENT_LENGTH", "")
    if not length_str:
        sys.stdout.write("CONTENT_LENGTH missing — server bug\n")
        return

    try:
        length = int(length_str)
    except ValueError:
        sys.stdout.write("CONTENT_LENGTH not an integer: {}\n".format(length_str))
        return

    sys.stdout.write("CONTENT_LENGTH: {}\n".format(length))
    body = sys.stdin.read(length)
    sys.stdout.write("bytes read: {}\n".format(len(body)))
    sys.stdout.write("body match: {}\n".format("OK" if len(body) == length else "MISMATCH"))
    sys.stdout.write("raw body:\n{}\n".format(body))

def test_large():
    """
    SERVER SIDE: verifies that the server does not truncate large POST bodies,
    that the stdin pipe handles buffering correctly, and that CONTENT_LENGTH
    is accurate for large payloads.
    Send with: dd if=/dev/urandom bs=1M count=1 | base64 | curl -X POST --data-binary @- ...
    """
    send_headers()
    sys.stdout.write(section("LARGE BODY TEST"))

    method = os.environ.get("REQUEST_METHOD", "")
    if method != "POST":
        sys.stdout.write("send a large POST body to trigger this test\n")
        sys.stdout.write("example: dd if=/dev/zero bs=100K count=1 | curl -X POST --data-binary @- '...?test=large'\n")
        return

    length_str = os.environ.get("CONTENT_LENGTH", "0")
    try:
        length = int(length_str)
    except ValueError:
        length = 0

    body = sys.stdin.read(length)
    received = len(body)

    sys.stdout.write("CONTENT_LENGTH declared: {}\n".format(length))
    sys.stdout.write("bytes actually received: {}\n".format(received))
    sys.stdout.write("match: {}\n".format("OK" if received == length else "MISMATCH — server truncated or padded body"))

def test_headers():
    """
    SERVER SIDE: verifies that the server correctly forwards multiple
    response headers from CGI output to the client without dropping,
    merging, or mangling them.
    Check with: curl -v ...?test=headers
    """
    send_headers(
        status="200 OK",
        content_type="text/plain",
        extra={
            "X-Custom-Header": "webserv-test",
            "X-Another-Header": "value123",
            "Cache-Control": "no-cache",
        }
    )
    sys.stdout.write(section("HEADER VALIDATION TEST"))
    sys.stdout.write("check curl -v output for X-Custom-Header and X-Another-Header\n")
    sys.stdout.write("all three extra headers must appear in the response\n")

def test_status():
    """
    SERVER SIDE: verifies that the server correctly forwards non-200
    status codes from the CGI Status header to the client.
    Test with: ?test=status&code=404 or &code=500
    If server ignores Status header and always sends 200, this test will show it.
    """
    qs = os.environ.get("QUERY_STRING", "")
    params = parse_query_string(qs)
    codes = params.get("code", ["200"])
    code = codes[0]

    status_map = {
        "200": "200 OK",
        "201": "201 Created",
        "301": "301 Moved Permanently",
        "400": "400 Bad Request",
        "403": "403 Forbidden",
        "404": "404 Not Found",
        "500": "500 Internal Server Error",
    }
    status_line = status_map.get(code, "200 OK")

    send_headers(status=status_line)
    sys.stdout.write(section("STATUS CODE TEST"))
    sys.stdout.write("requested code: {}\n".format(code))
    sys.stdout.write("sent Status header: {}\n".format(status_line))
    sys.stdout.write("if client received a different code, server is ignoring CGI Status header\n")

def test_pathinfo():
    """
    SERVER SIDE: verifies that PATH_INFO and SCRIPT_NAME are split correctly
    by the server. Configure your server so that a URL like:
    /cgi-bin/cgi_test.py/extra/path?test=pathinfo
    sets PATH_INFO=/extra/path and SCRIPT_NAME=/cgi-bin/cgi_test.py
    """
    send_headers()
    sys.stdout.write(section("PATH_INFO TEST"))
    sys.stdout.write("SCRIPT_NAME : {}\n".format(os.environ.get("SCRIPT_NAME", "(not set)")))
    sys.stdout.write("PATH_INFO   : {}\n".format(os.environ.get("PATH_INFO", "(not set)")))
    sys.stdout.write("expected SCRIPT_NAME=/cgi-bin/cgi_test.py\n")
    sys.stdout.write("expected PATH_INFO=/extra/path (if URL had extra path component)\n")

def test_delay():
    """
    SERVER SIDE: verifies that the server's CGI timeout kills hanging scripts.
    Send with: ?test=delay&secs=10
    If your CGI_TIMEOUT_SEC is 5, the server must kill this process and return
    a 504/500 before the sleep completes.
    If the client hangs forever, your timeout is not working.
    """
    qs = os.environ.get("QUERY_STRING", "")
    params = parse_query_string(qs)
    secs_list = params.get("secs", ["3"])
    try:
        secs = int(secs_list[0])
    except ValueError:
        secs = 3

    # cap at 60 so we don't accidentally hang forever during testing
    secs = min(secs, 60)

    sys.stderr.write("CGI delay test: sleeping for {} seconds\n".format(secs))
    sys.stderr.flush()

    time.sleep(secs)

    # if we reach here, the server did not kill us within the timeout
    send_headers()
    sys.stdout.write(section("DELAY / TIMEOUT TEST"))
    sys.stdout.write("slept for {} seconds\n".format(secs))
    sys.stdout.write("if you see this and secs > CGI_TIMEOUT_SEC, server timeout is broken\n")

def test_stderr():
    """
    SERVER SIDE: verifies that the server does not crash or block when the
    CGI writes to stderr. stderr from CGI should either be discarded or
    logged — it must not be forwarded to the client or block the pipe.
    """
    sys.stderr.write("=== STDERR TEST: line 1\n")
    sys.stderr.write("=== STDERR TEST: line 2\n")
    sys.stderr.write("=== STDERR TEST: this should go to server logs, not client\n")
    sys.stderr.flush()

    send_headers()
    sys.stdout.write(section("STDERR TEST"))
    sys.stdout.write("3 lines written to stderr\n")
    sys.stdout.write("they must NOT appear in this response body\n")
    sys.stdout.write("server must not block waiting for stderr to be read\n")

def test_badheader():
    """
    SERVER SIDE: verifies robustness of the server's CGI response parser.
    Sends malformed/edge-case headers to expose parser bugs:
    - header with no value
    - header with extra whitespace
    - duplicate Content-Type
    - empty header line before end of headers
    The server must not crash, hang, or corrupt the response.
    """
    # intentionally bypass send_headers() to control raw output
    sys.stdout.write("Status: 200 OK\r\n")
    sys.stdout.write("Content-Type: text/plain\r\n")
    sys.stdout.write("X-Empty-Value:\r\n")           # header with no value
    sys.stdout.write("X-Extra-Space:   padded   \r\n")  # whitespace padding
    sys.stdout.write("Content-Type: text/html\r\n")  # duplicate header
    sys.stdout.write("\r\n")                          # end of headers
    sys.stdout.write(section("INVALID HEADER TEST"))
    sys.stdout.write("headers sent:\n")
    sys.stdout.write("  X-Empty-Value: (empty)\n")
    sys.stdout.write("  X-Extra-Space: '   padded   '\n")
    sys.stdout.write("  Content-Type: sent twice\n")
    sys.stdout.write("server must not crash and must forward the body correctly\n")

# ─── dispatch ─────────────────────────────────────────────────────────────────

def main():
    qs = os.environ.get("QUERY_STRING", "")
    params = parse_query_string(qs)
    test = params.get("test", ["basic"])[0]

    dispatch = {
        "basic":     test_basic,
        "env":       test_env,
        "query":     test_query,
        "post":      test_post,
        "large":     test_large,
        "headers":   test_headers,
        "status":    test_status,
        "pathinfo":  test_pathinfo,
        "delay":     test_delay,
        "stderr":    test_stderr,
        "badheader": test_badheader,
    }

    handler = dispatch.get(test)
    if handler is None:
        send_headers(status="400 Bad Request")
        sys.stdout.write("unknown test: {}\n".format(test))
        sys.stdout.write("available: {}\n".format(", ".join(sorted(dispatch.keys()))))
        return

    try:
        handler()
    except Exception as e:
        # if headers not sent yet this will produce a valid error response
        sys.stdout.write("Status: 500 Internal Server Error\r\n")
        sys.stdout.write("Content-Type: text/plain\r\n")
        sys.stdout.write("\r\n")
        sys.stdout.write("CGI script exception: {}\n".format(str(e)))

    sys.stdout.flush()

if __name__ == "__main__":
    main()
