#ifndef LOAN_H
#define LOAN_H

#include <sqlite3.h>
#include <time.h>
#include "types.h"
#include "constants.h"

/**
 * @brief 도서를 대출합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param book_id 대출할 도서 ID
 * @param member_id 대출하는 회원 ID
 * @param loan_days 대출 기간 (일수, 0이면 기본값 사용)
 * @return int 성공 시 생성된 대출 ID, 실패 시 FAILURE 반환
 */
int loan_book(sqlite3 *db, int book_id, int member_id, int loan_days);

/**
 * @brief 도서를 반납합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param loan_id 반납할 대출 ID
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int return_book(sqlite3 *db, int loan_id);

/**
 * @brief 도서 ID와 회원 ID로 반납합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param book_id 반납할 도서 ID
 * @param member_id 반납하는 회원 ID
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int return_book_by_ids(sqlite3 *db, int book_id, int member_id);

/**
 * @brief 대출을 연장합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param loan_id 연장할 대출 ID
 * @param extend_days 연장할 일수
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int extend_loan(sqlite3 *db, int loan_id, int extend_days);

/**
 * @brief 대출 ID로 대출 정보를 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param loan_id 조회할 대출 ID
 * @param loan 조회된 대출 정보를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int get_loan_by_id(sqlite3 *db, int loan_id, Loan *loan);

/**
 * @brief 회원의 대출 이력을 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param member_id 회원 ID
 * @param result 조회 결과를 저장할 포인터
 * @param include_returned 반납된 대출도 포함할지 여부
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int get_member_loan_history(sqlite3 *db, int member_id, LoanSearchResult *result, int include_returned);

/**
 * @brief 회원의 현재 대출 목록을 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param member_id 회원 ID
 * @param result 조회 결과를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int get_member_current_loans(sqlite3 *db, int member_id, LoanSearchResult *result);

/**
 * @brief 도서의 대출 이력을 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param book_id 도서 ID
 * @param result 조회 결과를 저장할 포인터
 * @param include_returned 반납된 대출도 포함할지 여부
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int get_book_loan_history(sqlite3 *db, int book_id, LoanSearchResult *result, int include_returned);

/**
 * @brief 연체된 대출 목록을 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param result 조회 결과를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int get_overdue_loans(sqlite3 *db, LoanSearchResult *result);

/**
 * @brief 특정 날짜에 반납 예정인 대출 목록을 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param due_date 반납 예정일
 * @param result 조회 결과를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int get_loans_due_on_date(sqlite3 *db, time_t due_date, LoanSearchResult *result);

/**
 * @brief 현재 대출 중인 모든 대출 목록을 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param result 조회 결과를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int get_current_loans(sqlite3 *db, LoanSearchResult *result);

/**
 * @brief 대출 통계를 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param total_loans 총 대출 수를 저장할 포인터
 * @param current_loans 현재 대출 중인 수를 저장할 포인터
 * @param overdue_loans 연체 중인 수를 저장할 포인터
 * @param returned_loans 반납된 수를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int get_loan_statistics(sqlite3 *db, int *total_loans, int *current_loans, 
                       int *overdue_loans, int *returned_loans);

/**
 * @brief 인기 도서 통계를 조회합니다 (대출 횟수 기준).
 * 
 * @param db 데이터베이스 연결 포인터
 * @param book_ids 도서 ID 배열
 * @param loan_counts 대출 횟수 배열
 * @param max_books 최대 조회할 도서 수
 * @return int 조회된 도서 수, 실패 시 FAILURE 반환
 */
int get_popular_books_by_loans(sqlite3 *db, int *book_ids, int *loan_counts, int max_books);

/**
 * @brief 대출 가능 여부를 확인합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param book_id 도서 ID
 * @param member_id 회원 ID
 * @return int 대출 가능하면 SUCCESS, 불가능하면 FAILURE 반환
 */
int check_loan_availability(sqlite3 *db, int book_id, int member_id);

/**
 * @brief 중복 대출 여부를 확인합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param book_id 도서 ID
 * @param member_id 회원 ID
 * @return int 중복 대출이면 FAILURE, 아니면 SUCCESS 반환
 */
int check_duplicate_loan(sqlite3 *db, int book_id, int member_id);

/**
 * @brief 연체 일수를 계산합니다.
 * 
 * @param due_date 반납 예정일
 * @param return_date 실제 반납일 (현재 시간이면 0 전달)
 * @return int 연체 일수 (음수면 연체 아님)
 */
int calculate_overdue_days(time_t due_date, time_t return_date);

/**
 * @brief 대출 검색 결과 메모리를 초기화합니다.
 * 
 * @param result 초기화할 검색 결과 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int init_loan_search_result(LoanSearchResult *result);

/**
 * @brief 대출 검색 결과 메모리를 해제합니다.
 * 
 * @param result 해제할 검색 결과 포인터
 */
void free_loan_search_result(LoanSearchResult *result);

/**
 * @brief 대출 구조체를 초기화합니다.
 * 
 * @param loan 초기화할 대출 구조체 포인터
 */
void init_loan(Loan *loan);

/**
 * @brief 대출 정보를 검증합니다.
 * 
 * @param loan 검증할 대출 정보
 * @return int 유효하면 SUCCESS, 무효하면 FAILURE 반환
 */
int validate_loan(const Loan *loan);

/**
 * @brief 대출 정보를 출력합니다.
 * 
 * @param db 데이터베이스 연결 포인터 (도서/회원 정보 조회용)
 * @param loan 출력할 대출 정보
 */
void print_loan(sqlite3 *db, const Loan *loan);

/**
 * @brief 대출 목록을 출력합니다.
 * 
 * @param db 데이터베이스 연결 포인터 (도서/회원 정보 조회용)
 * @param result 출력할 대출 검색 결과
 */
void print_loan_list(sqlite3 *db, const LoanSearchResult *result);

/**
 * @brief 대출 통계를 출력합니다.
 * 
 * @param total_loans 총 대출 수
 * @param current_loans 현재 대출 중인 수
 * @param overdue_loans 연체 중인 수
 * @param returned_loans 반납된 수
 */
void print_loan_statistics(int total_loans, int current_loans, int overdue_loans, int returned_loans);

/**
 * @brief 연체 상태를 문자열로 반환합니다.
 * 
 * @param due_date 반납 예정일
 * @param return_date 실제 반납일 (미반납이면 0)
 * @param buffer 결과를 저장할 버퍼
 * @param buffer_size 버퍼 크기
 */
void get_overdue_status_string(time_t due_date, time_t return_date, char *buffer, size_t buffer_size);

#endif // LOAN_H
