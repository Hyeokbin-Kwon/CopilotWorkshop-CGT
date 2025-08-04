#ifndef BOOK_H
#define BOOK_H

#include <sqlite3.h>
#include "types.h"
#include "constants.h"

/**
 * @brief 새 도서를 데이터베이스에 추가합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param book 추가할 도서 정보
 * @return int 성공 시 생성된 도서 ID, 실패 시 FAILURE 반환
 */
int add_book(sqlite3 *db, const Book *book);

/**
 * @brief ID로 도서를 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param book_id 조회할 도서 ID
 * @param book 조회된 도서 정보를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int get_book_by_id(sqlite3 *db, int book_id, Book *book);

/**
 * @brief ISBN으로 도서를 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param isbn 조회할 ISBN
 * @param book 조회된 도서 정보를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int get_book_by_isbn(sqlite3 *db, const char *isbn, Book *book);

/**
 * @brief 제목으로 도서를 검색합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param title 검색할 제목 (부분 검색 가능)
 * @param result 검색 결과를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int search_books_by_title(sqlite3 *db, const char *title, BookSearchResult *result);

/**
 * @brief 저자로 도서를 검색합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param author 검색할 저자 (부분 검색 가능)
 * @param result 검색 결과를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int search_books_by_author(sqlite3 *db, const char *author, BookSearchResult *result);

/**
 * @brief 카테고리로 도서를 검색합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param category 검색할 카테고리
 * @param result 검색 결과를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int search_books_by_category(sqlite3 *db, const char *category, BookSearchResult *result);

/**
 * @brief 도서 정보를 수정합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param book 수정할 도서 정보 (id 필드가 설정되어야 함)
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int update_book(sqlite3 *db, const Book *book);

/**
 * @brief 도서를 삭제합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param book_id 삭제할 도서 ID
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int delete_book(sqlite3 *db, int book_id);

/**
 * @brief 전체 도서 목록을 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param result 조회 결과를 저장할 포인터
 * @param limit 최대 조회 개수 (0이면 전체)
 * @param offset 조회 시작 위치
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int list_all_books(sqlite3 *db, BookSearchResult *result, int limit, int offset);

/**
 * @brief 대출 가능한 도서 목록을 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param result 조회 결과를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int list_available_books(sqlite3 *db, BookSearchResult *result);

/**
 * @brief 인기 도서 목록을 조회합니다 (대출 횟수 기준).
 * 
 * @param db 데이터베이스 연결 포인터
 * @param result 조회 결과를 저장할 포인터
 * @param limit 최대 조회 개수
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int get_popular_books(sqlite3 *db, BookSearchResult *result, int limit);

/**
 * @brief 도서 검색 결과 메모리를 초기화합니다.
 * 
 * @param result 초기화할 검색 결과 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int init_book_search_result(BookSearchResult *result);

/**
 * @brief 도서 검색 결과 메모리를 해제합니다.
 * 
 * @param result 해제할 검색 결과 포인터
 */
void free_book_search_result(BookSearchResult *result);

/**
 * @brief 도서 구조체를 초기화합니다.
 * 
 * @param book 초기화할 도서 구조체 포인터
 */
void init_book(Book *book);

/**
 * @brief 도서 정보를 검증합니다.
 * 
 * @param book 검증할 도서 정보
 * @return int 유효하면 SUCCESS, 무효하면 FAILURE 반환
 */
int validate_book(const Book *book);

/**
 * @brief 도서 정보를 출력합니다.
 * 
 * @param book 출력할 도서 정보
 */
void print_book(const Book *book);

/**
 * @brief 도서 목록을 출력합니다.
 * 
 * @param result 출력할 도서 검색 결과
 */
void print_book_list(const BookSearchResult *result);

#endif // BOOK_H
