// Laboratory-9 HTTP Protocol -> inspired by requests.c
#include <stdlib.h> /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>		/* read, write, close */
#include <string.h>		/* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>		/* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.hpp"
#include "requests.hpp"
#include <vector>
#include <iostream>
using namespace std;
char *compute_get_request(char *host, char *url, char *query_params,
						  vector<char *> cookies, int cookies_count, char *token)
{
	char *message = (char *)calloc(BUFLEN, sizeof(char));
	char *line = (char *)calloc(LINELEN, sizeof(char));

	// Step 1: request line
	if (query_params)
		sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
	else
		sprintf(line, "GET %s HTTP/1.1", url);
	compute_message(message, line);

	// Step 2: Host header
	sprintf(line, "Host: %s", host);
	compute_message(message, line);

	// Step 3 (optional): cookies
	if (cookies_count > 0)
	{
		memset(line, 0, LINELEN);
		strcat(line, "Cookie: ");
		for (int i = 0; i < cookies_count; i++)
		{
			strcat(line, cookies[i]);
			if (i < cookies_count - 1)
				strcat(line, "; ");
		}
		compute_message(message, line);
	}

	// Step 4: blank line to end headers
	if (token != NULL)
	{
		sprintf(line, "Authorization: Bearer %s", token);
		compute_message(message, line);
	}
	compute_message(message, "");

	free(line);
	return message;
}

char *compute_post_request(char *host, char *url, char *content_type,
						   char **body_data, int body_data_fields_count,
						   vector<char *> cookies, int cookies_count, char *token)
{
	char *message = (char *)calloc(BUFLEN, sizeof(char));
	char *line = (char *)calloc(LINELEN, sizeof(char));
	char *body_data_buffer = (char *)calloc(LINELEN, sizeof(char));

	// Step 1: request line
	sprintf(line, "POST %s HTTP/1.1", url);
	compute_message(message, line);

	// Step 2: Host header
	sprintf(line, "Host: %s", host);
	compute_message(message, line);

	// Step 3: build payload and mandatory headers
	for (int i = 0; i < body_data_fields_count; i++)
	{
		strcat(body_data_buffer, body_data[i]);
		if (i < body_data_fields_count - 1)
			strcat(body_data_buffer, "&");
	}
	sprintf(line, "Content-Type: %s", content_type);
	compute_message(message, line);
	sprintf(line, "Content-Length: %zu", strlen(body_data_buffer));
	compute_message(message, line);

	// Step 4 (optional): cookies
	if (cookies_count > 0)
	{
		memset(line, 0, LINELEN);
		strcat(line, "Cookie: ");
		for (int i = 0; i < cookies_count; i++)
		{
			strcat(line, cookies[i]);
			if (i < cookies_count - 1)
				strcat(line, "; ");
		}
		compute_message(message, line);
	}

	// Step 5: blank line to end headers
	if (token != NULL)
	{
		sprintf(line, "Authorization: Bearer %s", token);
		compute_message(message, line);
	}
	compute_message(message, "");

	// Step 6: payload
	compute_message(message, body_data_buffer);

	free(line);
	free(body_data_buffer);
	return message;
}

char *compute_delete_request(char *host,
							 char *url,
							 std::vector<char *> cookies,
							 int cookies_count,
							 char *token)
{
	char *message = (char *)calloc(BUFLEN, sizeof(char));
	char *line = (char *)calloc(LINELEN, sizeof(char));
	// Step 1: request line
	sprintf(line, "DELETE %s HTTP/1.1", url);
	compute_message(message, line);
	// Step 2: Host header
	sprintf(line, "Host: %s", host);
	compute_message(message, line);

	// Step 3: cookies
	if (cookies_count > 0)
	{
		memset(line, 0, LINELEN);
		strcat(line, "Cookie: ");
		for (int i = 0; i < cookies_count; i++)
		{
			strcat(line, cookies[i]);
			if (i < cookies_count - 1)
				strcat(line, "; ");
		}
		compute_message(message, line);
	}
	// Step 4: blank line to end headers
	if (token)
	{
		sprintf(line, "Authorization: Bearer %s", token);
		compute_message(message, line);
	}

	compute_message(message, "");

	free(line);
	return message;
}
char *compute_put_request(const char *host, const char *url,
						  const char *content_type, const string &payload,
						  vector<char *> &cookies, int cookies_count,
						  char *token)
{
	char *message = (char *)calloc(BUFLEN, 1);
	char *line = (char *)calloc(LINELEN, 1);
	// Step 1: request line
	sprintf(line, "PUT %s HTTP/1.1", url);
	compute_message(message, line);
	// Step 2: Host header
	sprintf(line, "Host: %s", host);
	compute_message(message, line);
	// Step 3: build payload and mandatory headers
	sprintf(line, "Content-Type: %s", content_type);
	compute_message(message, line);
	sprintf(line, "Content-Length: %zu", payload.size());
	compute_message(message, line);

	// Step 4 (optional): cookies
	if (cookies_count > 0)
	{
		strcpy(line, "Cookie: ");
		for (int i = 0; i < cookies_count; ++i)
		{
			strcat(line, cookies[i]);
			if (i < cookies_count - 1)
				strcat(line, "; ");
		}
		compute_message(message, line);
	}
	// Step 5: blank line to end headers
	if (token)
	{
		sprintf(line, "Authorization: Bearer %s", token);
		compute_message(message, line);
	}
	compute_message(message, "");
	strcpy(line, payload.c_str());
	// Step 6: payload
	compute_message(message, line);

	free(line);
	return message;
}