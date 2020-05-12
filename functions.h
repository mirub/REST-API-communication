#ifndef _FUNCTIONS_
#define _FUNCTIONS_

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


#endif