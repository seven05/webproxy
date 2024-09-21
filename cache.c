#include <stdio.h>

#include "csapp.h"
#include "cache.h"

web_obj_t *rootp;
web_obj_t *lastp;
int total_cache_size = 0;

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
  if (web_object == rootp){ // 이미 루트면
    return;
  }
  if (web_object->next) {   // 다음이 있다면
    web_obj_t *prev_obj = web_object->prev;
    web_obj_t *next_obj = web_object->next;
    if (prev_obj){  //이전이 있다면
      web_object->prev->next = next_obj;
    }
    web_object->next->prev = prev_obj;
  }
  else {
    web_object->prev->next = NULL; 
  }
  web_object->next = rootp; 
  rootp = web_object;
}

void add_cache(web_obj_t *web_object){  // 캐시 리스트에 추가할때
  total_cache_size += web_object->content_length;   // 케시의 전체크기를 늘리고
  while (total_cache_size > MAX_CACHE_SIZE){    // 캐시의 전체 크기가 1MB를 초과한다면 삭제해야함
    total_cache_size -= lastp->content_length;  // 길이 갱신
    lastp = lastp->prev;    // 마지막 객체의 이전이 마지막이됨
    free(lastp->next);     // 마지막이였던 객체를 메모리 할당 해제함 
    lastp->next = NULL;     // lastp는 next 포인터가 없음
  }
  if (!rootp){  // 캐시리스트에 아무것도없다면
    lastp = web_object; // 추가된 객체가 lastp
  }
  if (rootp){   // 기존 루트가 있다면
    web_object->next = rootp;   // 새로 추가될애가 root가 되므로 포인터 조정
    rootp->prev = web_object;   
  }
  rootp = web_object;   // 새로 추가된 애가 무조건 rootp
}