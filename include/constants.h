#ifndef CONSTANTS_H
#define CONSTANTS_H

/* 데이터베이스 관련 상수 */
#define DATABASE_PATH "database/library.db"
#define MAX_SQL_LENGTH 2048

/* 문자열 최대 길이 */
#define MAX_TITLE_LENGTH 255
#define MAX_AUTHOR_LENGTH 127
#define MAX_ISBN_LENGTH 19
#define MAX_PUBLISHER_LENGTH 127
#define MAX_CATEGORY_LENGTH 63
#define MAX_NAME_LENGTH 63
#define MAX_EMAIL_LENGTH 127
#define MAX_PHONE_LENGTH 19
#define MAX_ADDRESS_LENGTH 255
#define MAX_INPUT_SIZE 1024

/* 대출 관련 상수 */
#define DEFAULT_LOAN_DAYS 14
#define MAX_RENEWAL_COUNT 2
#define MAX_BOOKS_PER_MEMBER 5

/* 검색 결과 관련 상수 */
#define INITIAL_SEARCH_CAPACITY 10
#define MAX_SEARCH_RESULTS 1000

/* 성공/실패 반환값 */
#define SUCCESS 0
#define FAILURE -1

/* 불린 값 */
#define TRUE 1
#define FALSE 0

/* 데이터베이스 테이블 이름 */
#define TABLE_BOOKS "books"
#define TABLE_MEMBERS "members"
#define TABLE_LOANS "loans"

/* SQL 쿼리 타입 */
#define QUERY_SELECT 1
#define QUERY_INSERT 2
#define QUERY_UPDATE 3
#define QUERY_DELETE 4

/* 로그 레벨 */
#define LOG_ERROR 1
#define LOG_WARNING 2
#define LOG_INFO 3
#define LOG_DEBUG 4

#endif // CONSTANTS_H
