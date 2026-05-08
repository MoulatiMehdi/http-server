#!/usr/bin/env python3
import os, sys

length = int(os.environ.get("CONTENT_LENGTH", 0))
body = sys.stdin.read(length) if length > 0 else ""

print("Content-Type: text/plain\r")
print(f"Content-Length: {len(body)}\r")
print("\r")
print(body)
