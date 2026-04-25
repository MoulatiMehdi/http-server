#!/usr/bin/env python3
import os

print("Content-Type: text/html\r")
print("\r")
print("<html><body>")
print("<h1>CGI works</h1>")
for k, v in os.environ.items():
    print(f"<p>{k} = {v}</p>")
print("</body></html>")
