#ifndef TYPES_H
#define TYPES_H

#include <time.h>

/**
 * @brief 도서 정보를 저장하는 구조체
 */
typedef struct {
    int id;                    /**< 도서 ID (기본키) */
    char title[256];           /**< 도서 제목 */
    char author[128];          /**< 저자 */
    char isbn[20];             /**< ISBN */
    char publisher[128];       /**< 출판사 */
    int publication_year;      /**< 출판년도 */
    int total_copies;          /**< 총 보유 권수 */
    int available_copies;      /**< 대출 가능 권수 */
    char category[64];         /**< 카테고리 */
    time_t created_at;         /**< 등록일 */
    time_t updated_at;         /**< 수정일 */
} Book;

/**
 * @brief 회원 정보를 저장하는 구조체
 */
typedef struct {
    int id;                    /**< 회원 ID (기본키) */
    char name[64];             /**< 회원 이름 */
    char email[128];           /**< 이메일 */
    char phone[20];            /**< 전화번호 */
    char address[256];         /**< 주소 */
    time_t registration_date;  /**< 가입일 */
    int is_active;             /**< 활성 상태 (1: 활성, 0: 비활성) */
    time_t created_at;         /**< 등록일 */
    time_t updated_at;         /**< 수정일 */
} Member;

/**
 * @brief 대출 정보를 저장하는 구조체
 */
typedef struct {
    int id;                    /**< 대출 ID (기본키) */
    int book_id;               /**< 도서 ID (외래키) */
    int member_id;             /**< 회원 ID (외래키) */
    time_t loan_date;          /**< 대출일 */
    time_t due_date;           /**< 반납 예정일 */
    time_t return_date;        /**< 실제 반납일 (NULL이면 미반납) */
    int is_returned;           /**< 반납 상태 (1: 반납, 0: 미반납) */
    int renewal_count;         /**< 연장 횟수 */
    time_t created_at;         /**< 등록일 */
    time_t updated_at;         /**< 수정일 */
} Loan;

/**
 * @brief 검색 결과를 위한 구조체
 */
typedef struct {
    Book *books;               /**< 도서 배열 */
    int count;                 /**< 결과 개수 */
    int capacity;              /**< 배열 용량 */
} BookSearchResult;

/**
 * @brief 회원 검색 결과를 위한 구조체
 */
typedef struct {
    Member *members;           /**< 회원 배열 */
    int count;                 /**< 결과 개수 */
    int capacity;              /**< 배열 용량 */
} MemberSearchResult;

/**
 * @brief 대출 검색 결과를 위한 구조체
 */
typedef struct {
    Loan *loans;               /**< 대출 배열 */
    int count;                 /**< 결과 개수 */
    int capacity;              /**< 배열 용량 */
} LoanSearchResult;

#endif // TYPES_H
