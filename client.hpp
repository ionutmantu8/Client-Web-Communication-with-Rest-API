#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include "json.hpp"
using namespace std;
using json = nlohmann::json;

void extract_cookies(char *response, char **cookies, int *cookies_count);

void login_admin(char *hostIp, int port, vector<char *> &admin_cookies);
void add_user(char *hostIp, int port, vector<char *> &admin_cookies, char *token);
void get_users(char *hostIp, int port, vector<char *> &admin_cookies, char *token);
void delete_user(char *hostIp, int port, vector<char *> &admin_cookies, char *token);
void logout_admin(char *hostIp, int port, vector<char *> &admin_cookies);

void login(char *hostIp, int port,
		   vector<char *> &admin_cookies,
		   vector<char *> &user_cookies);
void get_access(char *hostIp, int port, vector<char *> &user_cookies, char **token);
void logout(char *hostIp, int port, vector<char *> &user_cookies, char *token);

void add_movie(char *hostIp, int port, vector<char *> &user_cookies, char *token);
void get_movies(char *hostIp, int port, vector<char *> &user_cookies, char *token);
void get_movie(char *hostIp, int port, vector<char *> &user_cookies, char *token);
void delete_movie(char *hostIp, int port, vector<char *> &user_cookies, char *token);
void update_movie(char *hostIp, int port, vector<char *> &user_cookies, char *token);

void add_collection(char *hostIp, int port, vector<char *> &user_cookies, char *token);
void get_collections(char *hostIp, int port, vector<char *> &user_cookies, char *token);
void get_collection(char *hostIp, int port, vector<char *> &user_cookies, char *token);
void delete_collection(char *hostIp, int port, vector<char *> &user_cookies, char *token);

void add_movie_to_collection(char *hostIp, int port, vector<char *> &user_cookies, char *token);
void delete_movie_from_collection(char *hostIp, int port, vector<char *> &user_cookies, char *token);

#endif
