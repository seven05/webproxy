#include <stdio.h>

#include "csapp.h"
#include "cache.h"

web_obj_t *rootp;
web_obj_t *lastp;
int total_cache_size = 0;
pthread_mutex_t cache_mutex;
web_obj_t *find_cache(char *file_uri){ // 캐시에 존재하는지 체크함
  if (!rootp){  // 캐시리스트가 비어있으면
    return NULL;
  }
  web_obj_t *current = rootp;      // 루트부터 탐색
  for(current; current != NULL; current = current->next){
    if (!strcmp(current->file_uri, file_uri)){  // uri가 같은지 검사
      return current;
    }
  }
  return NULL;
}

void send_cache(web_obj_t *web_object, int clientfd){ // 캐시에 있으면 헤더와 파일을 바로 전송함
  Rio_writen(clientfd, web_object->header, strlen(web_object->header));
  Rio_writen(clientfd, web_object->file_ptr, web_object->content_length);
}

void use_cache(web_obj_t *web_object){  //  캐시를 사용하면 사용한 객체를 rootp로 바꿈
  // printf("\nweb_obj: %s\n", web_object->file_uri);
  // printf("rootp: %s\n", rootp->file_uri);
  // printf("lastp: %s\n", lastp->file_uri);
  // printf("web_obj_next: %s\n", web_object->next->file_uri);
  // printf("web_obj_prev: %s\n", web_object->prev->file_uri);
  if (web_object == rootp){ // 이미 루트면
    return;
  }
  if (web_object->next) {   // 다음이 있다면
    if (web_object->prev){  //이전이 있다면
      web_object->prev->next = web_object->next;
    }
    web_object->next->prev = web_object->prev;
  }
  else{
    web_object->prev->next = NULL;
    lastp = web_object->prev;
  }
  web_object->next = rootp;
  web_object->prev = NULL;
  rootp->prev = web_object; 
  rootp = web_object;
  // printf("new_rootp: %s\n", rootp->file_uri);
  // printf("new_rootp_next: %s\n", rootp->next->file_uri);
  // printf("new_rootp_next_prev: %s\n", rootp->next->prev->file_uri);
  // printf("new_lastp: %s\n\n", lastp->file_uri);
}

void add_cache(web_obj_t *web_object){  // 캐시 리스트에 추가할때
  // printf("cachesize=%d\n",total_cache_size);
  // printf("last_old: %s\n", lastp->file_uri);
  // printf("last_old_prev: %s\n", lastp->prev->file_uri);            
  // printf("root_old: %s\n", rootp->file_uri);
  total_cache_size += web_object->content_length;   // 케시의 전체크기를 늘리고
  if (rootp->content_length == 0){  // 캐시리스트에 아무것도없다면
    lastp = web_object; // 추가된 객체가 lastp
  }
  else{   // 기존 루트가 있다면
    web_object->next = rootp;   // 새로 추가될애가 root가 되므로 포인터 조정
    rootp->prev = web_object;   
  }
  rootp = web_object;   // 새로 추가된 애가 무조건 rootp
  // printf("last_new: %s\n", lastp->file_uri);
  // printf("last_new_prev: %s\n", lastp->prev->file_uri);
  // printf("root_new: %s\n", rootp->file_uri);
  while (total_cache_size > MAX_CACHE_SIZE){    // 캐시의 전체 크기가 1MB를 초과한다면 삭제해야함
    // printf("last_delete: %s\n", lastp->file_uri);
    // printf("last_delete_prev: %s\n", lastp->prev->file_uri);
    total_cache_size -= lastp->content_length;  // 길이 갱신
    web_obj_t *temp = lastp->prev;    // 마지막 객체의 이전이 마지막이됨
    free(lastp);     // 마지막이였던 객체를 메모리 할당 해제함
    temp->next = NULL;     // lastp는 next 포인터가 없음
    lastp = temp;
    // printf("last_replace: %s\n", lastp->file_uri);
  }
  // printf("\r\n");
}