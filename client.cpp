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
#include "functions.h"

#define IP "3.8.116.10"
#define PORT 8080

using namespace std;
using json = nlohmann::json;

enum CommandSet {
    regist, login, enter_library, get_books, get_book, add_book,
    delete_book, logout, ext
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
    commands["exit"] = CommandSet::ext;
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

        // If the current command is register
        switch(valid_commands[current_command]) {

            case CommandSet::regist : {
                cout << get_initial_response("/api/v1/tema/auth/register", sockfd, type) << "\n";
                break;
            } 

            case CommandSet::login : {
                response = get_initial_response("/api/v1/tema/auth/login", sockfd, type);
                cout << response << "\n";
                session_cookie = cookie(response);
                free(response);
                break;
            }

            case CommandSet::enter_library : {
                if (session_cookie.size() != 0) {
                    char *response = get_cookie_response("/api/v1/tema/library/access",
                                        sockfd, session_cookie, 0);
                    cout << response << "\n";
                    library_jwt = jwt_token(response);
                    free(response);
                } else {
                    cout << "No previous login found. Please retry!\n\n";
                }
                break;
            }

            case CommandSet::get_books : {
                if (library_jwt.size() != 0) {
                    string auth = "Authorization: Bearer " + library_jwt.substr(10, library_jwt.size() - 12);           
                    char *response = get_cookie_response("/api/v1/tema/library/books", sockfd, auth, 1);
                    cout << response << "\n";
                    free(response);
                } else {
                    cout << "No token found. Please retry!\n\n";
                }
                break;
            }

            case CommandSet::get_book : {
                if (library_jwt.size() != 0) {
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
                } else {
                    cout << "No token found. Please retry!\n\n";
                }
                break;
            }

            case CommandSet::add_book : {
                if (library_jwt.size() != 0) {
                    string auth = "Authorization: Bearer " + library_jwt.substr(10, library_jwt.size() - 12);
                    char *response = get_book_response("/api/v1/tema/library/books", sockfd, type, auth, 1);
                    cout << response;
                    free(response);
                } else {
                    cout << "No token found. Please retry!\n\n";
                }
                break;
            }

            case CommandSet::delete_book : {
                if (library_jwt.size() != 0) {
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
                } else {
                    cout << "No token found. Please retry!\n\n";
                }
                break;
            }

            case CommandSet::logout : {
                if (session_cookie.size() != 0) {
                    char* response = get_logout_response("/api/v1/auth/logout", sockfd, type, session_cookie);
                    cout << response;

                    // Remove token and cookies
                    library_jwt = "";
                    session_cookie = "";
                    cout << "The current session has ended.\n\n";
                } else {
                    cout << "No login found. Please login first!\n\n";
                }
                break;
            }

            case CommandSet::ext : {
                recv_commands = 0;
                break;
            }

            default : {
                cout << "Invalid command. Please retry!\n\n";
                break;
            }
        }
        close_connection(sockfd);
    }
    return 0;
}
