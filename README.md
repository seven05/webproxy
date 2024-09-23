9/12 ~ 9/23 동안 진행한 proxy lab 과제

1. tiny server web (CSAPP 11 장에 나오는 내용을 그대로 구현하고 숙제 문제까지 적용한 구현)

2. proxy.c 만들기 (이게 메인)

- CASPP 과제에서 시키는 drvier.sh 테스트를 통과하는 기본적인 코드 작성함
- Sequencial, Concurrency, Cache 순서대로 구현함
- drvier.sh에서 동시성과 캐시의 기능을 확인하는 테스트가 부족하다고 생각해서 cmu대학 과제 안내서에 적혀있는 사양을 구현했는지 테스트하는 sh파일을 새로 만듬

3. 추가된 테스트

- 1MB까지 캐시용량을 넘어서 캐시를 저장할때 문제가 있는지 체크
- LRU 정책을 지켜서 캐시를 저장하고있는지 체크
- 100KB가 넘는 웹 객체를 잘 처리하는지 체크
- 캐시가 멀트쓰레드 안정성을 보장하는지 체크

4. 기본 과제 파일과 달라진 파일들 정의

- proxy.c: 새로 만든 drvier_new2.sh 까지 전부 만족하는 최종 프록시 코드
- proxy_sequencial.c: Sequencial만 구현한 기본 proxy 코드
- proxy_concurrency.c: Sequencial 코드에서 멀티쓰레드를 추가한 코드
- proxy_cache.c: Concurrency 코드에서 이중 연결리스트를 사용해서 캐시까지 구현한 코드
- driver.sh: cmu 기본 과제에서 제공하는 기본테스트 sh파일
- driver_new2.sh: 위에서 언급한 여러가지 테스트를 직접 추가한 sh파일
- tiny/cache_test 폴더: driver_new2.sh 에서 실행하는 추가적인 테스트 코드들을 위한 tiny 서버 데이터