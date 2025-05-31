// Laboratory 9 HTTP Protocol -> helpers.c
#include <stdlib.h> /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>		/* read, write, close */
#include <string.h>		/* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>		/* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.hpp"
#include "buffer.hpp"
#include "requests.hpp"
#define HEADER_TERMINATOR "\r\n\r\n"
#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void compute_message(char *message, const char *line)
{
	strcat(message, line);
	strcat(message, "\r\n");
}

int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag)
{
	struct sockaddr_in serv_addr;
	int sockfd = socket(ip_type, socket_type, flag);
	if (sockfd < 0)
		error("ERROR opening socket");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = ip_type;
	serv_addr.sin_port = htons(portno);
	inet_aton(host_ip, &serv_addr.sin_addr);

	/* connect the socket */
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");

	return sockfd;
}

void close_connection(int sockfd)
{
	close(sockfd);
}

void send_to_server(int sockfd, char *message)
{
	int bytes, sent = 0;
	int total = strlen(message);

	do
	{
		bytes = write(sockfd, message + sent, total - sent);
		if (bytes < 0)
		{
			error("ERROR writing message to socket");
		}

		if (bytes == 0)
		{
			break;
		}

		sent += bytes;
	} while (sent < total);
}

char *receive_from_server(int sockfd)
{
	char response[BUFLEN];
	buffer buffer = buffer_init();
	int header_end = 0;
	int content_length = 0;

	do
	{
		int bytes = read(sockfd, response, BUFLEN);

		if (bytes < 0)
		{
			error("ERROR reading response from socket");
		}

		if (bytes == 0)
		{
			break;
		}

		buffer_add(&buffer, response, (size_t)bytes);

		header_end = buffer_find(&buffer, HEADER_TERMINATOR, HEADER_TERMINATOR_SIZE);

		if (header_end >= 0)
		{
			header_end += HEADER_TERMINATOR_SIZE;

			int content_length_start = buffer_find_insensitive(&buffer, CONTENT_LENGTH, CONTENT_LENGTH_SIZE);

			if (content_length_start < 0)
			{
				continue;
			}

			content_length_start += CONTENT_LENGTH_SIZE;
			content_length = strtol(buffer.data + content_length_start, NULL, 10);
			break;
		}
	} while (1);
	size_t total = content_length + (size_t)header_end;

	while (buffer.size < total)
	{
		int bytes = read(sockfd, response, BUFLEN);

		if (bytes < 0)
		{
			error("ERROR reading response from socket");
		}

		if (bytes == 0)
		{
			break;
		}

		buffer_add(&buffer, response, (size_t)bytes);
	}
	buffer_add(&buffer, "", 1);
	return buffer.data;
}

char *basic_extract_json_response(char *str)
{
	return strstr(str, "{\"");
}

void extract_cookies(char *response, char **cookies, int *cookies_count)
{
	const char *prefix = "Set-Cookie: ";
	char *p = response;
	while ((p = strstr(p, prefix)) != NULL)
	{
		p += strlen(prefix);
		char *end = strstr(p, "\r\n");
		if (!end)
			break;
		size_t len = end - p;
		char *cookie = (char *)calloc(len + 1, 1);
		memcpy(cookie, p, len);
		cookies[(*cookies_count)++] = cookie;
		p = end + 2;
	}
}
// Socket class constructor and destructor
Socket::Socket(const char *hostIp, int port)
{
	fd = open_connection(const_cast<char *>(hostIp), port, AF_INET, SOCK_STREAM, 0);
}
Socket::~Socket()
{
	close_connection(fd);
}
// HTTPResponse helper for processing the requests
HTTPResponse doRequest(
	const char *host,
	int port,
	const string &method,
	const char *url,
	const char *query_params,
	const char *content_type,
	char **body_data,
	int body_data_fields_count,
	const vector<char *> &cookies,
	int cookies_count,
	const char *token)
{
	char *message = nullptr;
	if (method == "GET")
	{
		message = compute_get_request(
			const_cast<char *>(host),
			const_cast<char *>(url),
			const_cast<char *>(query_params),
			cookies, cookies_count,
			const_cast<char *>(token));
	}
	else if (method == "POST")
	{
		message = compute_post_request(
			const_cast<char *>(host),
			const_cast<char *>(url),
			const_cast<char *>(content_type),
			body_data,
			body_data_fields_count,
			cookies, cookies_count,
			const_cast<char *>(token));
	}
	else if (method == "DELETE")
	{
		message = compute_delete_request(
			const_cast<char *>(host),
			const_cast<char *>(url),
			cookies, cookies_count,
			const_cast<char *>(token));
	}
	else if (method == "PUT")
	{
		string payload(body_data[0]);
		message = compute_put_request(
			host,
			url,
			content_type,
			payload,
			const_cast<vector<char *> &>(cookies),
			cookies_count,
			const_cast<char *>(token));
	}
	else
	{
		return {0, ""};
	}

	int sockfd = open_connection(const_cast<char *>(host), port, AF_INET, SOCK_STREAM, 0);
	send_to_server(sockfd, message);
	free(message);

	char *resp_c = receive_from_server(sockfd);
	close_connection(sockfd);

	int code = 0;
	sscanf(resp_c, "HTTP/%*s %d", &code);

	string raw{resp_c};
	free(resp_c);

	return {code, raw};
}
// A map for dispatching the commands to the appropriate functions
const unordered_map<string, Handler> &getDispatcher()
{
	static unordered_map<string, Handler> dispatch{
		{"login_admin", [](Context &c)
		 { login_admin((c.hostIp), c.port, *c.adminCookies); }},
		{"add_user", [](Context &c)
		 { add_user(c.hostIp, c.port, *c.adminCookies, *c.token); }},
		{"get_users", [](Context &c)
		 { get_users(c.hostIp, c.port, *c.adminCookies, *c.token); }},
		{"delete_user", [](Context &c)
		 { delete_user(c.hostIp, c.port, *c.adminCookies, *c.token); }},
		{"logout_admin", [](Context &c)
		 { logout_admin(c.hostIp, c.port, *c.adminCookies); }},
		{"login", [](Context &c)
		 { login(c.hostIp, c.port, *c.adminCookies, *c.userCookies); }},
		{"get_access", [](Context &c)
		 { get_access(c.hostIp, c.port, *c.userCookies, c.token); }},
		{"logout", [](Context &c)
		 { logout(c.hostIp, c.port, *c.userCookies, *c.token); }},
		{"add_movie", [](Context &c)
		 { add_movie(c.hostIp, c.port, *c.userCookies, *c.token); }},
		{"get_movies", [](Context &c)
		 { get_movies(c.hostIp, c.port, *c.userCookies, *c.token); }},
		{"get_movie", [](Context &c)
		 { get_movie(c.hostIp, c.port, *c.userCookies, *c.token); }},
		{"delete_movie", [](Context &c)
		 { delete_movie(c.hostIp, c.port, *c.userCookies, *c.token); }},
		{"update_movie", [](Context &c)
		 { update_movie(c.hostIp, c.port, *c.userCookies, *c.token); }},
		{"add_collection", [](Context &c)
		 { add_collection(c.hostIp, c.port, *c.userCookies, *c.token); }},
		{"get_collections", [](Context &c)
		 { get_collections(c.hostIp, c.port, *c.userCookies, *c.token); }},
		{"get_collection", [](Context &c)
		 { get_collection(c.hostIp, c.port, *c.userCookies, *c.token); }},
		{"delete_collection", [](Context &c)
		 { delete_collection(c.hostIp, c.port, *c.userCookies, *c.token); }},
		{"add_movie_to_collection", [](Context &c)
		 { add_movie_to_collection(c.hostIp, c.port, *c.userCookies, *c.token); }},
		{"delete_movie_from_collection", [](Context &c)
		 { delete_movie_from_collection(c.hostIp, c.port, *c.userCookies, *c.token); }}};
	return dispatch;
}
