#include <stdio.h>

#include "csapp.h"

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

typedef struct web_obj_t
{
  char file_uri[MAXLINE];
  int content_length;
  char header[MAXLINE];
  char *file_ptr;
  struct web_obj_t *prev, *next;
} web_obj_t;

web_obj_t *find_cache(char *file_uri);
void send_cache(web_obj_t *web_object, int clientfd);
void use_cache(web_obj_t *web_object);
void add_cache(web_obj_t *web_object);

extern web_obj_t *rootp;  
extern web_obj_t *lastp;  
extern int total_cache_size; 
extern pthread_mutex_t cache_mutex;