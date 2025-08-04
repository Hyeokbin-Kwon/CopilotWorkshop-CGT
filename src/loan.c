#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>
#include "../include/loan.h"
#include "../include/book.h"
#include "../include/member.h"
#include "../include/database.h"
#include "../include/constants.h"

static int loan_callback(void *data, int argc, char **argv, char **azColName);
static int count_callback(void *data, int argc, char **argv, char **azColName);
static int popular_books_callback(void *data, int argc, char **argv, char **azColName);

typedef struct {
    int *book_ids;
    int *loan_counts;
    int max_books;
    int current_count;
} PopularBooksData;

int loan_book(sqlite3 *db, int book_id, int member_id, int loan_days) {
    if (!db || book_id <= 0 || member_id <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    if (loan_days <= 0) {
        loan_days = DEFAULT_LOAN_DAYS;
    }
    
    // 대출 가능 여부 확인
    if (check_loan_availability(db, book_id, member_id) != SUCCESS) {
        return FAILURE;
    }
    
    // 트랜잭션 시작
    if (database_begin_transaction(db) != SUCCESS) {
        return FAILURE;
    }
    
    // 대출 기록 추가
    const char *loan_sql = 
        "INSERT INTO loans (book_id, member_id, due_date) "
        "VALUES (?, ?, datetime('now', '+%d days'));";
    
    char sql_buffer[MAX_SQL_LENGTH];
    snprintf(sql_buffer, sizeof(sql_buffer), loan_sql, loan_days);
    
    sqlite3_stmt *loan_stmt = NULL;
    int loan_id = FAILURE;
    
    if (database_prepare_statement(db, sql_buffer, &loan_stmt) != SUCCESS) {
        database_rollback_transaction(db);
        return FAILURE;
    }
    
    sqlite3_bind_int(loan_stmt, 1, book_id);
    sqlite3_bind_int(loan_stmt, 2, member_id);
    
    if (sqlite3_step(loan_stmt) == SQLITE_DONE) {
        loan_id = database_get_last_insert_id(db);
    } else {
        fprintf(stderr, "대출 기록 추가 실패: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(loan_stmt);
        database_rollback_transaction(db);
        return FAILURE;
    }
    
    sqlite3_finalize(loan_stmt);
    
    // 도서의 대출 가능 권수 감소
    const char *update_book_sql = 
        "UPDATE books SET available_copies = available_copies - 1 "
        "WHERE id = ? AND available_copies > 0;";
    
    sqlite3_stmt *update_stmt = NULL;
    
    if (database_prepare_statement(db, update_book_sql, &update_stmt) != SUCCESS) {
        database_rollback_transaction(db);
        return FAILURE;
    }
    
    sqlite3_bind_int(update_stmt, 1, book_id);
    
    if (sqlite3_step(update_stmt) != SQLITE_DONE) {
        fprintf(stderr, "도서 대출 가능 권수 업데이트 실패: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(update_stmt);
        database_rollback_transaction(db);
        return FAILURE;
    }
    
    sqlite3_finalize(update_stmt);
    
    // 트랜잭션 커밋
    if (database_commit_transaction(db) != SUCCESS) {
        return FAILURE;
    }
    
    return loan_id;
}

int return_book(sqlite3 *db, int loan_id) {
    if (!db || loan_id <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    // 대출 정보 조회
    Loan loan;
    if (get_loan_by_id(db, loan_id, &loan) != SUCCESS) {
        fprintf(stderr, "대출 정보를 찾을 수 없습니다.\n");
        return FAILURE;
    }
    
    if (loan.is_returned) {
        fprintf(stderr, "이미 반납된 도서입니다.\n");
        return FAILURE;
    }
    
    // 트랜잭션 시작
    if (database_begin_transaction(db) != SUCCESS) {
        return FAILURE;
    }
    
    // 대출 기록 업데이트 (반납 처리)
    const char *return_sql = 
        "UPDATE loans SET return_date = CURRENT_TIMESTAMP, is_returned = 1, "
        "updated_at = CURRENT_TIMESTAMP WHERE id = ?;";
    
    sqlite3_stmt *return_stmt = NULL;
    
    if (database_prepare_statement(db, return_sql, &return_stmt) != SUCCESS) {
        database_rollback_transaction(db);
        return FAILURE;
    }
    
    sqlite3_bind_int(return_stmt, 1, loan_id);
    
    if (sqlite3_step(return_stmt) != SQLITE_DONE) {
        fprintf(stderr, "반납 처리 실패: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(return_stmt);
        database_rollback_transaction(db);
        return FAILURE;
    }
    
    sqlite3_finalize(return_stmt);
    
    // 도서의 대출 가능 권수 증가
    const char *update_book_sql = 
        "UPDATE books SET available_copies = available_copies + 1 "
        "WHERE id = ?;";
    
    sqlite3_stmt *update_stmt = NULL;
    
    if (database_prepare_statement(db, update_book_sql, &update_stmt) != SUCCESS) {
        database_rollback_transaction(db);
        return FAILURE;
    }
    
    sqlite3_bind_int(update_stmt, 1, loan.book_id);
    
    if (sqlite3_step(update_stmt) != SQLITE_DONE) {
        fprintf(stderr, "도서 대출 가능 권수 업데이트 실패: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(update_stmt);
        database_rollback_transaction(db);
        return FAILURE;
    }
    
    sqlite3_finalize(update_stmt);
    
    // 트랜잭션 커밋
    if (database_commit_transaction(db) != SUCCESS) {
        return FAILURE;
    }
    
    return SUCCESS;
}

int return_book_by_ids(sqlite3 *db, int book_id, int member_id) {
    if (!db || book_id <= 0 || member_id <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    // 해당 대출 기록 찾기
    const char *find_sql = 
        "SELECT id FROM loans WHERE book_id = ? AND member_id = ? AND is_returned = 0 "
        "ORDER BY loan_date DESC LIMIT 1;";
    
    sqlite3_stmt *find_stmt = NULL;
    int loan_id = 0;
    
    if (database_prepare_statement(db, find_sql, &find_stmt) != SUCCESS) {
        return FAILURE;
    }
    
    sqlite3_bind_int(find_stmt, 1, book_id);
    sqlite3_bind_int(find_stmt, 2, member_id);
    
    if (sqlite3_step(find_stmt) == SQLITE_ROW) {
        loan_id = sqlite3_column_int(find_stmt, 0);
    }
    
    sqlite3_finalize(find_stmt);
    
    if (loan_id == 0) {
        fprintf(stderr, "해당 도서에 대한 대출 기록을 찾을 수 없습니다.\n");
        return FAILURE;
    }
    
    return return_book(db, loan_id);
}

int extend_loan(sqlite3 *db, int loan_id, int extend_days) {
    if (!db || loan_id <= 0 || extend_days <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    // 대출 정보 조회
    Loan loan;
    if (get_loan_by_id(db, loan_id, &loan) != SUCCESS) {
        fprintf(stderr, "대출 정보를 찾을 수 없습니다.\n");
        return FAILURE;
    }
    
    if (loan.is_returned) {
        fprintf(stderr, "이미 반납된 도서는 연장할 수 없습니다.\n");
        return FAILURE;
    }
    
    if (loan.renewal_count >= MAX_RENEWAL_COUNT) {
        fprintf(stderr, "최대 연장 횟수를 초과했습니다. (현재: %d회, 최대: %d회)\n", 
                loan.renewal_count, MAX_RENEWAL_COUNT);
        return FAILURE;
    }
    
    // 연체 상태 확인
    time_t current_time = time(NULL);
    if (loan.due_date < current_time) {
        fprintf(stderr, "연체된 도서는 연장할 수 없습니다.\n");
        return FAILURE;
    }
    
    // 대출 연장
    const char *extend_sql = 
        "UPDATE loans SET due_date = datetime(due_date, '+%d days'), "
        "renewal_count = renewal_count + 1, updated_at = CURRENT_TIMESTAMP "
        "WHERE id = ?;";
    
    char sql_buffer[MAX_SQL_LENGTH];
    snprintf(sql_buffer, sizeof(sql_buffer), extend_sql, extend_days);
    
    sqlite3_stmt *extend_stmt = NULL;
    
    if (database_prepare_statement(db, sql_buffer, &extend_stmt) != SUCCESS) {
        return FAILURE;
    }
    
    sqlite3_bind_int(extend_stmt, 1, loan_id);
    
    if (sqlite3_step(extend_stmt) == SQLITE_DONE) {
        sqlite3_finalize(extend_stmt);
        return SUCCESS;
    } else {
        fprintf(stderr, "대출 연장 실패: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(extend_stmt);
        return FAILURE;
    }
}

int get_loan_by_id(sqlite3 *db, int loan_id, Loan *loan) {
    if (!db || !loan || loan_id <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    const char *sql = 
        "SELECT id, book_id, member_id, loan_date, due_date, return_date, "
        "is_returned, renewal_count, created_at, updated_at "
        "FROM loans WHERE id = ?;";
    
    sqlite3_stmt *stmt = NULL;
    int result = FAILURE;
    
    if (database_prepare_statement(db, sql, &stmt) != SUCCESS) {
        return FAILURE;
    }
    
    sqlite3_bind_int(stmt, 1, loan_id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        loan->id = sqlite3_column_int(stmt, 0);
        loan->book_id = sqlite3_column_int(stmt, 1);
        loan->member_id = sqlite3_column_int(stmt, 2);
        
        const char *loan_date_str = (const char*)sqlite3_column_text(stmt, 3);
        const char *due_date_str = (const char*)sqlite3_column_text(stmt, 4);
        const char *return_date_str = (const char*)sqlite3_column_text(stmt, 5);
        
        // 문자열을 time_t로 변환 (SQLite의 datetime을 파싱)
        loan->loan_date = loan_date_str ? (time_t)sqlite3_column_int64(stmt, 3) : 0;
        loan->due_date = due_date_str ? (time_t)sqlite3_column_int64(stmt, 4) : 0;
        loan->return_date = return_date_str ? (time_t)sqlite3_column_int64(stmt, 5) : 0;
        
        loan->is_returned = sqlite3_column_int(stmt, 6);
        loan->renewal_count = sqlite3_column_int(stmt, 7);
        loan->created_at = (time_t)sqlite3_column_int64(stmt, 8);
        loan->updated_at = (time_t)sqlite3_column_int64(stmt, 9);
        
        result = SUCCESS;
    }
    
    sqlite3_finalize(stmt);
    return result;
}

int get_member_loan_history(sqlite3 *db, int member_id, LoanSearchResult *result, int include_returned) {
    if (!db || member_id <= 0 || !result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    char sql[MAX_SQL_LENGTH];
    if (include_returned) {
        snprintf(sql, sizeof(sql), 
            "SELECT id, book_id, member_id, loan_date, due_date, return_date, "
            "is_returned, renewal_count, created_at, updated_at "
            "FROM loans WHERE member_id = %d ORDER BY loan_date DESC;", member_id);
    } else {
        snprintf(sql, sizeof(sql), 
            "SELECT id, book_id, member_id, loan_date, due_date, return_date, "
            "is_returned, renewal_count, created_at, updated_at "
            "FROM loans WHERE member_id = %d AND is_returned = 0 ORDER BY loan_date DESC;", member_id);
    }
    
    return sqlite3_exec(db, sql, loan_callback, result, NULL) == SQLITE_OK ? SUCCESS : FAILURE;
}

int get_member_current_loans(sqlite3 *db, int member_id, LoanSearchResult *result) {
    return get_member_loan_history(db, member_id, result, FALSE);
}

int get_book_loan_history(sqlite3 *db, int book_id, LoanSearchResult *result, int include_returned) {
    if (!db || book_id <= 0 || !result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    char sql[MAX_SQL_LENGTH];
    if (include_returned) {
        snprintf(sql, sizeof(sql), 
            "SELECT id, book_id, member_id, loan_date, due_date, return_date, "
            "is_returned, renewal_count, created_at, updated_at "
            "FROM loans WHERE book_id = %d ORDER BY loan_date DESC;", book_id);
    } else {
        snprintf(sql, sizeof(sql), 
            "SELECT id, book_id, member_id, loan_date, due_date, return_date, "
            "is_returned, renewal_count, created_at, updated_at "
            "FROM loans WHERE book_id = %d AND is_returned = 0 ORDER BY loan_date DESC;", book_id);
    }
    
    return sqlite3_exec(db, sql, loan_callback, result, NULL) == SQLITE_OK ? SUCCESS : FAILURE;
}

int get_overdue_loans(sqlite3 *db, LoanSearchResult *result) {
    if (!db || !result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    const char *sql = 
        "SELECT id, book_id, member_id, loan_date, due_date, return_date, "
        "is_returned, renewal_count, created_at, updated_at "
        "FROM loans WHERE is_returned = 0 AND due_date < datetime('now') "
        "ORDER BY due_date ASC;";
    
    return sqlite3_exec(db, sql, loan_callback, result, NULL) == SQLITE_OK ? SUCCESS : FAILURE;
}

int get_loans_due_on_date(sqlite3 *db, time_t due_date, LoanSearchResult *result) {
    if (!db || !result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    char date_str[32];
    struct tm *tm_info = localtime(&due_date);
    strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_info);
    
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), 
        "SELECT id, book_id, member_id, loan_date, due_date, return_date, "
        "is_returned, renewal_count, created_at, updated_at "
        "FROM loans WHERE is_returned = 0 AND date(due_date) = '%s' "
        "ORDER BY due_date ASC;", date_str);
    
    return sqlite3_exec(db, sql, loan_callback, result, NULL) == SQLITE_OK ? SUCCESS : FAILURE;
}

int get_current_loans(sqlite3 *db, LoanSearchResult *result) {
    if (!db || !result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    const char *sql = 
        "SELECT id, book_id, member_id, loan_date, due_date, return_date, "
        "is_returned, renewal_count, created_at, updated_at "
        "FROM loans WHERE is_returned = 0 ORDER BY loan_date DESC;";
    
    return sqlite3_exec(db, sql, loan_callback, result, NULL) == SQLITE_OK ? SUCCESS : FAILURE;
}

int get_loan_statistics(sqlite3 *db, int *total_loans, int *current_loans, 
                       int *overdue_loans, int *returned_loans) {
    if (!db || !total_loans || !current_loans || !overdue_loans || !returned_loans) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    *total_loans = 0;
    *current_loans = 0;
    *overdue_loans = 0;
    *returned_loans = 0;
    
    // 총 대출 수
    const char *total_sql = "SELECT COUNT(*) FROM loans;";
    sqlite3_exec(db, total_sql, count_callback, total_loans, NULL);
    
    // 현재 대출 중인 수
    const char *current_sql = "SELECT COUNT(*) FROM loans WHERE is_returned = 0;";
    sqlite3_exec(db, current_sql, count_callback, current_loans, NULL);
    
    // 연체 중인 수
    const char *overdue_sql = 
        "SELECT COUNT(*) FROM loans WHERE is_returned = 0 AND due_date < datetime('now');";
    sqlite3_exec(db, overdue_sql, count_callback, overdue_loans, NULL);
    
    // 반납된 수
    const char *returned_sql = "SELECT COUNT(*) FROM loans WHERE is_returned = 1;";
    sqlite3_exec(db, returned_sql, count_callback, returned_loans, NULL);
    
    return SUCCESS;
}

int get_popular_books_by_loans(sqlite3 *db, int *book_ids, int *loan_counts, int max_books) {
    if (!db || !book_ids || !loan_counts || max_books <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    PopularBooksData data;
    data.book_ids = book_ids;
    data.loan_counts = loan_counts;
    data.max_books = max_books;
    data.current_count = 0;
    
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), 
        "SELECT book_id, COUNT(*) as loan_count "
        "FROM loans GROUP BY book_id "
        "ORDER BY loan_count DESC LIMIT %d;", max_books);
    
    if (sqlite3_exec(db, sql, popular_books_callback, &data, NULL) == SQLITE_OK) {
        return data.current_count;
    }
    
    return FAILURE;
}

int check_loan_availability(sqlite3 *db, int book_id, int member_id) {
    if (!db || book_id <= 0 || member_id <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    // 도서 존재 및 대출 가능 여부 확인
    Book book;
    if (get_book_by_id(db, book_id, &book) != SUCCESS) {
        fprintf(stderr, "도서 정보를 찾을 수 없습니다.\n");
        return FAILURE;
    }
    
    if (book.available_copies <= 0) {
        fprintf(stderr, "대출 가능한 도서가 없습니다.\n");
        return FAILURE;
    }
    
    // 회원 대출 자격 확인
    if (check_member_loan_eligibility(db, member_id) != SUCCESS) {
        return FAILURE;
    }
    
    // 중복 대출 확인
    if (check_duplicate_loan(db, book_id, member_id) != SUCCESS) {
        return FAILURE;
    }
    
    return SUCCESS;
}

int check_duplicate_loan(sqlite3 *db, int book_id, int member_id) {
    if (!db || book_id <= 0 || member_id <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    const char *sql = 
        "SELECT COUNT(*) FROM loans WHERE book_id = ? AND member_id = ? AND is_returned = 0;";
    
    sqlite3_stmt *stmt = NULL;
    int count = 0;
    
    if (database_prepare_statement(db, sql, &stmt) != SUCCESS) {
        return FAILURE;
    }
    
    sqlite3_bind_int(stmt, 1, book_id);
    sqlite3_bind_int(stmt, 2, member_id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    
    if (count > 0) {
        fprintf(stderr, "이미 대출 중인 도서입니다.\n");
        return FAILURE;
    }
    
    return SUCCESS;
}

int calculate_overdue_days(time_t due_date, time_t return_date) {
    time_t current_time = (return_date == 0) ? time(NULL) : return_date;
    
    // 일 단위로 계산
    double diff_seconds = difftime(current_time, due_date);
    int diff_days = (int)(diff_seconds / (24 * 60 * 60));
    
    return diff_days;
}

int init_loan_search_result(LoanSearchResult *result) {
    if (!result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    result->loans = malloc(sizeof(Loan) * INITIAL_SEARCH_CAPACITY);
    if (!result->loans) {
        fprintf(stderr, "메모리 할당 실패\n");
        return FAILURE;
    }
    
    result->count = 0;
    result->capacity = INITIAL_SEARCH_CAPACITY;
    
    return SUCCESS;
}

void free_loan_search_result(LoanSearchResult *result) {
    if (result && result->loans) {
        free(result->loans);
        result->loans = NULL;
        result->count = 0;
        result->capacity = 0;
    }
}

void init_loan(Loan *loan) {
    if (!loan) {
        return;
    }
    
    memset(loan, 0, sizeof(Loan));
    loan->id = 0;
    loan->book_id = 0;
    loan->member_id = 0;
    loan->loan_date = time(NULL);
    loan->due_date = 0;
    loan->return_date = 0;
    loan->is_returned = FALSE;
    loan->renewal_count = 0;
    loan->created_at = time(NULL);
    loan->updated_at = time(NULL);
}

int validate_loan(const Loan *loan) {
    if (!loan) {
        return FAILURE;
    }
    
    if (loan->book_id <= 0 || loan->member_id <= 0) {
        return FAILURE;
    }
    
    if (loan->loan_date <= 0 || loan->due_date <= 0) {
        return FAILURE;
    }
    
    if (loan->due_date <= loan->loan_date) {
        return FAILURE;
    }
    
    if (loan->is_returned && loan->return_date <= 0) {
        return FAILURE;
    }
    
    if (loan->renewal_count < 0 || loan->renewal_count > MAX_RENEWAL_COUNT) {
        return FAILURE;
    }
    
    return SUCCESS;
}

void print_loan(sqlite3 *db, const Loan *loan) {
    if (!loan) {
        return;
    }
    
    Book book;
    Member member;
    
    printf("==========================================\n");
    printf("대출 ID: %d\n", loan->id);
    
    // 도서 정보 출력
    if (get_book_by_id(db, loan->book_id, &book) == SUCCESS) {
        printf("도서: %s (ID: %d)\n", book.title, book.id);
        printf("저자: %s\n", book.author);
    } else {
        printf("도서 ID: %d (정보 없음)\n", loan->book_id);
    }
    
    // 회원 정보 출력
    if (get_member_by_id(db, loan->member_id, &member) == SUCCESS) {
        printf("대출자: %s (ID: %d)\n", member.name, member.id);
        printf("이메일: %s\n", member.email);
    } else {
        printf("회원 ID: %d (정보 없음)\n", loan->member_id);
    }
    
    printf("대출일: %s", ctime(&loan->loan_date));
    printf("반납예정일: %s", ctime(&loan->due_date));
    
    if (loan->is_returned) {
        printf("반납일: %s", ctime(&loan->return_date));
        printf("상태: 반납완료\n");
    } else {
        printf("상태: 대출중\n");
        
        char overdue_status[64];
        get_overdue_status_string(loan->due_date, 0, overdue_status, sizeof(overdue_status));
        printf("연체상태: %s\n", overdue_status);
    }
    
    printf("연장횟수: %d회\n", loan->renewal_count);
    printf("==========================================\n");
}

void print_loan_list(sqlite3 *db, const LoanSearchResult *result) {
    if (!result || !result->loans) {
        printf("검색 결과가 없습니다.\n");
        return;
    }
    
    printf("\n총 %d건의 대출 기록이 검색되었습니다.\n\n", result->count);
    
    for (int i = 0; i < result->count; i++) {
        printf("%d. ", i + 1);
        print_loan(db, &result->loans[i]);
        printf("\n");
    }
}

void print_loan_statistics(int total_loans, int current_loans, int overdue_loans, int returned_loans) {
    printf("==========================================\n");
    printf("대출 통계\n");
    printf("총 대출 건수: %d건\n", total_loans);
    printf("현재 대출 중: %d건\n", current_loans);
    printf("연체 중: %d건\n", overdue_loans);
    printf("반납 완료: %d건\n", returned_loans);
    printf("==========================================\n");
}

void get_overdue_status_string(time_t due_date, time_t return_date, char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return;
    }
    
    int overdue_days = calculate_overdue_days(due_date, return_date);
    
    if (overdue_days <= 0) {
        snprintf(buffer, buffer_size, "정상");
    } else {
        snprintf(buffer, buffer_size, "%d일 연체", overdue_days);
    }
}

// 콜백 함수들
static int loan_callback(void *data, int argc, char **argv, char **azColName) {
    LoanSearchResult *result = (LoanSearchResult*)data;
    
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
        
        Loan *new_loans = realloc(result->loans, sizeof(Loan) * new_capacity);
        if (!new_loans) {
            return SQLITE_ABORT;
        }
        
        result->loans = new_loans;
        result->capacity = new_capacity;
    }
    
    Loan *loan = &result->loans[result->count];
    init_loan(loan);
    
    // 데이터 복사
    if (argv[0]) loan->id = atoi(argv[0]);
    if (argv[1]) loan->book_id = atoi(argv[1]);
    if (argv[2]) loan->member_id = atoi(argv[2]);
    if (argv[3]) loan->loan_date = (time_t)atoll(argv[3]);
    if (argv[4]) loan->due_date = (time_t)atoll(argv[4]);
    if (argv[5]) loan->return_date = (time_t)atoll(argv[5]);
    if (argv[6]) loan->is_returned = atoi(argv[6]);
    if (argv[7]) loan->renewal_count = atoi(argv[7]);
    if (argv[8]) loan->created_at = (time_t)atoll(argv[8]);
    if (argv[9]) loan->updated_at = (time_t)atoll(argv[9]);
    
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

static int popular_books_callback(void *data, int argc, char **argv, char **azColName) {
    PopularBooksData *popular_data = (PopularBooksData*)data;
    
    if (!popular_data || popular_data->current_count >= popular_data->max_books) {
        return SQLITE_ABORT;
    }
    
    if (argc >= 2 && argv[0] && argv[1]) {
        popular_data->book_ids[popular_data->current_count] = atoi(argv[0]);
        popular_data->loan_counts[popular_data->current_count] = atoi(argv[1]);
        popular_data->current_count++;
    }
    
    return SQLITE_OK;
}
