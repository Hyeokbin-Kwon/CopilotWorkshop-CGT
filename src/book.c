#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>
#include "../include/book.h"
#include "../include/database.h"
#include "../include/constants.h"

static int book_callback(void *data, int argc, char **argv, char **azColName);
static int count_callback(void *data, int argc, char **argv, char **azColName);

int add_book(sqlite3 *db, const Book *book) {
    if (!db || !book) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    if (validate_book(book) != SUCCESS) {
        fprintf(stderr, "유효하지 않은 도서 정보입니다.\n");
        return FAILURE;
    }
    
    const char *sql = 
        "INSERT INTO books (title, author, isbn, publisher, publication_year, "
        "total_copies, available_copies, category) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    
    sqlite3_stmt *stmt = NULL;
    int result = FAILURE;
    
    if (database_prepare_statement(db, sql, &stmt) != SUCCESS) {
        return FAILURE;
    }
    
    // 매개변수 바인딩
    sqlite3_bind_text(stmt, 1, book->title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, book->author, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, book->isbn, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, book->publisher, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, book->publication_year);
    sqlite3_bind_int(stmt, 6, book->total_copies);
    sqlite3_bind_int(stmt, 7, book->available_copies);
    sqlite3_bind_text(stmt, 8, book->category, -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        result = database_get_last_insert_id(db);
    } else {
        fprintf(stderr, "도서 추가 실패: %s\n", sqlite3_errmsg(db));
    }
    
    sqlite3_finalize(stmt);
    return result;
}

int get_book_by_id(sqlite3 *db, int book_id, Book *book) {
    if (!db || !book || book_id <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    const char *sql = 
        "SELECT id, title, author, isbn, publisher, publication_year, "
        "total_copies, available_copies, category, created_at, updated_at "
        "FROM books WHERE id = ?;";
    
    sqlite3_stmt *stmt = NULL;
    int result = FAILURE;
    
    if (database_prepare_statement(db, sql, &stmt) != SUCCESS) {
        return FAILURE;
    }
    
    sqlite3_bind_int(stmt, 1, book_id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        book->id = sqlite3_column_int(stmt, 0);
        strncpy(book->title, (const char*)sqlite3_column_text(stmt, 1), MAX_TITLE_LENGTH);
        book->title[MAX_TITLE_LENGTH] = '\0';
        strncpy(book->author, (const char*)sqlite3_column_text(stmt, 2), MAX_AUTHOR_LENGTH);
        book->author[MAX_AUTHOR_LENGTH] = '\0';
        strncpy(book->isbn, (const char*)sqlite3_column_text(stmt, 3), MAX_ISBN_LENGTH);
        book->isbn[MAX_ISBN_LENGTH] = '\0';
        strncpy(book->publisher, (const char*)sqlite3_column_text(stmt, 4), MAX_PUBLISHER_LENGTH);
        book->publisher[MAX_PUBLISHER_LENGTH] = '\0';
        book->publication_year = sqlite3_column_int(stmt, 5);
        book->total_copies = sqlite3_column_int(stmt, 6);
        book->available_copies = sqlite3_column_int(stmt, 7);
        strncpy(book->category, (const char*)sqlite3_column_text(stmt, 8), MAX_CATEGORY_LENGTH);
        book->category[MAX_CATEGORY_LENGTH] = '\0';
        book->created_at = (time_t)sqlite3_column_int64(stmt, 9);
        book->updated_at = (time_t)sqlite3_column_int64(stmt, 10);
        
        result = SUCCESS;
    }
    
    sqlite3_finalize(stmt);
    return result;
}

int get_book_by_isbn(sqlite3 *db, const char *isbn, Book *book) {
    if (!db || !isbn || !book) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    const char *sql = 
        "SELECT id, title, author, isbn, publisher, publication_year, "
        "total_copies, available_copies, category, created_at, updated_at "
        "FROM books WHERE isbn = ?;";
    
    sqlite3_stmt *stmt = NULL;
    int result = FAILURE;
    
    if (database_prepare_statement(db, sql, &stmt) != SUCCESS) {
        return FAILURE;
    }
    
    sqlite3_bind_text(stmt, 1, isbn, -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        book->id = sqlite3_column_int(stmt, 0);
        strncpy(book->title, (const char*)sqlite3_column_text(stmt, 1), MAX_TITLE_LENGTH);
        book->title[MAX_TITLE_LENGTH] = '\0';
        strncpy(book->author, (const char*)sqlite3_column_text(stmt, 2), MAX_AUTHOR_LENGTH);
        book->author[MAX_AUTHOR_LENGTH] = '\0';
        strncpy(book->isbn, (const char*)sqlite3_column_text(stmt, 3), MAX_ISBN_LENGTH);
        book->isbn[MAX_ISBN_LENGTH] = '\0';
        strncpy(book->publisher, (const char*)sqlite3_column_text(stmt, 4), MAX_PUBLISHER_LENGTH);
        book->publisher[MAX_PUBLISHER_LENGTH] = '\0';
        book->publication_year = sqlite3_column_int(stmt, 5);
        book->total_copies = sqlite3_column_int(stmt, 6);
        book->available_copies = sqlite3_column_int(stmt, 7);
        strncpy(book->category, (const char*)sqlite3_column_text(stmt, 8), MAX_CATEGORY_LENGTH);
        book->category[MAX_CATEGORY_LENGTH] = '\0';
        book->created_at = (time_t)sqlite3_column_int64(stmt, 9);
        book->updated_at = (time_t)sqlite3_column_int64(stmt, 10);
        
        result = SUCCESS;
    }
    
    sqlite3_finalize(stmt);
    return result;
}

int search_books_by_title(sqlite3 *db, const char *title, BookSearchResult *result) {
    if (!db || !title || !result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), 
        "SELECT id, title, author, isbn, publisher, publication_year, "
        "total_copies, available_copies, category, created_at, updated_at "
        "FROM books WHERE title LIKE '%%%s%%' ORDER BY title;", title);
    
    return sqlite3_exec(db, sql, book_callback, result, NULL) == SQLITE_OK ? SUCCESS : FAILURE;
}

int search_books_by_author(sqlite3 *db, const char *author, BookSearchResult *result) {
    if (!db || !author || !result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), 
        "SELECT id, title, author, isbn, publisher, publication_year, "
        "total_copies, available_copies, category, created_at, updated_at "
        "FROM books WHERE author LIKE '%%%s%%' ORDER BY author, title;", author);
    
    return sqlite3_exec(db, sql, book_callback, result, NULL) == SQLITE_OK ? SUCCESS : FAILURE;
}

int search_books_by_category(sqlite3 *db, const char *category, BookSearchResult *result) {
    if (!db || !category || !result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), 
        "SELECT id, title, author, isbn, publisher, publication_year, "
        "total_copies, available_copies, category, created_at, updated_at "
        "FROM books WHERE category = '%s' ORDER BY title;", category);
    
    return sqlite3_exec(db, sql, book_callback, result, NULL) == SQLITE_OK ? SUCCESS : FAILURE;
}

int update_book(sqlite3 *db, const Book *book) {
    if (!db || !book || book->id <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    if (validate_book(book) != SUCCESS) {
        fprintf(stderr, "유효하지 않은 도서 정보입니다.\n");
        return FAILURE;
    }
    
    const char *sql = 
        "UPDATE books SET title = ?, author = ?, isbn = ?, publisher = ?, "
        "publication_year = ?, total_copies = ?, available_copies = ?, "
        "category = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?;";
    
    sqlite3_stmt *stmt = NULL;
    int result = FAILURE;
    
    if (database_prepare_statement(db, sql, &stmt) != SUCCESS) {
        return FAILURE;
    }
    
    // 매개변수 바인딩
    sqlite3_bind_text(stmt, 1, book->title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, book->author, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, book->isbn, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, book->publisher, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, book->publication_year);
    sqlite3_bind_int(stmt, 6, book->total_copies);
    sqlite3_bind_int(stmt, 7, book->available_copies);
    sqlite3_bind_text(stmt, 8, book->category, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 9, book->id);
    
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        result = SUCCESS;
    } else {
        fprintf(stderr, "도서 수정 실패: %s\n", sqlite3_errmsg(db));
    }
    
    sqlite3_finalize(stmt);
    return result;
}

int delete_book(sqlite3 *db, int book_id) {
    if (!db || book_id <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    // 대출 중인 도서인지 확인
    const char *check_sql = 
        "SELECT COUNT(*) FROM loans WHERE book_id = ? AND is_returned = 0;";
    
    sqlite3_stmt *check_stmt = NULL;
    int loan_count = 0;
    
    if (database_prepare_statement(db, check_sql, &check_stmt) != SUCCESS) {
        return FAILURE;
    }
    
    sqlite3_bind_int(check_stmt, 1, book_id);
    
    if (sqlite3_step(check_stmt) == SQLITE_ROW) {
        loan_count = sqlite3_column_int(check_stmt, 0);
    }
    
    sqlite3_finalize(check_stmt);
    
    if (loan_count > 0) {
        fprintf(stderr, "대출 중인 도서는 삭제할 수 없습니다.\n");
        return FAILURE;
    }
    
    // 도서 삭제
    const char *sql = "DELETE FROM books WHERE id = ?;";
    sqlite3_stmt *stmt = NULL;
    int result = FAILURE;
    
    if (database_prepare_statement(db, sql, &stmt) != SUCCESS) {
        return FAILURE;
    }
    
    sqlite3_bind_int(stmt, 1, book_id);
    
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        result = SUCCESS;
    } else {
        fprintf(stderr, "도서 삭제 실패: %s\n", sqlite3_errmsg(db));
    }
    
    sqlite3_finalize(stmt);
    return result;
}

int list_all_books(sqlite3 *db, BookSearchResult *result, int limit, int offset) {
    if (!db || !result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    char sql[MAX_SQL_LENGTH];
    if (limit > 0) {
        snprintf(sql, sizeof(sql), 
            "SELECT id, title, author, isbn, publisher, publication_year, "
            "total_copies, available_copies, category, created_at, updated_at "
            "FROM books ORDER BY title LIMIT %d OFFSET %d;", limit, offset);
    } else {
        snprintf(sql, sizeof(sql), 
            "SELECT id, title, author, isbn, publisher, publication_year, "
            "total_copies, available_copies, category, created_at, updated_at "
            "FROM books ORDER BY title;");
    }
    
    return sqlite3_exec(db, sql, book_callback, result, NULL) == SQLITE_OK ? SUCCESS : FAILURE;
}

int list_available_books(sqlite3 *db, BookSearchResult *result) {
    if (!db || !result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    const char *sql = 
        "SELECT id, title, author, isbn, publisher, publication_year, "
        "total_copies, available_copies, category, created_at, updated_at "
        "FROM books WHERE available_copies > 0 ORDER BY title;";
    
    return sqlite3_exec(db, sql, book_callback, result, NULL) == SQLITE_OK ? SUCCESS : FAILURE;
}

int get_popular_books(sqlite3 *db, BookSearchResult *result, int limit) {
    if (!db || !result || limit <= 0) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), 
        "SELECT b.id, b.title, b.author, b.isbn, b.publisher, b.publication_year, "
        "b.total_copies, b.available_copies, b.category, b.created_at, b.updated_at "
        "FROM books b "
        "LEFT JOIN loans l ON b.id = l.book_id "
        "GROUP BY b.id "
        "ORDER BY COUNT(l.id) DESC, b.title "
        "LIMIT %d;", limit);
    
    return sqlite3_exec(db, sql, book_callback, result, NULL) == SQLITE_OK ? SUCCESS : FAILURE;
}

int init_book_search_result(BookSearchResult *result) {
    if (!result) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    result->books = malloc(sizeof(Book) * INITIAL_SEARCH_CAPACITY);
    if (!result->books) {
        fprintf(stderr, "메모리 할당 실패\n");
        return FAILURE;
    }
    
    result->count = 0;
    result->capacity = INITIAL_SEARCH_CAPACITY;
    
    return SUCCESS;
}

void free_book_search_result(BookSearchResult *result) {
    if (result && result->books) {
        free(result->books);
        result->books = NULL;
        result->count = 0;
        result->capacity = 0;
    }
}

void init_book(Book *book) {
    if (!book) {
        return;
    }
    
    memset(book, 0, sizeof(Book));
    book->id = 0;
    book->publication_year = 0;
    book->total_copies = 1;
    book->available_copies = 1;
    book->created_at = time(NULL);
    book->updated_at = time(NULL);
}

int validate_book(const Book *book) {
    if (!book) {
        return FAILURE;
    }
    
    if (strlen(book->title) == 0 || strlen(book->title) > MAX_TITLE_LENGTH) {
        return FAILURE;
    }
    
    if (strlen(book->author) == 0 || strlen(book->author) > MAX_AUTHOR_LENGTH) {
        return FAILURE;
    }
    
    if (book->isbn && strlen(book->isbn) > MAX_ISBN_LENGTH) {
        return FAILURE;
    }
    
    if (book->publisher && strlen(book->publisher) > MAX_PUBLISHER_LENGTH) {
        return FAILURE;
    }
    
    if (book->category && strlen(book->category) > MAX_CATEGORY_LENGTH) {
        return FAILURE;
    }
    
    if (book->total_copies < 0 || book->available_copies < 0) {
        return FAILURE;
    }
    
    if (book->available_copies > book->total_copies) {
        return FAILURE;
    }
    
    return SUCCESS;
}

void print_book(const Book *book) {
    if (!book) {
        return;
    }
    
    printf("==========================================\n");
    printf("도서 ID: %d\n", book->id);
    printf("제목: %s\n", book->title);
    printf("저자: %s\n", book->author);
    printf("ISBN: %s\n", book->isbn);
    printf("출판사: %s\n", book->publisher);
    printf("출판년도: %d\n", book->publication_year);
    printf("총 권수: %d\n", book->total_copies);
    printf("대출가능 권수: %d\n", book->available_copies);
    printf("카테고리: %s\n", book->category);
    printf("==========================================\n");
}

void print_book_list(const BookSearchResult *result) {
    if (!result || !result->books) {
        printf("검색 결과가 없습니다.\n");
        return;
    }
    
    printf("\n총 %d권의 도서가 검색되었습니다.\n\n", result->count);
    
    for (int i = 0; i < result->count; i++) {
        printf("%d. ", i + 1);
        print_book(&result->books[i]);
        printf("\n");
    }
}

// 콜백 함수들
static int book_callback(void *data, int argc, char **argv, char **azColName) {
    BookSearchResult *result = (BookSearchResult*)data;
    
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
        
        Book *new_books = realloc(result->books, sizeof(Book) * new_capacity);
        if (!new_books) {
            return SQLITE_ABORT;
        }
        
        result->books = new_books;
        result->capacity = new_capacity;
    }
    
    Book *book = &result->books[result->count];
    init_book(book);
    
    // 데이터 복사
    if (argv[0]) book->id = atoi(argv[0]);
    if (argv[1]) {
        strncpy(book->title, argv[1], MAX_TITLE_LENGTH);
        book->title[MAX_TITLE_LENGTH] = '\0';
    }
    if (argv[2]) {
        strncpy(book->author, argv[2], MAX_AUTHOR_LENGTH);
        book->author[MAX_AUTHOR_LENGTH] = '\0';
    }
    if (argv[3]) {
        strncpy(book->isbn, argv[3], MAX_ISBN_LENGTH);
        book->isbn[MAX_ISBN_LENGTH] = '\0';
    }
    if (argv[4]) {
        strncpy(book->publisher, argv[4], MAX_PUBLISHER_LENGTH);
        book->publisher[MAX_PUBLISHER_LENGTH] = '\0';
    }
    if (argv[5]) book->publication_year = atoi(argv[5]);
    if (argv[6]) book->total_copies = atoi(argv[6]);
    if (argv[7]) book->available_copies = atoi(argv[7]);
    if (argv[8]) {
        strncpy(book->category, argv[8], MAX_CATEGORY_LENGTH);
        book->category[MAX_CATEGORY_LENGTH] = '\0';
    }
    if (argv[9]) book->created_at = (time_t)atoll(argv[9]);
    if (argv[10]) book->updated_at = (time_t)atoll(argv[10]);
    
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
