#ifndef MEMBER_H
#define MEMBER_H

#include <sqlite3.h>
#include "types.h"
#include "constants.h"

/**
 * @brief 새 회원을 데이터베이스에 등록합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param member 등록할 회원 정보
 * @return int 성공 시 생성된 회원 ID, 실패 시 FAILURE 반환
 */
int add_member(sqlite3 *db, const Member *member);

/**
 * @brief ID로 회원을 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param member_id 조회할 회원 ID
 * @param member 조회된 회원 정보를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int get_member_by_id(sqlite3 *db, int member_id, Member *member);

/**
 * @brief 이메일로 회원을 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param email 조회할 이메일
 * @param member 조회된 회원 정보를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int get_member_by_email(sqlite3 *db, const char *email, Member *member);

/**
 * @brief 이름으로 회원을 검색합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param name 검색할 이름 (부분 검색 가능)
 * @param result 검색 결과를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int search_members_by_name(sqlite3 *db, const char *name, MemberSearchResult *result);

/**
 * @brief 전화번호로 회원을 검색합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param phone 검색할 전화번호 (부분 검색 가능)
 * @param result 검색 결과를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int search_members_by_phone(sqlite3 *db, const char *phone, MemberSearchResult *result);

/**
 * @brief 회원 정보를 수정합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param member 수정할 회원 정보 (id 필드가 설정되어야 함)
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int update_member(sqlite3 *db, const Member *member);

/**
 * @brief 회원을 삭제합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param member_id 삭제할 회원 ID
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int delete_member(sqlite3 *db, int member_id);

/**
 * @brief 회원을 비활성화합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param member_id 비활성화할 회원 ID
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int deactivate_member(sqlite3 *db, int member_id);

/**
 * @brief 회원을 활성화합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param member_id 활성화할 회원 ID
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int activate_member(sqlite3 *db, int member_id);

/**
 * @brief 전체 회원 목록을 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param result 조회 결과를 저장할 포인터
 * @param limit 최대 조회 개수 (0이면 전체)
 * @param offset 조회 시작 위치
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int list_all_members(sqlite3 *db, MemberSearchResult *result, int limit, int offset);

/**
 * @brief 활성 회원 목록을 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param result 조회 결과를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int list_active_members(sqlite3 *db, MemberSearchResult *result);

/**
 * @brief 회원의 대출 통계를 조회합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param member_id 회원 ID
 * @param total_loans 총 대출 횟수를 저장할 포인터
 * @param current_loans 현재 대출 중인 도서 수를 저장할 포인터
 * @param overdue_loans 연체 중인 도서 수를 저장할 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int get_member_loan_stats(sqlite3 *db, int member_id, int *total_loans, 
                         int *current_loans, int *overdue_loans);

/**
 * @brief 회원의 대출 가능 여부를 확인합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param member_id 회원 ID
 * @return int 대출 가능하면 SUCCESS, 불가능하면 FAILURE 반환
 */
int check_member_loan_eligibility(sqlite3 *db, int member_id);

/**
 * @brief 회원 검색 결과 메모리를 초기화합니다.
 * 
 * @param result 초기화할 검색 결과 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE 반환
 */
int init_member_search_result(MemberSearchResult *result);

/**
 * @brief 회원 검색 결과 메모리를 해제합니다.
 * 
 * @param result 해제할 검색 결과 포인터
 */
void free_member_search_result(MemberSearchResult *result);

/**
 * @brief 회원 구조체를 초기화합니다.
 * 
 * @param member 초기화할 회원 구조체 포인터
 */
void init_member(Member *member);

/**
 * @brief 회원 정보를 검증합니다.
 * 
 * @param member 검증할 회원 정보
 * @return int 유효하면 SUCCESS, 무효하면 FAILURE 반환
 */
int validate_member(const Member *member);

/**
 * @brief 이메일 형식을 검증합니다.
 * 
 * @param email 검증할 이메일
 * @return int 유효하면 SUCCESS, 무효하면 FAILURE 반환
 */
int validate_email(const char *email);

/**
 * @brief 전화번호 형식을 검증합니다.
 * 
 * @param phone 검증할 전화번호
 * @return int 유효하면 SUCCESS, 무효하면 FAILURE 반환
 */
int validate_phone(const char *phone);

/**
 * @brief 회원 정보를 출력합니다.
 * 
 * @param member 출력할 회원 정보
 */
void print_member(const Member *member);

/**
 * @brief 회원 목록을 출력합니다.
 * 
 * @param result 출력할 회원 검색 결과
 */
void print_member_list(const MemberSearchResult *result);

/**
 * @brief 회원 대출 통계를 출력합니다.
 * 
 * @param member_id 회원 ID
 * @param total_loans 총 대출 횟수
 * @param current_loans 현재 대출 중인 도서 수
 * @param overdue_loans 연체 중인 도서 수
 */
void print_member_loan_stats(int member_id, int total_loans, int current_loans, int overdue_loans);

#endif // MEMBER_H
