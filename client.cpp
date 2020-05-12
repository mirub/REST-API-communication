#include <bits/stdc++.h>
#include <nlohmann/json.hpp>
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

#define IP "3.8.116.10"
#define PORT 8080

using namespace std;
using json = nlohmann::json;

enum CommandSet {
    regist, login, enter_library, get_books, get_book, add_book,
    delete_book, logout, exit
};

// Initialize the commands map with all the required commands
void initialize_map(unordered_map <string, CommandSet> &commands) {
    commands["register"] = CommandSet::regist;
    commands["login"] = CommandSet::login;
    commands["enter_library"] = CommandSet::enter_library;
    commands["get_books"] = CommandSet::get_books;
    commands["get_book"] = CommandSet::get_book;
    commands["add_book"] = CommandSet::add_book;
    commands["delete_book"] = CommandSet::delete_book;
    commands["logout"] = CommandSet::logout;
    commands["exit"] = CommandSet::exit;
}

char* get_credentials() {
    vector<string> credentials;
    string username, password;

    cout << "username=";
    cin >> username;
    cout << "password=";
    cin >> password;

    credentials.push_back(username);
    credentials.push_back(password);
    
    json j = {{"username", credentials[0]}, {"password", credentials[1]}};
    string converted_json = j.dump();

    char *creds_to_send = (char *) malloc(converted_json.size() + 1);
    strcpy(creds_to_send, converted_json.c_str());

    return creds_to_send;
}

char *get_book_details() {
    string title, author, genre, publisher, page_count;
    cout << "title=";
    cin >> title;
    cout << "author=";
    cin >> author;
    cout << "genre=";
    cin >> genre;
    cout << "publisher=";
    cin >> publisher;
    cout << "page_count=";
    cin >> page_count;

    json j = {{"title", title},
              {"author", author},
              {"genre", genre},
              {"publisher", publisher},
              {"page_count", page_count}};

    string converted_json = j.dump();             

    char *deets_to_send = (char *) malloc(converted_json.size() + 1);
    strcpy(deets_to_send, converted_json.c_str());

    return deets_to_send;
}

char* get_initial_response(char *url, int sockfd, char *type) {
    char *message, *response;
    // Get the credentials
    char *creds_to_send = get_credentials();
    char *creds[1];
    creds[0] = (char *) malloc(strlen(creds_to_send) + 1);
    strcpy(creds[0], creds_to_send);

    // Create, forward & receive the request
    message = compute_post_request(IP, url, type, creds, 1, NULL, 0, 0);

    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    free(creds[0]);
    free(message);
    return response;
}

char* get_logout_response(char *url, int sockfd, char *type, string cookie) {
    char *message, *response;

    char *char_cookie[1];
    char_cookie[0] = (char*) malloc(cookie.size() + 1);
    strcpy(char_cookie[0], cookie.c_str());

    // Create, forward & receive the request
    message = compute_get_request(IP, url, NULL, char_cookie, 1, 0);

    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    free(char_cookie[0]);
    free(message);
    return response;
}

char* get_book_response(char *url, int sockfd, char *type,
            string cookie, int cookies_count) {
    char *message, *response;
    // Get the credentials
    char *deets_to_send = get_book_details();
    char *deets[1];
    deets[0] = (char *) malloc(strlen(deets_to_send) + 1);
    strcpy(deets[0], deets_to_send);

    char *char_cookie[1];
    char_cookie[0] = (char*) malloc(cookie.size() + 1);
    strcpy(char_cookie[0], cookie.c_str());

    // Create, forward & receive the request
    message = compute_post_request(IP, url, type, deets, 1, char_cookie, cookies_count, 1);

    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    free(deets[0]);
    free(message);
    free(char_cookie[0]);
    return response;
}

char* get_cookie_response(char *url, int sockfd, string cookie, int isAuth) {
    char* message;
    char *char_cookie[1];
    char_cookie[0] = (char*) malloc(cookie.size() + 1);
    strcpy(char_cookie[0], cookie.c_str());

    message = compute_get_request(IP, url, NULL, char_cookie, 1, isAuth);
    send_to_server(sockfd, message);
    char* response = receive_from_server(sockfd);

    free(char_cookie[0]);
    free(message);

    return response;        
}

char* get_delete_response(char *url, int sockfd, string cookie, int isAuth) {
    char* message;
    char *char_cookie[1];
    char_cookie[0] = (char*) malloc(cookie.size() + 1);
    strcpy(char_cookie[0], cookie.c_str());

    message = compute_delete_request(IP, url, NULL, char_cookie, 1, isAuth);
    send_to_server(sockfd, message);
    char* response = receive_from_server(sockfd);

    free(char_cookie[0]);
    free(message);

    return response;        
}

string cookie(char* response) {
    string str_resp(response);
    istringstream tokenize_line(str_resp);
    string token, prev_token = "";

    while (getline(tokenize_line, token, ' ')) {
        if (token.compare(0, 7, "connect") == 0) {
            break;
        }
    }

    return token;
}

string jwt_token(char* response) {
    string str_resp(response);
    istringstream tokenize_line(str_resp);
    string token;

    while (getline(tokenize_line, token, '\n')) {
        if (token.compare(0, 9, "{\"token\":") == 0) {
            break;
        }
    }

    return token;
}

string concat_authorization(string jwt_token) {
    string header = "Authorization: Bearer ";
    json j = json::parse(jwt_token);

    string converted_json = j.dump();
    int jwt_size = converted_json.size() - 12;
    header += converted_json.substr(10, jwt_size);

    return header;
}

int main(int argc, char *argv[]) {
    char *message;
    char *response;
    int sockfd;
    int recv_commands = 1;
    string session_cookie;
    string library_jwt;

    // To be able to use switch instead of if
    unordered_map<string, CommandSet> valid_commands;
    initialize_map(valid_commands);

    char* type = "application/json";
    
    while (recv_commands) {
        // Connect to the server
        sockfd = open_connection(IP, PORT, AF_INET, SOCK_STREAM, 0);

        // Read the current command
        string current_command;
        cin >> current_command;

        // // If the current command is register
        // switch(valid_commands[current_command]) {

        //     case CommandSet::regist : {
        //         cout << get_initial_response("/api/v1/tema/auth/register", sockfd, type);
        //         break;
        //     } 

        //     case CommandSet::login : {
        //         response = get_initial_response("/api/v1/tema/auth/login", sockfd, type);
        //         cout << response;
        //         session_cookie = cookie(response);
        //         free(response);
        //         break;
        //     }

        //     case CommandSet::enter_library : {
        //             char *response = get_cookie_response("/api/v1/tema/library/access",
        //                                 sockfd, session_cookie, 0);
        //             cout << response << "\n";
        //             library_jwt = jwt_token(response);
        //             free(response);
        //         break;
        //     }
        // }

        // If the current command is register
        if (current_command == "register") {
            cout << get_initial_response("/api/v1/tema/auth/register", sockfd, type);

        } else if (current_command == "login") {
            response = get_initial_response("/api/v1/tema/auth/login", sockfd, type);
            cout << response;
            session_cookie = cookie(response);
            free(response);

        } else if (current_command == "enter_library") {
            if (session_cookie.size() != 0) {
                char *response = get_cookie_response("/api/v1/tema/library/access",
                                    sockfd, session_cookie, 0);
                cout << response << "\n";
                library_jwt = jwt_token(response);
                free(response);
            }

        } else if (current_command == "get_books") {
            string auth = "Authorization: Bearer " + library_jwt.substr(10, library_jwt.size() - 12);           
            char *response = get_cookie_response("/api/v1/tema/library/books", sockfd, auth, 1);
            cout << response;
            free(response);

        } else if (current_command == "get_book") {
            string id;
            cout << "id=";
            cin >> id;

            string auth = "Authorization: Bearer " + library_jwt.substr(10, library_jwt.size() - 12);
            string string_url = "/api/v1/tema/library/books/" + id;

            char* url = (char *) malloc(string_url.size() + 1);
            strcpy(url, string_url.c_str());

            char *response = get_cookie_response(url, sockfd, auth, 1);
            cout << response;
            free(response);
            free(url);

        } else if (current_command == "add_book") {
            string auth = "Authorization: Bearer " + library_jwt.substr(10, library_jwt.size() - 12);
            char *response = get_book_response("/api/v1/tema/library/books", sockfd, type, auth, 1);
            cout << response;
            free(response);

        } else if (current_command == "delete_book") {
            string id;
            cout << "id=";
            cin >> id;

            string auth = "Authorization: Bearer " + library_jwt.substr(10, library_jwt.size() - 12);
            string string_url = "/api/v1/tema/library/books/" + id;

            char* url = (char *) malloc(string_url.size() + 1);
            strcpy(url, string_url.c_str());

            char* response = get_delete_response(url, sockfd, auth, 1);
            cout << response;
            free(url);
            free(response);
            
        } else if (current_command == "logout") {
            char* response = get_logout_response("/api/v1/auth/logout", sockfd, type, session_cookie);
            cout << response;

            // Remove token and cookies
            library_jwt = "";
            session_cookie = "";

        } else if (current_command == "exit") {
            recv_commands = 0;
        } else {
            cout << "Invalid command, please retry.\n";
        }

        close_connection(sockfd);
    }

    return 0;
}
