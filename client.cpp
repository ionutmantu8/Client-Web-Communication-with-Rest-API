#include <stdio.h>		/* printf, sprintf */
#include <stdlib.h>		/* exit, atoi, malloc, free */
#include <unistd.h>		/* read, write, close */
#include <string.h>		/* memcpy, memset, strdup */
#include <sys/socket.h> /* socket definitions */
#include <vector>
#include <string>
#include "helpers.hpp"
#include "requests.hpp"
#include "json.hpp"
#include "client.hpp"
#include <iostream>
#include <unordered_map>
using namespace std;
using json = nlohmann::json;

static const unordered_map<int, const char *> HttpErrorMessages = {
	{400, "ERROR: Date invalide/incomplete\n"},
	{403, "ERROR: Nu sunteti owner\n"},
	{404, "ERROR: ID invalid\n"},

};
/**
 * This function returns the error message corresponding to the given error code.
 * If the error code is not found, it returns a default error message.
 **/

inline const char *errorMessageFor(int code)
{
	auto it = HttpErrorMessages.find(code);
	return it != HttpErrorMessages.end()
			   ? it->second
			   : "ERROR: Fara acces la library\n";
}
/**
 * This function handles the login process for the admin user.
 * It prompts for username and password, validates them,extracts the login cookie
 * in an array for admin cookies and sends a login request to the server.
 **/
void login_admin(char *hostIp, int port, vector<char *> &admin_cookies)
{
	cout << "username=" << flush;
	string username;
	getline(cin, username);
	cout << "password=" << flush;
	string password;
	getline(cin, password);

	if (username.empty() || password.empty())
	{
		cout << "ERROR: Credentiale invalide\n"
			 << flush;
		return;
	}
	if (!admin_cookies.empty())
	{
		cout << "ERROR: Admin deja autentificat\n"
			 << flush;
		return;
	}

	json json_data;
	json_data["username"] = username;
	json_data["password"] = password;
	char *body = strdup(json_data.dump().c_str());
	char *bd[1] = {body};

	auto response = doRequest(hostIp, port, "POST",
							  "/api/v1/tema/admin/login",
							  nullptr,
							  "application/json",
							  bd, 1,
							  admin_cookies, admin_cookies.size(),
							  nullptr);

	free(body);

	if (response.code >= 200 && response.code < 300)
	{
		cout << "SUCCESS: Admin autentificat cu succes\n";
		char *cookie_arr[16];
		int cookie_count = 0;
		extract_cookies((char *)response.raw.c_str(), cookie_arr, &cookie_count);
		for (int i = 0; i < cookie_count; i++)
		{
			admin_cookies.push_back(cookie_arr[i]);
		}
	}
	else if (response.code == 401)
	{
		cout << "ERROR: Credentiale invalide\n";
	}
	else
	{
		cout << "ERROR: Autentificare esuata\n";
	}
	cout << flush;
}

/**
 * This function handles the process of adding a new user.
 * It prompts for username and password, validates them, and sends a request to add the user.
 **/
void add_user(char *hostIp, int port, vector<char *> &admin_cookies, char *token)
{
	cout << "username=" << flush;
	string username;
	getline(cin, username);
	cout << "password=" << flush;
	string password;
	getline(cin, password);

	if (username.empty() || password.empty())
	{
		cout << "ERROR: Credentiale invalide\n"
			 << flush;
		return;
	}

	json json_data;
	json_data["username"] = username;
	json_data["password"] = password;
	char *body = strdup(json_data.dump().c_str());
	char *bd[1] = {body};

	auto response = doRequest(hostIp, port, "POST",
							  "/api/v1/tema/admin/users",
							  nullptr,
							  "application/json",
							  bd, 1,
							  admin_cookies, admin_cookies.size(),
							  token);

	free(body);

	if (response.code >= 200 && response.code < 300)
	{
		cout << "SUCCESS: User adaugat cu succes\n";
	}
	else if (response.code == 403)
	{
		cout << "ERROR: Lipsa permisiuni admin\n";
	}
	else
	{
		cout << "ERROR: Adaugare user esuata\n";
	}
}
/**
 * This function displays the list of users.
 * It parses the JSON response and prints the username and password for each user in the get_users request.
 **/
void show_users(const json &j)
{
	cout << "SUCCESS: Lista utilizatorilor\n";
	int idx = 1;
	for (auto &user : j["users"])
	{
		cout << "#" << idx++
			 << " " << user["username"].get<string>()
			 << ":" << user["password"].get<string>() << "\n";
	}
	cout << flush;
}
/*
 * This function retrieves the list of users from the server.
 * It sends a GET request to the server and displays the list of users.
 */
void get_users(char *hostIp, int port, vector<char *> &admin_cookies, char *token)
{
	auto response = doRequest(hostIp, port, "GET",
							  "/api/v1/tema/admin/users",
							  nullptr,
							  nullptr,
							  nullptr, 0,
							  admin_cookies, admin_cookies.size(),
							  token);

	if (response.code >= 200 && response.code < 300)
	{
		char *body = strstr((char *)response.raw.c_str(), "\r\n\r\n");
		if (body)
		{
			auto json_parsed = json::parse(body + 4);
			show_users(json_parsed);
		}
		else
		{
			cout << "SUCCESS: Lista utilizatorilor\n"
				 << flush;
		}
	}
	else if (response.code == 403)
	{
		cout << "ERROR: Lipsa permisiuni admin\n";
	}
	else
	{
		cout << "ERROR: Eroare la obtinerea userilor\n"
			 << flush;
	}
}
/**
 * This function deletes a user from the server.
 * It prompts for the username, validates it, and sends a request to delete the user.
 **/
void delete_user(char *hostIp, int port, vector<char *> &admin_cookies, char *token)
{
	cout << "username=" << flush;
	string username;
	getline(cin, username);
	if (username.empty())
	{
		cout << "ERROR: Invalid username\n"
			 << flush;
		return;
	}

	string path = "/api/v1/tema/admin/users/" + username;
	auto response = doRequest(hostIp, port, "DELETE",
							  path.c_str(),
							  nullptr,
							  nullptr,
							  nullptr, 0,
							  admin_cookies, admin_cookies.size(),
							  token);

	if (response.code >= 200 && response.code < 300)
	{
		cout << "SUCCESS: Utilizator sters\n";
	}
	else if (response.code == 403)
	{
		cout << "ERROR: Lipsa permisiuni admin\n";
	}
	else if (response.code == 404)
	{
		cout << "ERROR: Username invalid\n";
	}
	else
	{
		cout << "ERROR: Stergere user esuata\n";
	}
}

/**
 * This function handles the logout process for the admin user.
 * It sends a logout request to the server and clears the admin cookies.
 **/
void logout_admin(char *hostIp, int port, vector<char *> &admin_cookies)
{
	if (admin_cookies.empty())
	{
		cout << "ERROR: Admin nu este autentificat\n"
			 << flush;
		return;
	}

	auto response = doRequest(hostIp, port, "GET",
							  "/api/v1/tema/admin/logout",
							  nullptr,
							  nullptr,
							  nullptr, 0,
							  admin_cookies, admin_cookies.size(),
							  nullptr);

	if (response.code >= 200 && response.code < 300)
	{
		cout << "SUCCESS: Admin delogat\n"
			 << flush;
		for (auto cookie : admin_cookies)
			free(cookie);
		admin_cookies.clear();
	}
	else
	{
		cout << "ERROR: Logout esuat\n"
			 << flush;
	}
}
/**
 * This function handles the login process for a regular user.
 * It prompts for username and password, validates them, extracts the login cookie for the user
 * and sends a login request to the server.
 **/
void login(char *hostIp, int port,
		   vector<char *> &admin_cookies,
		   vector<char *> &user_cookies)
{
	cout << "admin_username=" << flush;
	string admin_username;
	getline(cin, admin_username);
	cout << "username=" << flush;
	string username;
	getline(cin, username);
	cout << "password=" << flush;
	string password;
	getline(cin, password);

	if (admin_username.empty() || username.empty() || password.empty())
	{
		cout << "ERROR: Credentiale invalide\n"
			 << flush;
		return;
	}
	if (!user_cookies.empty())
	{
		cout << "ERROR: User deja autentificat\n"
			 << flush;
		return;
	}

	json json_data;
	json_data["admin_username"] = admin_username;
	json_data["username"] = username;
	json_data["password"] = password;
	char *body = strdup(json_data.dump().c_str());
	char *bd[1] = {body};

	auto response = doRequest(hostIp, port, "POST",
							  "/api/v1/tema/user/login",
							  nullptr,
							  "application/json",
							  bd, 1,
							  user_cookies, user_cookies.size(),
							  nullptr);

	free(body);

	if (response.code >= 200 && response.code < 300)
	{
		cout << "SUCCESS: User autentificat cu succes\n"
			 << flush;
		char *ck_arr[16];
		int ck_cnt = 0;
		extract_cookies((char *)response.raw.c_str(), ck_arr, &ck_cnt);
		for (int i = 0; i < ck_cnt; ++i)
			user_cookies.push_back(ck_arr[i]);
	}
	else if (response.code == 401)
	{
		cout << "ERROR: Credentiale invalide\n"
			 << flush;
	}
	else
	{
		cout << "ERROR: Autentificare esuata\n"
			 << flush;
	}
}
/**
 * This function retrieves the access token for the user.
 * It sends a request to the server and extracts the token from the response.
 **/
void get_access(char *hostIp, int port,
				vector<char *> &user_cookies,
				char **token)
{
	if (user_cookies.empty())
	{
		cout << "ERROR: User nu este autentificat\n"
			 << flush;
		return;
	}

	auto response = doRequest(hostIp, port, "GET",
							  "/api/v1/tema/library/access",
							  nullptr,
							  nullptr,
							  nullptr, 0,
							  user_cookies, user_cookies.size(),
							  nullptr);

	if (response.code < 200 || response.code >= 300)
	{
		cout << "ERROR: Acces esuat\n"
			 << flush;
		return;
	}

	auto pos = response.raw.find("\r\n\r\n");
	if (pos == string::npos)
	{
		cout << "ERROR: Raspuns invalid\n"
			 << flush;
		return;
	}

	string body = response.raw.substr(pos + 4);
	if (!json::accept(body))
	{
		cout << "ERROR: Raspuns invalid \n"
			 << flush;
		return;
	}

	auto json_parsed = json::parse(body);
	if (json_parsed.contains("token") && json_parsed["token"].is_string())
	{
		*token = strdup(json_parsed["token"].get<string>().c_str());
		cout << "SUCCESS: Token JWT primit\n"
			 << flush;
	}
	else
	{
		cout << "ERROR: Token lipsa in raspuns\n"
			 << flush;
	}
}
/**
 * This function handles the logout process for the user.
 * It sends a logout request to the server and clears the user cookies.
 **/
void logout(char *hostIp, int port,
			vector<char *> &user_cookies,
			char *token)
{
	if (user_cookies.empty())
	{
		cout << "SUCCESS: Utilizator delogat\n"
			 << flush;
		if (token)
		{
			free(token);
			token = nullptr;
		}
		return;
	}

	auto response = doRequest(hostIp, port, "GET",
							  "/api/v1/tema/user/logout",
							  nullptr,
							  nullptr,
							  nullptr, 0,
							  user_cookies, user_cookies.size(),
							  nullptr);

	if (response.code >= 200 && response.code < 300)
	{
		cout << "SUCCESS: Utilizator delogat\n"
			 << flush;
		for (auto cookie : user_cookies)
			free(cookie);
		user_cookies.clear();
	}
	else
	{
		cout << "ERROR: Logout esuat\n"
			 << flush;
	}
}
/**
 * This function adds a new movie to the library.
 * It prompts for movie details, validates them, and sends a request to add the movie.
 **/
void add_movie(char *hostIp, int port,
			   vector<char *> &user_cookies,
			   char *token)
{
	cout << "title=" << flush;
	string title;
	getline(cin, title);
	cout << "year=" << flush;
	string year_s;
	getline(cin, year_s);
	cout << "description=" << flush;
	string description;
	getline(cin, description);
	cout << "rating=" << flush;
	string rating_s;
	getline(cin, rating_s);

	if (title.empty() || year_s.empty() ||
		description.empty() || rating_s.empty())
	{
		cout << "ERROR: Invalid input\n"
			 << flush;
		return;
	}

	int year;
	double rating;
	try
	{
		year = stoi(year_s);
		rating = stod(rating_s);
	}
	catch (...)
	{
		cout << "ERROR: Invalid input\n"
			 << flush;
		return;
	}

	json json_data;
	json_data["title"] = title;
	json_data["year"] = year;
	json_data["description"] = description;
	json_data["rating"] = rating;
	char *body = strdup(json_data.dump().c_str());
	char *bd[1] = {body};

	auto response = doRequest(hostIp, port, "POST",
							  "/api/v1/tema/library/movies",
							  nullptr,
							  "application/json",
							  bd, 1,
							  user_cookies, user_cookies.size(),
							  token);

	free(body);

	if (response.code >= 200 && response.code < 300)
		cout << "SUCCESS: Film adaugat\n";
	else if (response.code == 403)
		cout << "ERROR: Fara acces library\n";
	else if (response.code == 400)
		cout << "ERROR: Date invalide\n";
	else
		cout << "ERROR: Adaugare film esuata\n";
}
/**
 * This function displays the list of movies.
 * It parses the JSON response and prints the movie ID and title for each movie in the get_movies request.
 **/
void show_movies(const json &movies_arr)
{
	cout << "SUCCESS: Lista filmelor\n";
	for (const auto &movie : movies_arr)
	{
		cout << "#" << movie["id"].get<int>()
			 << " " << movie["title"].get<string>() << "\n";
	}
	cout << flush;
}

/*
 * This function retrieves the list of movies from the server.
 * It sends a GET request to the server and displays the list of movies.
 */
void get_movies(char *hostIp, int port, vector<char *> &user_cookies, char *token)
{
	auto response = doRequest(hostIp, port, "GET",
							  "/api/v1/tema/library/movies",
							  nullptr,
							  nullptr,
							  nullptr, 0,
							  user_cookies, user_cookies.size(),
							  token);

	if (response.code < 200 || response.code >= 300)
	{
		cout << "ERROR: Fara acces library\n";
		return;
	}

	char *json_part = basic_extract_json_response((char *)response.raw.c_str());
	if (!json_part)
	{
		cout << "ERROR: Fara acces library\n";
		return;
	}

	auto root = json::parse(json_part, nullptr, false);
	if (root.is_discarded())
	{
		cout << "ERROR: Fara acces library\n";
	}
	else if (root.is_array())
	{
		show_movies(root);
	}
	else if (root.is_object() && root.contains("movies") && root["movies"].is_array())
	{
		show_movies(root["movies"]);
	}
	else
	{
		cout << "ERROR: Fara acces library\n";
	}
}
/**
 * This function retrieves the details of a specific movie.
 * It prompts for the movie ID, validates it, and sends a request to get the movie details.
 **/
void get_movie(char *hostIp, int port, vector<char *> &user_cookies, char *token)
{
	cout << "id=" << flush;
	string id;
	getline(cin, id);

	string path = "/api/v1/tema/library/movies/" + id;
	auto response = doRequest(hostIp, port, "GET",
							  path.c_str(),
							  nullptr,
							  nullptr,
							  nullptr, 0,
							  user_cookies, user_cookies.size(),
							  token);

	if (response.code >= 200 && response.code < 300)
	{
		auto pos = response.raw.find("\r\n\r\n");
		if (pos != string::npos)
		{
			cout << response.raw.substr(pos + 4) << "\n";
		}
		else
		{
			cout << "{}\n";
		}
	}
	else if (response.code == 404)
	{
		cout << "ERROR: ID invalid\n";
	}
	else
	{
		cout << "ERROR: Fara acces library\n";
	}
}
/**
 * This function deletes a movie from the library.
 * It prompts for the movie ID, validates it, and sends a request to delete the movie.
 **/
void delete_movie(char *hostIp, int port, vector<char *> &user_cookies, char *token)
{
	cout << "id=" << flush;
	string id;
	getline(cin, id);
	if (id.empty())
	{
		cout << "ERROR: Invalid ID\n";
		return;
	}

	string path = "/api/v1/tema/library/movies/" + id;
	auto response = doRequest(hostIp, port, "DELETE",
							  path.c_str(),
							  nullptr,
							  nullptr,
							  nullptr, 0,
							  user_cookies, user_cookies.size(),
							  token);

	if (response.code >= 200 && response.code < 300)
	{
		cout << "SUCCESS: Film sters\n";
	}
	else if (response.code == 404)
	{
		cout << "ERROR: ID invalid\n";
	}
	else if (response.code == 403)
	{
		cout << "ERROR: Fara acces library\n";
	}

	else
	{
		cout << "ERROR: Stergere film esuata\n";
	}
}
/**
 * This function updates the details of a specific movie.
 * It prompts for the movie ID and new details, validates them, and sends a request to update the movie.
 **/
void update_movie(char *hostIp, int port, vector<char *> &user_cookies, char *token)
{
	cout << "id=" << flush;
	string id;
	getline(cin, id);
	cout << "title=" << flush;
	string title;
	getline(cin, title);
	cout << "year=" << flush;
	string year_s;
	getline(cin, year_s);
	cout << "description=" << flush;
	string description;
	getline(cin, description);
	cout << "rating=" << flush;
	string rating_s;
	getline(cin, rating_s);

	if (title.empty() || year_s.empty() || description.empty() || rating_s.empty())
	{
		cout << "ERROR: Invalid input\n";
		return;
	}

	int year;
	double rating;
	try
	{
		year = stoi(year_s);
		rating = stod(rating_s);
	}
	catch (...)
	{
		cout << "ERROR: Invalid input\n";
		return;
	}

	json json_data;
	json_data["title"] = title;
	json_data["year"] = year;
	json_data["description"] = description;
	json_data["rating"] = rating;
	char *body = strdup(json_data.dump().c_str());
	char *bd[1] = {body};

	string path = "/api/v1/tema/library/movies/" + id;
	auto response = doRequest(hostIp, port, "PUT",
							  path.c_str(),
							  nullptr,
							  "application/json",
							  bd, 1,
							  user_cookies, user_cookies.size(),
							  token);

	free(body);

	if (response.code >= 200 && response.code < 300)
		cout << "SUCCESS: Film actualizat\n";
	else if (response.code == 403)
		cout << "ERROR: Fara acces library\n";
	else if (response.code == 404)
		cout << "ERROR: ID invalid\n";
	else if (response.code == 400)
		cout << "ERROR: Date invalide\n";
	else
		cout << "ERROR: Actualizare film esuata\n";
}
/**
 * This function adds a new collection to the library.The logic is made with 2 post requests,one to add an empty collection with the title
 * and the second to add the movies that were given as input to the new collection.
 * It prompts for the collection title and movie IDs, validates them, and sends a request to add the collection.
 **/
void add_collection(char *hostIp, int port, vector<char *> &user_cookies, char *token)
{
	cout << "title=" << flush;
	string title;
	getline(cin, title);
	cout << "num_movies=" << flush;
	string num_s;
	getline(cin, num_s);

	int num = 0;
	try
	{
		num = stoi(num_s);
	}
	catch (...)
	{
		num = 0;
	}

	vector<int> ids;
	for (int i = 0; i < num; i++)
	{
		cout << "movie_id[" << i << "]=" << flush;
		string movie_id;
		getline(cin, movie_id);
		if (!movie_id.empty() && all_of(movie_id.begin(), movie_id.end(), ::isdigit))
			ids.push_back(stoi(movie_id));
		else
		{
			cout << "ERROR: Date invalide/incomplete\n";
			return;
		}
	}
	if (title.empty())
	{
		cout << "ERROR: Date invalide/incomplete\n";
		return;
	}

	json collection_json;
	collection_json["title"] = title;
	char *body1 = strdup(collection_json.dump().c_str());
	char *bd1[1] = {body1};

	auto response1 = doRequest(hostIp, port, "POST",
							   "/api/v1/tema/library/collections",
							   nullptr,
							   "application/json",
							   bd1, 1,
							   user_cookies, user_cookies.size(),
							   token);

	free(body1);

	if (response1.code == 400)
	{
		cout << "ERROR: Date invalide/incomplete\n";
		return;
	}
	else if (response1.code < 200 || response1.code >= 300)
	{
		cout << "ERROR: Fara acces library\n";
		return;
	}

	char *json_part = basic_extract_json_response((char *)response1.raw.c_str());
	auto json_response = json::parse(json_part);
	int coll_id = json_response["id"].get<int>();

	for (int id : ids)
	{
		json movie_json;
		movie_json["id"] = id;
		char *body2 = strdup(movie_json.dump().c_str());
		char *bd2[1] = {body2};

		string path = "/api/v1/tema/library/collections/" + to_string(coll_id) + "/movies";
		auto response2 = doRequest(hostIp, port, "POST",
								   path.c_str(),
								   nullptr,
								   "application/json",
								   bd2, 1,
								   user_cookies, user_cookies.size(),
								   token);

		free(body2);
	}

	cout << "SUCCESS: Colectie adaugata\n";
}
/**
 * This function displays the list of collections.
 * It parses the JSON response and prints the collection ID and title for each collection in the get_collections request.
 **/
void show_collections(const json *arr)
{
	cout << "SUCCESS: Lista colectiilor\n";
	if (!arr)
		return;
	for (auto &collection : *arr)
	{
		cout << "#" << collection["id"].get<int>()
			 << ": " << collection["title"].get<string>() << "\n";
	}
}
/**
 * This function retrieves the list of collections from the server.
 * It sends a GET request to the server and displays the list of collections.
 **/
void get_collections(char *hostIp, int port, vector<char *> &user_cookies, char *token)
{
	auto response = doRequest(hostIp, port, "GET",
							  "/api/v1/tema/library/collections",
							  nullptr,
							  nullptr,
							  nullptr, 0,
							  user_cookies, user_cookies.size(),
							  token);

	if (response.code < 200 || response.code >= 300)
	{
		cout << "ERROR: Fara acces library\n";
		return;
	}

	auto pos = response.raw.find("\r\n\r\n");
	if (pos == string::npos)
	{
		cout << "ERROR: Fara acces library\n";
		return;
	}

	auto root = json::parse(response.raw.substr(pos + 4), nullptr, false);
	if (root.is_discarded())
	{
		cout << "ERROR: Fara acces library\n";
		return;
	}

	if (root.is_array())
	{
		show_collections(&root);
	}
	else if (root.is_object() && root.contains("collections") && root["collections"].is_array())
	{
		show_collections(&root["collections"]);
	}
}
/**
 * This function retrieves the details of a specific collection.
 * It prompts for the collection ID, validates it, and sends a request to get the collection details.
 **/
void get_collection(char *hostIp, int port, vector<char *> &user_cookies, char *token)
{
	cout << "id=" << flush;
	string id;
	getline(cin, id);

	string path = "/api/v1/tema/library/collections/" + id;
	auto response = doRequest(hostIp, port, "GET",
							  path.c_str(),
							  nullptr,
							  nullptr,
							  nullptr, 0,
							  user_cookies, user_cookies.size(),
							  token);

	if (response.code == 404)
	{
		cout << "ERROR: ID invalid\n";
		return;
	}
	else if (response.code < 200 || response.code >= 300)
	{
		cout << "ERROR: Fara acces library\n";
		return;
	}

	auto pos = response.raw.find("\r\n\r\n");
	if (pos == string::npos)
	{
		cout << "ERROR: Raspuns invalid\n";
		return;
	}

	char *json_part = basic_extract_json_response((char *)response.raw.c_str() + pos + 4);
	auto j = json::parse(json_part, nullptr, false);

	if (j.is_discarded() || !j.contains("title") || !j["title"].is_string() || !j.contains("owner") || !j["owner"].is_string() || !j.contains("movies") || !j["movies"].is_array())
	{
		cout << "ERROR: Raspuns invalid\n";
		return;
	}

	cout << "SUCCESS: Detalii colectie\n"
		 << "title: " << j["title"].get<string>() << "\n"
		 << "owner: " << j["owner"].get<string>() << "\n";

	for (auto &movie : j["movies"])
	{
		cout << "#" << movie["id"].get<int>()
			 << ": " << movie["title"].get<string>() << "\n";
	}
}
/**
 * This function deletes a collection from the library.
 * It prompts for the collection ID, validates it, and sends a request to delete the collection.
 **/
void delete_collection(char *hostIp, int port, vector<char *> &user_cookies, char *token)
{
	cout << "id=" << flush;
	string id;
	getline(cin, id);
	if (id.empty())
	{
		cout << "ERROR: Invalid ID\n";
		return;
	}

	string path = "/api/v1/tema/library/collections/" + id;
	auto response = doRequest(hostIp, port, "DELETE",
							  path.c_str(),
							  nullptr,
							  nullptr,
							  nullptr, 0,
							  user_cookies, user_cookies.size(),
							  token);

	if (response.code >= 200 && response.code < 300)
	{
		cout << "SUCCESS: Colectie stearsa\n";
	}
	else
	{
		cout << errorMessageFor(response.code);
	}
}
/**
 * This function adds a movie to a specific collection.
 * It prompts for the collection ID and movie ID, validates them, and sends a request to add the movie to the collection.
 **/
void add_movie_to_collection(char *hostIp, int port, vector<char *> &user_cookies, char *token)
{
	cout << "collection_id=" << flush;
	string collection_id;
	getline(cin, collection_id);
	cout << "movie_id=" << flush;
	string movie_id;
	getline(cin, movie_id);

	if (collection_id.empty() || movie_id.empty() || !all_of(collection_id.begin(), collection_id.end(), ::isdigit) || !all_of(movie_id.begin(), movie_id.end(), ::isdigit))
	{
		cout << "ERROR: Date invalide/incomplete\n";
		return;
	}

	json json_data;
	json_data["id"] = stoi(movie_id);
	char *body = strdup(json_data.dump().c_str());
	char *bd[1] = {body};

	string path = "/api/v1/tema/library/collections/" + collection_id + "/movies";
	auto response = doRequest(hostIp, port, "POST",
							  path.c_str(),
							  nullptr,
							  "application/json",
							  bd, 1,
							  user_cookies, user_cookies.size(),
							  token);

	free(body);

	if (response.code >= 200 && response.code < 300)
		cout << "SUCCESS: Film adaugat in colectie\n";
	else
		cout << errorMessageFor(response.code);
}
/**
 * This function deletes a movie from a specific collection.
 * It prompts for the collection ID and movie ID, validates them, and sends a request to delete the movie from the collection.
 **/
void delete_movie_from_collection(char *hostIp, int port, vector<char *> &user_cookies, char *token)
{
	cout << "collection_id=" << flush;
	string collection_id;
	getline(cin, collection_id);
	cout << "movie_id=" << flush;
	string movie_id;
	getline(cin, movie_id);

	if (collection_id.empty() || movie_id.empty())
	{
		cout << "ERROR: Invalid ID\n";
		return;
	}

	string path = "/api/v1/tema/library/collections/" + collection_id + "/movies/" + movie_id;
	auto response = doRequest(hostIp, port, "DELETE",
							  path.c_str(),
							  nullptr,
							  nullptr,
							  nullptr, 0,
							  user_cookies, user_cookies.size(), token);

	if (response.code >= 200 && response.code < 300)
		cout << "SUCCESS: Film sters din colectie\n";
	else
		cout << errorMessageFor(response.code);
}

int main(int argc, char *argv[])
{
	char *hostIp = "63.32.125.183";
	int port = 8081;
	vector<char *> user_cookies, admin_cookies;
	char *token = nullptr;
	Context ctx{hostIp, port, &admin_cookies, &user_cookies, &token};

	auto const &dispatch = getDispatcher();

	string line;
	while (true)
	{
		if (!getline(cin, line))
			break;
		istringstream iss(line);
		string cmd;
		iss >> cmd;
		if (cmd == "exit")
			break;

		auto it = dispatch.find(cmd);
		if (it != dispatch.end())
		{
			it->second(ctx);
			if (cmd == "delete_user")
			{
				for (auto cookie : user_cookies)
					free(cookie);
				user_cookies.clear();
				if (token)
				{
					free(token);
					token = nullptr;
				}
			}
		}
		else
		{
			cout << "ERROR: Comandă necunoscută\n";
		}
	}

	for (auto cookie : admin_cookies)
		free(cookie);
	for (auto cookie : user_cookies)
		free(cookie);
	if (token)
		free(token);
	return 0;
}
