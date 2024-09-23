#include <stdio.h>
#include "csapp.h"
#include "cache.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 104900

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";
void doit(int fd);
void *thread(void *vargp);

int main(int argc, char **argv) {
  int listenfd, *connfdp;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  pthread_t tid;
  rootp = (web_obj_t *)calloc(1, sizeof(web_obj_t));
  lastp = (web_obj_t *)calloc(1, sizeof(web_obj_t));
  pthread_mutex_init(&cache_mutex, NULL);  // 기본 속성으로 뮤텍스 초기화

  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfdp = Malloc(sizeof(int));
    *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept
    Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE,
        port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    Pthread_create(&tid,NULL,thread,connfdp);                                              //line:netp:tiny:close
  }
  // // printf("%s", user_agent_hdr);
  // return 0;
}

void *thread(void *vargp){
  int connfd = *((int *)vargp);
  Pthread_detach(pthread_self());
  Free(vargp);
  doit(connfd);
  Close(connfd);
  return NULL;
}

void doit(int fd){
  rio_t rio;
  char host[MAXLINE], port[MAXLINE], file_uri[MAXLINE];
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  Rio_readinitb(&rio, fd);
  if (!Rio_readlineb(&rio, buf, MAXLINE))  //line:netp:doit:readrequest
    return;
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  // "http://" 부분을 건너뛰기 위해 7번째 문자부터 시작
  char *host_start = uri + 7;
  // ':' 문자를 찾아서 포트 시작 위치 파악
  char *port_start = strchr(host_start, ':');
  if (port_start != NULL) {
    *port_start = '\0';  // ':'을 NULL로 바꿔서 host 추출
    strcpy(host, host_start);
    // '/' 문자를 찾아서 파일 URI 시작 위치 파악
    char *file_start = strchr(port_start + 1, '/');
    if (file_start != NULL) {
        *file_start = '\0';  // '/'을 NULL로 바꿔서 port 추출
        strcpy(port, port_start + 1); // ':' 다음부터 port 추출
        strcpy(file_uri, file_start + 1); // '/' 다음부터 file URI 추출
    } else {
        strcpy(port, port_start + 1); // '/'가 없을 경우 포트만 추출
        strcpy(file_uri, "");  // file URI가 없을 경우 빈 문자열
    }
  }

  pthread_mutex_lock(&cache_mutex);  // 캐시에 접근하기 전에 잠금
  web_obj_t *request_cache = find_cache(file_uri);
  if(request_cache){
    send_cache(request_cache,fd);
    use_cache(request_cache);
    pthread_mutex_unlock(&cache_mutex);    
    return;
  }
  pthread_mutex_unlock(&cache_mutex);
  // printf("Host: %s\n", host);
  // printf("Port: %s\n", port);
  // printf("File URI: %s\n", file_uri);
  
  int serverfd, filesize;
  char tmp[MAXLINE];
  char *filep;
  serverfd = open_clientfd(host,port);
  printf("serverfd = %d\n",serverfd);
  if((serverfd) < 0){
    char error_head[MAXLINE];
    // char error_body[MAXLINE];
    sprintf(error_head, "HTTP/1.1 503 Service Unavailable\r\n");
    sprintf(error_head, "%sContent-Type: text/html\r\n\r\n", error_head);
    // sprintf(error_body, "<html><title>Tiny Error</title>\r\n");
    // sprintf(error_body, "%s<body>\r\n", error_body);
    // sprintf(error_body, "%s <p>The server is currently unable to handle the request.</p>\r\n", error_body);
    // sprintf(error_body, "%s</body>\r\n", error_body);
    // sprintf(error_body, "%s</html>\r\n", error_body);
    Rio_writen(fd,error_head,strlen(error_head));
    // Rio_writen(fd,error_body,strlen(error_body));
    memset(error_head, 0, sizeof(error_head));
    return;
  }
  Rio_readinitb(&rio,serverfd);
  sprintf(tmp, "GET /%s HTTP/1.0\r\n", file_uri);
  sprintf(tmp, "%sAccept: */*\r\n",tmp);
  sprintf(tmp, "%sConnection: close\r\n",tmp);
  sprintf(tmp, "%sHost: %s\r\n",tmp ,host);
  sprintf(tmp, "%s%s\r\n",tmp,user_agent_hdr);
  Rio_writen(serverfd,tmp,strlen(tmp));
  char tmp2[MAXLINE];
  while(strcmp(tmp, "\r\n")){
    Rio_readlineb(&rio,tmp,MAXLINE);
    if(strcmp(tmp,"Connection: close\r\n")){
      sprintf(tmp2,"%s%s",tmp2,tmp);
      // Fputs(tmp2,stdout);
    }else{
      sprintf(tmp2,"%sProxy-%s",tmp2,tmp);
    }
    if(strstr(tmp,"Content-length")){
      sscanf(tmp, "Content-length: %d", &filesize);
      printf("filesize = %d\n",filesize);
    }
    // Fputs(tmp,stdout);
  }
  memset(tmp, 0, sizeof(tmp));
  Fputs(tmp2,stdout);
  filep = (char*)Malloc(filesize);
  Rio_readnb(&rio, filep, filesize); // Rio_readn(serverfd, srcp, filesize) 을 쓰면 헤더 첫부분부터 다시읽어서 망함
  Rio_writen(fd,tmp2,strlen(tmp2));
  Rio_writen(fd, filep, filesize);
  
  if (filesize <= MAX_OBJECT_SIZE){
    web_obj_t *web_object = (web_obj_t *)calloc(1, sizeof(web_obj_t));
    web_object->file_ptr = filep;
    web_object->content_length = filesize;
    strcpy(web_object->file_uri, file_uri);                                 
    strcpy(web_object->header,tmp2);
    pthread_mutex_lock(&cache_mutex);  // 캐시에 접근하기 전에 잠금
    add_cache(web_object);
    pthread_mutex_unlock(&cache_mutex);
  }
  else{
    free(filep);
  }
  memset(tmp2, 0, sizeof(tmp2));
  Close(serverfd);
}