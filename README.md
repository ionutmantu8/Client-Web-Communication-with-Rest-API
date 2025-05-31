# Tema 4 - Client Web.Communication with REST API

## Author
Mantu Ionut – 323CA

## Project Summary
This C++ command‑line application serves as a client for interacting with a Movie Library REST API over raw TCP sockets. It implements both administrator and regular user workflows, offering functionality for:

- Admin authentication and user management (create, list, delete users).
- User authentication and JWT‑based access to the movie library.
- CRUD operations on movies (`GET`, `POST`, `PUT`, `DELETE`).
- CRUD operations on movie collections, including adding and removing movies within collections.
- Graceful handling of HTTP status codes and JSON payloads.
The implementation has as a skeleton code Laboratory 9 - HTTP Protocol.

## Project Structure
```
.
├── client.cpp       # Entry point, command dispatch loop
├── client.hpp       # The request functions
├── helpers.cpp      # Socket I/O,HTTP send/receive,other important commands
├── helpers.hpp      # Declarations: Socket , doRequest, error handling
├── requests.cpp     # HTTP request building (GET/POST/PUT/DELETE)
├── requests.hpp     # Prototypes for compute_*_request functions
├── buffer.cpp       # Dynamic buffer for reading server responses
├── buffer.hpp       # Buffer struct and utility functions
├── json.hpp         # nlohmann/json header-only library
├── Makefile         # Build instructions
└── README.md        # This document
```

## Design & Implementation Details

### 1. Socket Management 
- **`Socket`** in `helpers.cpp` wraps a file descriptor.
  - Constructor: calls `open_connection()` to create and connect a socket.
  - Destructor: calls `close_connection()` to ensure the socket is closed automatically.

```cpp
struct Socket {
    int fd;
    Socket(const char* hostIp, int port) {
        fd = open_connection(const_cast<char*>(hostIp), port, AF_INET, SOCK_STREAM, 0);
    }
    ~Socket() {
        close_connection(fd);
    }
};
```

This approach prevents resource leaks and simplifies error handling.

### 2. Unified HTTP Requests: `doRequest()`
Instead of repeating boilerplate for each HTTP verb, I provide `doRequest()`:

- **Inputs**: method, URL, optional query parameters, content type, payload fields, cookies, JWT token.
- **Process**:
  1. Build the raw HTTP request string via `compute_get_request()`, `compute_post_request()`, `compute_put_request()`, or `compute_delete_request()`.
  2. Open a socket, `send_to_server()`, then `receive_from_server()`.
  3. Parse the status code and return an `HTTPResponse { code, raw }`.

```cpp
HTTPResponse doRequest(
    const char* host,
    int port,
    const string &method,
    const char* url,
    const char* query_params,
    const char* content_type,
    char** body_data,
    int body_data_fields_count,
    const vector<char*> &cookies,
    int cookies_count,
    const char* token
);
```

This abstraction reduces each handler to:
```cpp
auto resp = doRequest(host, port, "POST", "/api/v1/tema/library/movies",
                      nullptr, "application/json", body_array, 1,
                      user_cookies, user_cookies.size(),
                      jwt_token);
```

### 3. HTTP Request Builders
- **`compute_get_request()`** formats the request line, Host header, optional Cookie header, and Authorization header.
- **`compute_post_request()`**, **`compute_put_request()`**, and **`compute_delete_request()`** follow the same pattern, adding `Content-Type` and `Content-Length` for bodies, and concatenating multiple payload parts when needed.

### 4. Buffering Responses
Server responses can arrive in multiple chunks. I use a dynamic `buffer` in `buffer.cpp` to accumulate data until the full header + body is read based on the `Content-Length` header. This ensures I parse complete JSON or status lines correctly.

### 5. JSON Serialization & Parsing
- **Library**: `nlohmann/json` (`json.hpp`).
- **Usage**:
  - **Serialize**: Build a JSON object in code, call `dump()` to get a string for the request payload.
  - **Parse**: After a successful 2xx response, locate the JSON start via `basic_extract_json_response()`, then call `json::parse()` to obtain an in-memory `json` object or array.

This decouples low‑level HTTP from high‑level data structures.

### 6. Command Dispatcher
To avoid a long `if-else if` chain in `main()`, I register each command in a `unordered_map<string, Handler>`:

```cpp
using Handler  = function<void(Context&)>;
static const auto& getDispatcher() {
    static unordered_map<string, Handler> dispatch {
        { "login_admin",    [](Context& c){ login_admin(c.hostIp, c.port, *c.adminCookies); } },
        { "get_movies",     [](Context& c){ get_movies(c.hostIp, c.port, *c.userCookies, *c.token); } },
        // ... other commands ...
    };
    return dispatch;
}
```

In `main()`:
1. Read input line.
2. Split the command token.
3. Lookup in `dispatch` and invoke the corresponding lambda.
4. If missing, print `"ERROR: Unknown command
"`.

This pattern is extensible and keeps command logic modular.

### 7. Error Handling & Messaging
I use a centralized `unordered_map<int,const char*> HttpErrorMessages`:

```cpp
static const unordered_map<int,const char*> HttpErrorMessages = {
    {400, "ERROR: Invalid data or incomplete
"},
    {403, "ERROR: Forbidden – insufficient permissions
"},
    {404, "ERROR: Not found
"},
};

```

This ensures consistency and eliminates repetition.


This design balances clarity, modularity, and performance by layering:

1. **Transport** (sockets + buffer)
2. **Protocol** (HTTP request builders + parser)
3. **Data** (JSON serialization/parsing)
4. **Logic** (command handlers + dispatcher)

### 8.Functionality Overall Resume 

- **Input Handling**  
  Each command function begins by prompting the user for the required fields (e.g. `username`, `password`, `id`, etc.) and reading them via `getline()`.

- **JSON Serialization**  
  User inputs are marshaled into a `nlohmann::json` object and serialized with `json::dump()` into a temporary C‐string buffer.

- **Unified Request Dispatch**  
  All HTTP verbs (`GET`, `POST`, `PUT`, `DELETE`) are funneled through a single `doRequest()` call, which takes:
  - The HTTP method and endpoint URL  
  - Optional query parameters or JSON payload  
  - Session cookies and JWT token (if present)

- **Low‐Level HTTP Construction**  
  Inside `doRequest()`, I call the appropriate `compute_*_request()` helper to build the raw HTTP request string (request line, headers, blank line, and body).

- **Socket I/O**  
  A TCP connection is opened via `open_connection()`, the request is sent with `send_to_server()`, and the full response is read into a dynamic buffer using `receive_from_server()`.

- **Response Parsing**  
  I extract the HTTP status code with `sscanf()`, and keep the entire raw response (headers + body) in an `HTTPResponse` struct.

- **Error Mapping & JSON Parsing**  
  - If `response.code` is in the 2xx range, handlers parse the JSON body (when applicable) and update local state (e.g. storing cookies, extracting the JWT).  
  - On non‐2xx codes,I look up a user‐friendly error string via our `errorMessageFor(code)` map or other necessary errors.

- **User Feedback**  
  Each handler prints either a `SUCCESS:` message (and any returned data) or an `ERROR:` message based on the status code. The main loop then continues until the user enters `exit`.  

