#ifndef _REQUESTS_
#define _REQUESTS_

#include "nlohmann/json.hpp"

using json = nlohmann::json;

// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(char *host, char *url, char *token_jwt, char *query_params,
                          char **cookies, int cookies_count);

// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(char *host, char *url, char *content_type, json js, char *token,
                           char **cookies, int cookies_count);
                           
char *compute_delete_request(char *host, char *url, char *id, char *token_jwt);

#endif
