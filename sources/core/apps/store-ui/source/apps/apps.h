#ifndef APPS_H
#define APPS_H

#include <stddef.h>

typedef struct{
    char* buffer;
    size_t size;
}fetch_apps_data_t;

typedef struct{
    char* url;
    char* name;
    char* description;
}app_info_t;

fetch_apps_data_t* fetch_apps_data(CURL* curl);
app_info_t** find_apps_url_by_name(CURL* curl, char* name);
app_info_t** find_apps_url_by_tag(CURL* curl, char* tag);
void free_app_url_by(app_info_t** data);

#endif // APPS_H