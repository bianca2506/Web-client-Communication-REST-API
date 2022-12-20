#include <stdio.h>         /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <iostream>
#include "helpers.hpp"
#include "requests.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

using namespace std;

#define BUFFLEN 100
#define IP_ADDRESS_LENGTH 16
#define CLEN 300
#define JWTLEN 500
#define MESSAGELEN 1000

int main(int argc, char *argv[])
{
    // Disable buffering
    setvbuf(stdout, NULL, _IONBF, BUFFLEN);

    char *message;
    char *response;
    char *buffer = new char[BUFFLEN];
    int login = 0;
    int enter_library = 0;
    int sockfd;
    char *cookies = new char[CLEN];
    char *aux = new char[CLEN];
    char *token_jwt = new char[JWTLEN];
    char *host = new char[IP_ADDRESS_LENGTH];
    
    // Make connection to server
    strcpy(host, "34.241.4.235");
    sockfd = open_connection(host, 8080, AF_INET, SOCK_STREAM, 0);

    while (1)
    {
        fgets(buffer, BUFFLEN, stdin);

        if (strcmp(buffer, "register\n") == 0)
        {
            // If the user is not logged in, register him
            if (login == 0)
            {
                char *username = new char[BUFFLEN];
                char *password = new char[BUFFLEN];

                cout << "username=";
                fgets(username, BUFFLEN, stdin);
                username[strlen(username) - 1] = '\0';

                cout << "password=";
                fgets(password, BUFFLEN, stdin);
                password[strlen(password) - 1] = '\0';

                sockfd = open_connection(host, 8080, AF_INET, SOCK_STREAM, 0);

                // Compute the request
                json js;
                js["username"] = username;
                js["password"] = password;

                char *url = new char[BUFFLEN];
                strcpy(url, "/api/v1/tema/auth/register");

                char *payload = new char[BUFFLEN];
                strcpy(payload, "application/json");

                message = compute_post_request(host, url, payload, js, NULL, NULL, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // Display a message
                if (strstr(response, "HTTP/1.1 201") != NULL)
                {
                    cout << "User '" << username << "' registered successfully!" << endl;
                }
                else if (strstr(response, "HTTP/1.1 400") != NULL)
                {
                    cout << "Error: User '" << username << "' already exists!" << endl;
                }
                else if (strstr(response, "HTTP/1.1 429") != NULL)
                {
                    cout << "Error: Too many requests!" << endl;
                }
                else
                {
                    cout << "Error while registering user '" << username << "'!" << endl;
                }

                // Free memory
                delete[] username;
                delete[] password;
                delete[] url;
                delete[] payload;
            }
            else // If the user is logged in, display a message
            {
                cout << "Error: You have to logout first!" << endl;
            }
        }
        else if (strcmp(buffer, "login\n") == 0)
        {
            // If the user is not logged in, login him
            if (login == 0)
            {
                char *username = new char[BUFFLEN];
                char *password = new char[BUFFLEN];

                cout << "username=";
                fgets(username, BUFFLEN, stdin);
                username[strlen(username) - 1] = '\0';

                cout << "password=";
                fgets(password, BUFFLEN, stdin);
                password[strlen(password) - 1] = '\0';

                sockfd = open_connection(host, 8080, AF_INET, SOCK_STREAM, 0);

                json js;
                js["username"] = username;
                js["password"] = password;

                char *url = new char[BUFFLEN];
                strcpy(url, "/api/v1/tema/auth/login");

                char *payload = new char[BUFFLEN];
                strcpy(payload, "application/json");

                message = compute_post_request(host, url, payload, js, NULL, NULL, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                if (strstr(response, "HTTP/1.1 400") != NULL)
                {
                    cout << "Error: Invalid username or password!" << endl;
                    continue;
                }
                
                // Get the cookies
                if (strstr(response, "HTTP/1.1 200") != NULL)
                {
                    cout << "User '" << username << "' logged in successfully!" << endl;
                    login = 1;
                    strcpy(aux, strstr(response, "Set-Cookie:"));
                    int i = 12;
                    while (aux[i] != ';' || aux[i + 1] != ' ')
                    {
                        cookies[i - 12] = aux[i];
                        i++;
                    }
                    cookies[i - 12] = '\0';
                }

                // Free memory
                delete[] username;
                delete[] password;
                delete[] url;
                delete[] payload;
            }
            else // If the user is logged in, display a message
            {
                cout << "Error: You are already logged in!" << endl;
            }
        }
        else if (strcmp(buffer, "enter_library\n") == 0)
        {
            // If the user is logged in, enter the library
            if (login == 1)
            {
                char *url = new char[BUFFLEN];
                strcpy(url, "/api/v1/tema/library/access");

                sockfd = open_connection(host, 8080, AF_INET, SOCK_STREAM, 0);

                // Get the cookie
                char **cookie = new char *[1];
                cookie[0] = new char[CLEN];
                strcpy(cookie[0], cookies);

                message = compute_get_request(host, url, NULL, NULL, cookie, 1);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // Compute the token
                char *token = new char[JWTLEN];
                strcpy(token, strstr(response, "token"));
                if (token != NULL)
                {
                    cout << "You have successfully entered the library!" << endl;
                    enter_library = 1;
                    char *aux1 = strtok(token + 8, "\"");
                    strcpy(token_jwt, token + 8);
                }
                else
                {
                    cout << "Error while entering the library!" << endl;
                }

                // Free memory
                delete[] url;
                delete[] cookie[0];
                delete[] cookie;
            }
            else // If the user is not logged in, display a message
            {
                cout << "Error: You have to login first!" << endl;
            }
        }
        else if (strcmp(buffer, "get_books\n") == 0)
        {
            // If the user is in the library and logged in,
            // get the books
            if (enter_library == 1 && login == 1)
            {
                char *url = new char[BUFFLEN];
                strcpy(url, "/api/v1/tema/library/books");

                sockfd = open_connection(host, 8080, AF_INET, SOCK_STREAM, 0);

                // Get the cookie
                char **cookie = new char *[1];
                cookie[0] = new char[CLEN];
                strcpy(cookie[0], cookies);

                message = compute_get_request(host, url, token_jwt, NULL, cookie, 1);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // Display the books
                if (strstr(response, "HTTP/1.1 200") != NULL)
                {
                    json js = json::parse(strstr(response, "\r\n["));
                    if (js.size() > 0)
                    {
                        cout << "Books:" << endl;
                        cout << "[" << endl;
                        for (auto &i : js)
                        {
                            cout << "\t{" << endl;
                            cout << "\t\t\"id\": " << i["id"] << "," << endl;
                            cout << "\t\t\"title\": " << i["title"] << endl;
                            cout << "\t}" << endl;
                        }
                        cout << "]" << endl;
                    }
                    else
                    {
                        cout << "No books in the library!" << endl;
                    }
                }
                else
                {
                    cout << "Error while getting books from the library!" << endl;
                }

                // Free memory
                delete[] url;
                delete[] cookie[0];
                delete[] cookie;
            }
            // If the user is not in the library, display a message
            else if (enter_library == 0)
            {
                cout << "Error: You have to enter the library first!" << endl;
            }
            // If the user is not logged in, display a message
            else if (login == 0)
            {
                cout << "Error: You have to login first!" << endl;
            }
        }
        else if (strcmp(buffer, "get_book\n") == 0)
        {
            // If the user is in the library and logged in,
            // get the book
            if (enter_library == 1 && login == 1)
            {
                char *id = new char[20];
                cout << "id=";
                fgets(id, 20, stdin);
                id[strlen(id) - 1] = '\0';

                // If the id is not a number
                if (id[0] < '0' || id[0] > '9')
                {
                    cout << "Error: Invalid id!" << endl;
                    continue;
                }

                char *url = new char[BUFFLEN];
                strcpy(url, "/api/v1/tema/library/books/");
                strcat(url, id);

                sockfd = open_connection(host, 8080, AF_INET, SOCK_STREAM, 0);

                // Get the cookie
                char **cookie = new char *[1];
                cookie[0] = new char[CLEN];
                strcpy(cookie[0], cookies);

                message = compute_get_request(host, url, token_jwt, NULL, cookie, 1);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                
                // Display the book
                if (strstr(response, "HTTP/1.1 404") != NULL)
                {
                    cout << "The book with the id " << id << " does not exist!" << endl;
                }
                else if (strstr(response, "HTTP/1.1 200") != NULL)
                {
                    json js = json::parse(strstr(response, "[{"));
                    auto &i = js[0];
                    cout << "The book with the id " << id << " is:" << endl;
                    cout << "[" << endl;
                    cout << "\t{" << endl;
                    cout << "\t\t\"title\": " << i["title"] << "," << endl;
                    cout << "\t\t\"author\": " << i["author"] << "," << endl;
                    cout << "\t\t\"publisher\": " << i["publisher"] << "," << endl;
                    cout << "\t\t\"genre\": " << i["genre"] << "," << endl;
                    cout << "\t\t\"page_count\": " << i["page_count"] << endl;
                    cout << "\t}" << endl;
                    cout << "]" << endl;
                }
                else
                {
                    cout << "Error while getting book with the id " << id << "!" << endl;
                }

                // Free memory
                delete[] url;
                delete[] cookie[0];
                delete[] cookie;
            }
            // If the user is not in the library, display a message
            else if (enter_library == 0)
            {
                cout << "Error: You have to enter the library first!" << endl;
            }
            // If the user is not logged in, display a message
            else if (login == 0)
            {
                cout << "Error: You have to login first!" << endl;
            }
        }
        else if (strcmp(buffer, "add_book\n") == 0)
        {
            // If the user is in the library and logged in,
            // add the book
            if (enter_library == 1 && login == 1)
            {
                char *title = new char[20];
                cout << "title=";
                fgets(title, 20, stdin);
                title[strlen(title) - 1] = '\0';

                string author;
                cout << "author=";
                getline(cin, author);
                // If the author is not a string
                if (author.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ") != string::npos)
                {
                    cout << "Error: Invalid author!" << endl;
                    continue;
                }

                string genre;
                cout << "genre=";
                getline(cin, genre);
                // If genre is not a string
                if (genre.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ") != string::npos)
                {
                    cout << "Error: Invalid genre!" << endl;
                    continue;
                }

                char *page_count = new char[20];
                cout << "page_count=";
                fgets(page_count, 20, stdin);
                page_count[strlen(page_count) - 1] = '\0';
                // If page_count is not a number
                if (page_count[0] < '0' || page_count[0] > '9')
                {
                    cout << "Error: Invalid page_count!" << endl;
                    continue;
                }

                string publisher;
                cout << "publisher=";
                getline(cin, publisher);
                // If publisher is not a string
                if (publisher.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ") != string::npos)
                {
                    cout << "Error: Invalid publisher!" << endl;
                    continue;
                }
                char *url = new char[BUFFLEN];
                strcpy(url, "/api/v1/tema/library/books");

                char *payload = new char[BUFFLEN];
                strcpy(payload, "application/json");

                sockfd = open_connection(host, 8080, AF_INET, SOCK_STREAM, 0);

                // Get the cookie
                char **cookie = new char *[1];
                cookie[0] = new char[CLEN];
                strcpy(cookie[0], cookies);

                json js;
                js["title"] = title;
                js["author"] = author;
                js["genre"] = genre;
                js["page_count"] = page_count;
                js["publisher"] = publisher;

                message = compute_post_request(host, url, payload, js, token_jwt, NULL, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                if (strstr(response, "HTTP/1.1 200") != NULL)
                {
                    cout << "The book with the title '" << title << "' was added!" << endl;
                }
                else
                {
                    cout << "Error while adding book with the title " << title << "!" << endl;
                }

                // Free memory
                delete[] url;
                delete[] payload;
                delete[] cookie[0];
                delete[] cookie;
            }
            // If the user is not in the library, display a message
            else if (enter_library == 0)
            {
                cout << "Error: You have to enter the library first!" << endl;
                continue;
            }
            // If the user is not logged in, display a message
            else if (login == 0)
            {
                cout << "Error: You have to login first!" << endl;
                continue;
            }
        }
        else if (strcmp(buffer, "delete_book\n") == 0)
        {

            if (enter_library == 1 && login == 1)
            {

                char *id = new char[20];
                cout << "id=";
                fgets(id, 20, stdin);
                id[strlen(id) - 1] = '\0';

                // If id isn't a number
                if (id[0] < '0' || id[0] > '9')
                {
                    cout << "Error: Invalid id!" << endl;
                    continue;
                }
                sockfd = open_connection(host, 8080, AF_INET, SOCK_STREAM, 0);

                char *url = new char[BUFFLEN];
                strcpy(url, "/api/v1/tema/library/books/");

                message = compute_delete_request(host, url, id, token_jwt);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                if (strstr(response, "HTTP/1.1 404") != NULL)
                {
                    cout << "The book with the id " << id << " does not exist!" << endl;
                }
                else if (strstr(response, "HTTP/1.1 200") != NULL)
                {
                    cout << "The book with the id " << id << " was deleted!" << endl;
                }
                else
                {
                    cout << "Error while deleting book with the id " << id << "!" << endl;
                }

                // Free memory
                delete[] url;
            }
            // If the user is not logged in, display a message
            else if (login == 0)
            {
                cout << "ERROR: You have to be logged in." << endl;
                continue;
            }
            // If the user is not in the library, display a message
            else if (enter_library == 0)
            {
                cout << "ERROR: You have to enter the library first." << endl;
                continue;
            }
        }
        else if (strcmp(buffer, "logout\n") == 0)
        {
            // If the user is logged in, logout
            if (login == 1)
            {
                char *url = new char[BUFFLEN];
                strcpy(url, "/api/v1/tema/auth/logout");

                sockfd = open_connection(host, 8080, AF_INET, SOCK_STREAM, 0);

                // Get the cookie
                char **cookie = new char *[1];
                cookie[0] = new char[CLEN];
                strcpy(cookie[0], cookies);

                message = compute_get_request(host, url, NULL, NULL, cookie, 1);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                if (strstr(response, "HTTP/1.1 200") != NULL)
                {
                    cout << "You have been successfully logged out!" << endl;
                    login = 0;
                    enter_library = 0;
                    memset(cookies, 0, CLEN);
                    memset(token_jwt, 0, CLEN);
                }
                else
                {
                    cout << "Error while logging out!" << endl;
                }

                // Free memory
                delete[] url;
                delete[] cookie[0];
                delete[] cookie;
            }
            else if (login == 0)
            {
                cout << "You are not logged in!" << endl;
            }
        }
        else if (strcmp(buffer, "exit\n") == 0)
        {
            break;
        }
        else
        {
            cout << "Error: Invalid command!" << endl;
        }
    }

    // Free memory
    delete[] host;
    delete[] buffer;
    delete[] cookies;
    delete[] aux;
    delete[] token_jwt;
    delete[] message;
    delete[] response;

    // Close the socket
    close_connection(sockfd);

    return 0;
}
