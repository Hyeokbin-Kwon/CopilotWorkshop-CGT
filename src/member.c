#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sqlite3.h>
#include "../include/member.h"
#include "../include/database.h"
#include "../include/constants.h"

static int member_callback(void *data, int argc, char **argv, char **azColName);
static int count_callback(void *data, int argc, char **argv, char **azColName);

int add_member(sqlite3 *db, const Member *member) {
    if (!db || !member) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    if (validate_member(member) != SUCCESS) {
        fprintf(stderr, "유효하지 않은 회원 정보입니다.\n");
        return FAILURE;
    }
    
    // 이메일 중복 확인
    Member existing_member;
    if (get_member_by_email(db, member->email, &existing_member) == SUCCESS) {
        fprintf(stderr, "이미 등록된 이메일입니다: %s\n", member->email);
        return FAILURE;
    }
    
    const char *sql = 
        "INSERT INTO members (name, email, phone, address, is_active) "
        "VALUES (?, ?, ?, ?, ?);";
    
    sqlite3_stmt *stmt = NULL;
    int result = FAILURE;
    
    if (database_prepare_statement(db, sql, &stmt) != SUCCESS) {
        return FAILURE;
    }
    
    // 매개변수 바인딩
    sqlite3_bind_text(stmt, 1, member->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, member->email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, member->phone, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, member->address, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, member->is_active);
    
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        result = database_get_last_insert_id(db);
    } else {
        fprintf(stderr, "회원 등록 실패: %s\n", sqlite3_errmsg(db));
    }
    
    sqlite3_finalize(stmt);
    return result;
}

int get_member_by_id(sqlite3 *db, int member_id, Member *member) {
    if (!db || !member || member_id <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    const char *sql = 
        "SELECT id, name, email, phone, address, registration_date, "
        "is_active, created_at, updated_at "
        "FROM members WHERE id = ?;";
    
    sqlite3_stmt *stmt = NULL;
    int result = FAILURE;
    
    if (database_prepare_statement(db, sql, &stmt) != SUCCESS) {
        return FAILURE;
    }
    
    sqlite3_bind_int(stmt, 1, member_id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        member->id = sqlite3_column_int(stmt, 0);
        strncpy(member->name, (const char*)sqlite3_column_text(stmt, 1), MAX_NAME_LENGTH);
        member->name[MAX_NAME_LENGTH] = '\0';
        strncpy(member->email, (const char*)sqlite3_column_text(stmt, 2), MAX_EMAIL_LENGTH);
        member->email[MAX_EMAIL_LENGTH] = '\0';
        strncpy(member->phone, (const char*)sqlite3_column_text(stmt, 3), MAX_PHONE_LENGTH);
        member->phone[MAX_PHONE_LENGTH] = '\0';
        strncpy(member->address, (const char*)sqlite3_column_text(stmt, 4), MAX_ADDRESS_LENGTH);
        member->address[MAX_ADDRESS_LENGTH] = '\0';
        member->registration_date = (time_t)sqlite3_column_int64(stmt, 5);
        member->is_active = sqlite3_column_int(stmt, 6);
        member->created_at = (time_t)sqlite3_column_int64(stmt, 7);
        member->updated_at = (time_t)sqlite3_column_int64(stmt, 8);
        
        result = SUCCESS;
    }
    
    sqlite3_finalize(stmt);
    return result;
}

int get_member_by_email(sqlite3 *db, const char *email, Member *member) {
    if (!db || !email || !member) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    const char *sql = 
        "SELECT id, name, email, phone, address, registration_date, "
        "is_active, created_at, updated_at "
        "FROM members WHERE email = ?;";
    
    sqlite3_stmt *stmt = NULL;
    int result = FAILURE;
    
    if (database_prepare_statement(db, sql, &stmt) != SUCCESS) {
        return FAILURE;
    }
    
    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        member->id = sqlite3_column_int(stmt, 0);
        strncpy(member->name, (const char*)sqlite3_column_text(stmt, 1), MAX_NAME_LENGTH);
        member->name[MAX_NAME_LENGTH] = '\0';
        strncpy(member->email, (const char*)sqlite3_column_text(stmt, 2), MAX_EMAIL_LENGTH);
        member->email[MAX_EMAIL_LENGTH] = '\0';
        strncpy(member->phone, (const char*)sqlite3_column_text(stmt, 3), MAX_PHONE_LENGTH);
        member->phone[MAX_PHONE_LENGTH] = '\0';
        strncpy(member->address, (const char*)sqlite3_column_text(stmt, 4), MAX_ADDRESS_LENGTH);
        member->address[MAX_ADDRESS_LENGTH] = '\0';
        member->registration_date = (time_t)sqlite3_column_int64(stmt, 5);
        member->is_active = sqlite3_column_int(stmt, 6);
        member->created_at = (time_t)sqlite3_column_int64(stmt, 7);
        member->updated_at = (time_t)sqlite3_column_int64(stmt, 8);
        
        result = SUCCESS;
    }
    
    sqlite3_finalize(stmt);
    return result;
}

int search_members_by_name(sqlite3 *db, const char *name, MemberSearchResult *result) {
    if (!db || !name || !result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), 
        "SELECT id, name, email, phone, address, registration_date, "
        "is_active, created_at, updated_at "
        "FROM members WHERE name LIKE '%%%s%%' ORDER BY name;", name);
    
    return sqlite3_exec(db, sql, member_callback, result, NULL) == SQLITE_OK ? SUCCESS : FAILURE;
}

int search_members_by_phone(sqlite3 *db, const char *phone, MemberSearchResult *result) {
    if (!db || !phone || !result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), 
        "SELECT id, name, email, phone, address, registration_date, "
        "is_active, created_at, updated_at "
        "FROM members WHERE phone LIKE '%%%s%%' ORDER BY name;", phone);
    
    return sqlite3_exec(db, sql, member_callback, result, NULL) == SQLITE_OK ? SUCCESS : FAILURE;
}

int update_member(sqlite3 *db, const Member *member) {
    if (!db || !member || member->id <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    if (validate_member(member) != SUCCESS) {
        fprintf(stderr, "유효하지 않은 회원 정보입니다.\n");
        return FAILURE;
    }
    
    // 이메일 중복 확인 (자신 제외)
    Member existing_member;
    if (get_member_by_email(db, member->email, &existing_member) == SUCCESS) {
        if (existing_member.id != member->id) {
            fprintf(stderr, "이미 등록된 이메일입니다: %s\n", member->email);
            return FAILURE;
        }
    }
    
    const char *sql = 
        "UPDATE members SET name = ?, email = ?, phone = ?, address = ?, "
        "is_active = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?;";
    
    sqlite3_stmt *stmt = NULL;
    int result = FAILURE;
    
    if (database_prepare_statement(db, sql, &stmt) != SUCCESS) {
        return FAILURE;
    }
    
    // 매개변수 바인딩
    sqlite3_bind_text(stmt, 1, member->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, member->email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, member->phone, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, member->address, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, member->is_active);
    sqlite3_bind_int(stmt, 6, member->id);
    
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        result = SUCCESS;
    } else {
        fprintf(stderr, "회원 정보 수정 실패: %s\n", sqlite3_errmsg(db));
    }
    
    sqlite3_finalize(stmt);
    return result;
}

int delete_member(sqlite3 *db, int member_id) {
    if (!db || member_id <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    // 대출 중인 도서가 있는지 확인
    const char *check_sql = 
        "SELECT COUNT(*) FROM loans WHERE member_id = ? AND is_returned = 0;";
    
    sqlite3_stmt *check_stmt = NULL;
    int loan_count = 0;
    
    if (database_prepare_statement(db, check_sql, &check_stmt) != SUCCESS) {
        return FAILURE;
    }
    
    sqlite3_bind_int(check_stmt, 1, member_id);
    
    if (sqlite3_step(check_stmt) == SQLITE_ROW) {
        loan_count = sqlite3_column_int(check_stmt, 0);
    }
    
    sqlite3_finalize(check_stmt);
    
    if (loan_count > 0) {
        fprintf(stderr, "대출 중인 도서가 있는 회원은 삭제할 수 없습니다.\n");
        return FAILURE;
    }
    
    // 회원 삭제
    const char *sql = "DELETE FROM members WHERE id = ?;";
    sqlite3_stmt *stmt = NULL;
    int result = FAILURE;
    
    if (database_prepare_statement(db, sql, &stmt) != SUCCESS) {
        return FAILURE;
    }
    
    sqlite3_bind_int(stmt, 1, member_id);
    
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        result = SUCCESS;
    } else {
        fprintf(stderr, "회원 삭제 실패: %s\n", sqlite3_errmsg(db));
    }
    
    sqlite3_finalize(stmt);
    return result;
}

int deactivate_member(sqlite3 *db, int member_id) {
    if (!db || member_id <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    const char *sql = 
        "UPDATE members SET is_active = 0, updated_at = CURRENT_TIMESTAMP WHERE id = ?;";
    
    sqlite3_stmt *stmt = NULL;
    int result = FAILURE;
    
    if (database_prepare_statement(db, sql, &stmt) != SUCCESS) {
        return FAILURE;
    }
    
    sqlite3_bind_int(stmt, 1, member_id);
    
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        result = SUCCESS;
    } else {
        fprintf(stderr, "회원 비활성화 실패: %s\n", sqlite3_errmsg(db));
    }
    
    sqlite3_finalize(stmt);
    return result;
}

int activate_member(sqlite3 *db, int member_id) {
    if (!db || member_id <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    const char *sql = 
        "UPDATE members SET is_active = 1, updated_at = CURRENT_TIMESTAMP WHERE id = ?;";
    
    sqlite3_stmt *stmt = NULL;
    int result = FAILURE;
    
    if (database_prepare_statement(db, sql, &stmt) != SUCCESS) {
        return FAILURE;
    }
    
    sqlite3_bind_int(stmt, 1, member_id);
    
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        result = SUCCESS;
    } else {
        fprintf(stderr, "회원 활성화 실패: %s\n", sqlite3_errmsg(db));
    }
    
    sqlite3_finalize(stmt);
    return result;
}

int list_all_members(sqlite3 *db, MemberSearchResult *result, int limit, int offset) {
    if (!db || !result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    char sql[MAX_SQL_LENGTH];
    if (limit > 0) {
        snprintf(sql, sizeof(sql), 
            "SELECT id, name, email, phone, address, registration_date, "
            "is_active, created_at, updated_at "
            "FROM members ORDER BY name LIMIT %d OFFSET %d;", limit, offset);
    } else {
        snprintf(sql, sizeof(sql), 
            "SELECT id, name, email, phone, address, registration_date, "
            "is_active, created_at, updated_at "
            "FROM members ORDER BY name;");
    }
    
    return sqlite3_exec(db, sql, member_callback, result, NULL) == SQLITE_OK ? SUCCESS : FAILURE;
}

int list_active_members(sqlite3 *db, MemberSearchResult *result) {
    if (!db || !result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    const char *sql = 
        "SELECT id, name, email, phone, address, registration_date, "
        "is_active, created_at, updated_at "
        "FROM members WHERE is_active = 1 ORDER BY name;";
    
    return sqlite3_exec(db, sql, member_callback, result, NULL) == SQLITE_OK ? SUCCESS : FAILURE;
}

int get_member_loan_stats(sqlite3 *db, int member_id, int *total_loans, 
                         int *current_loans, int *overdue_loans) {
    if (!db || member_id <= 0 || !total_loans || !current_loans || !overdue_loans) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    *total_loans = 0;
    *current_loans = 0;
    *overdue_loans = 0;
    
    // 총 대출 횟수
    const char *total_sql = "SELECT COUNT(*) FROM loans WHERE member_id = ?;";
    sqlite3_stmt *total_stmt = NULL;
    
    if (database_prepare_statement(db, total_sql, &total_stmt) == SUCCESS) {
        sqlite3_bind_int(total_stmt, 1, member_id);
        if (sqlite3_step(total_stmt) == SQLITE_ROW) {
            *total_loans = sqlite3_column_int(total_stmt, 0);
        }
        sqlite3_finalize(total_stmt);
    }
    
    // 현재 대출 중인 도서 수
    const char *current_sql = 
        "SELECT COUNT(*) FROM loans WHERE member_id = ? AND is_returned = 0;";
    sqlite3_stmt *current_stmt = NULL;
    
    if (database_prepare_statement(db, current_sql, &current_stmt) == SUCCESS) {
        sqlite3_bind_int(current_stmt, 1, member_id);
        if (sqlite3_step(current_stmt) == SQLITE_ROW) {
            *current_loans = sqlite3_column_int(current_stmt, 0);
        }
        sqlite3_finalize(current_stmt);
    }
    
    // 연체 중인 도서 수
    const char *overdue_sql = 
        "SELECT COUNT(*) FROM loans WHERE member_id = ? AND is_returned = 0 "
        "AND due_date < datetime('now');";
    sqlite3_stmt *overdue_stmt = NULL;
    
    if (database_prepare_statement(db, overdue_sql, &overdue_stmt) == SUCCESS) {
        sqlite3_bind_int(overdue_stmt, 1, member_id);
        if (sqlite3_step(overdue_stmt) == SQLITE_ROW) {
            *overdue_loans = sqlite3_column_int(overdue_stmt, 0);
        }
        sqlite3_finalize(overdue_stmt);
    }
    
    return SUCCESS;
}

int check_member_loan_eligibility(sqlite3 *db, int member_id) {
    if (!db || member_id <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    // 회원이 활성 상태인지 확인
    Member member;
    if (get_member_by_id(db, member_id, &member) != SUCCESS) {
        fprintf(stderr, "회원 정보를 찾을 수 없습니다.\n");
        return FAILURE;
    }
    
    if (!member.is_active) {
        fprintf(stderr, "비활성 회원은 대출할 수 없습니다.\n");
        return FAILURE;
    }
    
    // 현재 대출 중인 도서 수 확인
    const char *current_sql = 
        "SELECT COUNT(*) FROM loans WHERE member_id = ? AND is_returned = 0;";
    sqlite3_stmt *current_stmt = NULL;
    int current_loans = 0;
    
    if (database_prepare_statement(db, current_sql, &current_stmt) == SUCCESS) {
        sqlite3_bind_int(current_stmt, 1, member_id);
        if (sqlite3_step(current_stmt) == SQLITE_ROW) {
            current_loans = sqlite3_column_int(current_stmt, 0);
        }
        sqlite3_finalize(current_stmt);
    }
    
    if (current_loans >= MAX_BOOKS_PER_MEMBER) {
        fprintf(stderr, "최대 대출 가능 권수를 초과했습니다. (현재: %d권, 최대: %d권)\n", 
                current_loans, MAX_BOOKS_PER_MEMBER);
        return FAILURE;
    }
    
    // 연체 도서가 있는지 확인
    const char *overdue_sql = 
        "SELECT COUNT(*) FROM loans WHERE member_id = ? AND is_returned = 0 "
        "AND due_date < datetime('now');";
    sqlite3_stmt *overdue_stmt = NULL;
    int overdue_loans = 0;
    
    if (database_prepare_statement(db, overdue_sql, &overdue_stmt) == SUCCESS) {
        sqlite3_bind_int(overdue_stmt, 1, member_id);
        if (sqlite3_step(overdue_stmt) == SQLITE_ROW) {
            overdue_loans = sqlite3_column_int(overdue_stmt, 0);
        }
        sqlite3_finalize(overdue_stmt);
    }
    
    if (overdue_loans > 0) {
        fprintf(stderr, "연체 중인 도서가 있어 대출할 수 없습니다. (연체: %d권)\n", overdue_loans);
        return FAILURE;
    }
    
    return SUCCESS;
}

int init_member_search_result(MemberSearchResult *result) {
    if (!result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    result->members = malloc(sizeof(Member) * INITIAL_SEARCH_CAPACITY);
    if (!result->members) {
        fprintf(stderr, "메모리 할당 실패\n");
        return FAILURE;
    }
    
    result->count = 0;
    result->capacity = INITIAL_SEARCH_CAPACITY;
    
    return SUCCESS;
}

void free_member_search_result(MemberSearchResult *result) {
    if (result && result->members) {
        free(result->members);
        result->members = NULL;
        result->count = 0;
        result->capacity = 0;
    }
}

void init_member(Member *member) {
    if (!member) {
        return;
    }
    
    memset(member, 0, sizeof(Member));
    member->id = 0;
    member->is_active = TRUE;
    member->registration_date = time(NULL);
    member->created_at = time(NULL);
    member->updated_at = time(NULL);
}

int validate_member(const Member *member) {
    if (!member) {
        return FAILURE;
    }
    
    if (strlen(member->name) == 0 || strlen(member->name) > MAX_NAME_LENGTH) {
        return FAILURE;
    }
    
    if (validate_email(member->email) != SUCCESS) {
        return FAILURE;
    }
    
    if (member->phone && strlen(member->phone) > 0) {
        if (validate_phone(member->phone) != SUCCESS) {
            return FAILURE;
        }
    }
    
    if (member->address && strlen(member->address) > MAX_ADDRESS_LENGTH) {
        return FAILURE;
    }
    
    return SUCCESS;
}

int validate_email(const char *email) {
    if (!email || strlen(email) == 0 || strlen(email) > MAX_EMAIL_LENGTH) {
        return FAILURE;
    }
    
    // 기본적인 이메일 형식 검증 (@ 포함 여부)
    const char *at_pos = strchr(email, '@');
    if (!at_pos || at_pos == email || at_pos == email + strlen(email) - 1) {
        return FAILURE;
    }
    
    // 점(.) 포함 여부 확인 (@ 이후)
    if (!strchr(at_pos, '.')) {
        return FAILURE;
    }
    
    return SUCCESS;
}

int validate_phone(const char *phone) {
    if (!phone || strlen(phone) == 0 || strlen(phone) > MAX_PHONE_LENGTH) {
        return FAILURE;
    }
    
    // 숫자, 하이픈, 공백만 허용
    for (int i = 0; phone[i] != '\0'; i++) {
        if (!isdigit(phone[i]) && phone[i] != '-' && phone[i] != ' ' && phone[i] != '(' && phone[i] != ')') {
            return FAILURE;
        }
    }
    
    return SUCCESS;
}

void print_member(const Member *member) {
    if (!member) {
        return;
    }
    
    printf("==========================================\n");
    printf("회원 ID: %d\n", member->id);
    printf("이름: %s\n", member->name);
    printf("이메일: %s\n", member->email);
    printf("전화번호: %s\n", member->phone);
    printf("주소: %s\n", member->address);
    printf("가입일: %s", ctime(&member->registration_date));
    printf("상태: %s\n", member->is_active ? "활성" : "비활성");
    printf("==========================================\n");
}

void print_member_list(const MemberSearchResult *result) {
    if (!result || !result->members) {
        printf("검색 결과가 없습니다.\n");
        return;
    }
    
    printf("\n총 %d명의 회원이 검색되었습니다.\n\n", result->count);
    
    for (int i = 0; i < result->count; i++) {
        printf("%d. ", i + 1);
        print_member(&result->members[i]);
        printf("\n");
    }
}

void print_member_loan_stats(int member_id, int total_loans, int current_loans, int overdue_loans) {
    printf("==========================================\n");
    printf("회원 ID %d 대출 통계\n", member_id);
    printf("총 대출 횟수: %d회\n", total_loans);
    printf("현재 대출 중: %d권\n", current_loans);
    printf("연체 중: %d권\n", overdue_loans);
    printf("==========================================\n");
}

// 콜백 함수들
static int member_callback(void *data, int argc, char **argv, char **azColName) {
    MemberSearchResult *result = (MemberSearchResult*)data;
    
    if (!result) {
        return SQLITE_ABORT;
    }
    
    // 용량 확장이 필요한 경우
    if (result->count >= result->capacity) {
        int new_capacity = result->capacity * 2;
        if (new_capacity > MAX_SEARCH_RESULTS) {
            new_capacity = MAX_SEARCH_RESULTS;
        }
        
        if (result->count >= new_capacity) {
            return SQLITE_ABORT; // 최대 검색 결과 수 초과
        }
        
        Member *new_members = realloc(result->members, sizeof(Member) * new_capacity);
        if (!new_members) {
            return SQLITE_ABORT;
        }
        
        result->members = new_members;
        result->capacity = new_capacity;
    }
    
    Member *member = &result->members[result->count];
    init_member(member);
    
    // 데이터 복사
    if (argv[0]) member->id = atoi(argv[0]);
    if (argv[1]) {
        strncpy(member->name, argv[1], MAX_NAME_LENGTH);
        member->name[MAX_NAME_LENGTH] = '\0';
    }
    if (argv[2]) {
        strncpy(member->email, argv[2], MAX_EMAIL_LENGTH);
        member->email[MAX_EMAIL_LENGTH] = '\0';
    }
    if (argv[3]) {
        strncpy(member->phone, argv[3], MAX_PHONE_LENGTH);
        member->phone[MAX_PHONE_LENGTH] = '\0';
    }
    if (argv[4]) {
        strncpy(member->address, argv[4], MAX_ADDRESS_LENGTH);
        member->address[MAX_ADDRESS_LENGTH] = '\0';
    }
    if (argv[5]) member->registration_date = (time_t)atoll(argv[5]);
    if (argv[6]) member->is_active = atoi(argv[6]);
    if (argv[7]) member->created_at = (time_t)atoll(argv[7]);
    if (argv[8]) member->updated_at = (time_t)atoll(argv[8]);
    
    result->count++;
    return SQLITE_OK;
}

static int count_callback(void *data, int argc, char **argv, char **azColName) {
    int *count = (int*)data;
    if (argc > 0 && argv[0]) {
        *count = atoi(argv[0]);
    }
    return SQLITE_OK;
}
