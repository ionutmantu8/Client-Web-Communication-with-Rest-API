// Laboratory-9 HTTP Protocol -> inspired by requests.h
#ifndef REQUESTS_HPP
#define REQUESTS_HPP

#include <vector>
#include <string>
using namespace std;
char *compute_get_request(char *host,
						  char *url,
						  char *query_params,
						  vector<char *> cookies,
						  int cookies_count,
						  char *token);

char *compute_post_request(char *host,
						   char *url,
						   char *content_type,
						   char **body_data,
						   int body_data_fields_count,
						   vector<char *> cookies,
						   int cookies_count,
						   char *token);

char *compute_delete_request(char *host,
							 char *url,
							 vector<char *> cookies,
							 int cookies_count,
							 char *token);
char *compute_put_request(const char *host, const char *url,
						  const char *content_type, const string &payload,
						  vector<char *> &cookies, int cookies_count,
						  char *token);
#endif