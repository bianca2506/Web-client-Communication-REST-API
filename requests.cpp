#include "requests.hpp"
#include "helpers.hpp"
#include <arpa/inet.h>
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdio.h>
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h>     /* read, write, close */

#include "nlohmann/json.hpp"

using json = nlohmann::json;

char *compute_get_request(char *host, char *url, char *token_jwt, char *query_params,
                          char **cookies, int cookies_count)
{
    char *message = new char[BUFLEN];
    char *line = new char[LINELEN];

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL)
    {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    }
    else
    {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL && cookies_count)
    {
        for (int i = 0; i < cookies_count; i++)
        {
            sprintf(line, "Cookie: %s", cookies[i]);
            compute_message(message, line);
        }
    }

    // Add the JWT token
    if (token_jwt != NULL)
    {
        sprintf(line, "Authorization: Bearer %s", token_jwt);
        compute_message(message, line);
    }

    // Step 4: add final new line
    compute_message(message, "");

    // Free memory
    delete[] line;

    return message;
}

char *compute_post_request(char *host, char *url, char *content_type, json js, char *token,
                           char **cookies, int cookies_count)
{
    char *message = new char[BUFLEN];
    char *line = new char[LINELEN];

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);
    
    /* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    sprintf(line, "Content-Length: %ld", strlen(js.dump().c_str()));
    compute_message(message, line);

    // Step 4 (optional): add cookies
    if (cookies != NULL && cookies_count)
    {
        for (int i = 0; i < cookies_count; i++)
        {
            sprintf(line, "Cookie: %s", cookies[i]);
            compute_message(message, line);
        }
    }

    // Add the JWT token
    if (token != NULL)
    {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // Step 5: add new line at end of header
    compute_message(message, "");

    // Step 6: add the actual payload data
    compute_message(message, js.dump().c_str());

    delete[] line;
    return message;
}

char *compute_delete_request(char *host, char *url, char *id, char *token_jwt)
{
    char *message = new char[BUFLEN];
    char *line = new char[LINELEN];

    // Write the method name, URL and id
    sprintf(line, "DELETE %s%s HTTP/1.1", url, id);

    compute_message(message, line);

    // Add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Add the JWT token
    if (token_jwt != NULL)
    {
        sprintf(line, "Authorization: Bearer %s", token_jwt);
        compute_message(message, line);
    }

    // Add new line at end of header
    compute_message(message, "");

    // Free memory
    delete[] line;

    return message;
}
