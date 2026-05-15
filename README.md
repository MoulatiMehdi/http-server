*This project has been created as part of the 42 curriculum by yel-guad, mmoulati , ihajji.*

**IN THE NAME OF ALLAH., THE MOST MERCIFUL, THE MOST GRACIOUS.**

# webserv

## Description

`webserv` is a lightweight HTTP/1.1 web server written in C++98.

The goal of this project is to understand how web servers work internally by implementing the core parts of the HTTP protocol from scratch.

The server handles : 
- client connections
- parses HTTP requests
- serves static files
- executes CGI scripts
- supports file uploads
- manages multiple clients simultaneously using non-blocking I/O.


This project was developed according to the constraints and requirements defined in the 42 `webserv` subject.

Main features include:

- Non-blocking I/O using `epoll`
- HTTP request/response parsing
- Static file serving
- Multiple routes and server blocks
- File uploads
- DELETE method support
- CGI execution
- Autoindex generation
- HTTP redirections
- Custom error pages
- Multiple port listening
- Cookie support (bonus)

---

# Features

## HTTP Methods

Supported methods:

- `GET`
- `POST`
- `DELETE`

---

## Configuration System

The server uses a configuration file inspired by NGINX syntax.

Example:

```nginx
server {
    listen 8080;

    root ./www/;
    error_pages 400 ./www/errors/400.html;
    # client_max_body_size 1m;

    location / {
        allowed_methods GET POST DELETE;
        index index.html;
        autoindex on;
        return 301 /;
    }

}
```
### Server directives

    ### listen
        Syntax: `listen <port>;` or `listen <host>:<port>;`  
        Params: port number, optional host/IP.  
        Example: `listen 8080;`

    ### root
        Syntax: `root <absolute_path>;`  
        Params: absolute directory path.  
        Example: `root /var/www/site;`

    ### index
        Syntax: `index <file> [file...];`  
        Params: one or more default index filenames.  
        Example: `index index.html index.htm;`

    ### client_max_body_size
        Syntax: `client_max_body_size <size>;`  
        Params: max request body size in bytes.  
        Example: `client_max_body_size 1048576;`

    ### error_page
        Syntax: `error_page <status_code> <path>;`  
        Params: HTTP status code + error page path.  
        Example: `error_page 404 /errors/404.html;`

---

## Location directives

    ### allowed_methods
        Syntax: `allowed_methods <method> [method...];`  
        Params: allowed methods: `GET`, `POST`, `DELETE`.  
        Example: `allowed_methods GET POST;`

    ### return
        Syntax: `return <status_code> <url>;`  
        Params: redirect status code + target URL/path.  
        Example: `return 301 /new-page;`

    ### root
        Syntax: `root <path>;`  
        Params: directory path, overrides server root.  
        Example: `root /var/www/images;`

    ### index
        Syntax: `index <file> [file...];`  
        Params: one or more default index filenames.  
        Example: `index index.html;`

    ### autoindex
        Syntax: `autoindex <on|off>;`  
        Params: on/off directory listing.  
        Example: `autoindex on;`

    ### client_max_body_size
        Syntax: `client_max_body_size <size>;`  
        Params: max request body size in bytes, overrides server value.  
        Example: `client_max_body_size 2097152;`

    ### cgi
        Syntax: `cgi <extension> <executable_path>;`  
        Params: file extension + CGI executable/interpreter path.  
        Example: `cgi .py /usr/bin/python3;`
---

## CGI

The server supports CGI execution based on file extensions.

Example:

```nginx
cgi .py /usr/bin/python3;
```

Request:

```text
/cgi-bin/script.py
```

Executed command:

```bash
/usr/bin/python3 script.py
```

---

## Upload Support

The server supports file uploads using `multipart/form-data` , `text/plain`.

Example:

```html
<form action="/upload/" method="POST" enctype="multipart/form-data">
    <input type="file" name="file">
    <button type="submit">Upload</button>
</form>
```

---

## Autoindex

When enabled, the server automatically generates directory listings if no index file exists.

---

## Redirections

HTTP redirections are supported:

```nginx
location /old/ {
    return 301 /new/;
}
```

---

# Instructions

## Requirements

- Linux
- `c++`
- `make`

---

## Build

Compile the project:

```bash
make
```

Rebuild:

```bash
make re
```

Clean object files:

```bash
make clean
```

Remove all generated files:

```bash
make fclean
```

---

## Run

Run with configuration file:

```bash
./webserv config/default.conf
```

Run using default configuration:

```bash
./webserv
```

---

# Testing

## Browser

Open:

```text
http://localhost:8080
```

---

## GET Request

```bash
curl http://localhost:8080/
```

---

## Upload File

```bash
curl -F "file=@main.cpp" http://localhost:8080/upload/
```

---

## DELETE File

```bash
curl -X DELETE http://localhost:8080/files/test.txt
```

---

## CGI

```bash
curl http://localhost:8080/cgi-bin/hello.py
```

---

### Main Components

- **Config** → Stores parsed configuration data
- **Server** → Manages sockets and connections
- **HTTP** → Handles request/response logic
- **Router** → Resolves routes and target resources
- **CGI** → Executes CGI scripts

---

# AI Usage

AI tools were used during the project for:

- understanding HTTP protocol behavior
- comparing behaviors with NGINX
- explaining low-level networking concepts
- debugging ideas
- generating testing scenarios
- clarifying RFC-related behavior

All generated explanations and suggestions were reviewed, tested, and adapted manually before integration into the project.

---

## Useful References

- (Hypertext Transfer Protocol -- HTTP/1.0)[https://www.rfc-editor.org/rfc/rfc1945.html]
- (The Common Gateway Interface (CGI) Version 1.1)[https://datatracker.ietf.org/doc/html/rfc3875]
- (Linux Programming Interface)[https://broman.dev/download/The%20Linux%20Programming%20Interface.pdf]
- (Linux Manual Page)[https://man7.org/linux/man-pages/]

---

# Allowed Functions

This project follows the restrictions imposed by the 42 subject and only uses the allowed system calls and functions.

---

# Authors

- yel-guad
- mmoulati 
- ihajji
