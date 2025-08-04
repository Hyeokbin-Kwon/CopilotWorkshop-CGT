#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "../include/database.h"
#include "../include/constants.h"

sqlite3* database_init(const char *db_path) {
    sqlite3 *db = NULL;
    int result = sqlite3_open(db_path, &db);
    
    if (result != SQLITE_OK) {
        fprintf(stderr, "데이터베이스 열기 실패: %s\n", sqlite3_errmsg(db));
        if (db) {
            sqlite3_close(db);
        }
        return NULL;
    }
    
    // 외래키 제약 조건 활성화
    database_execute_query(db, "PRAGMA foreign_keys = ON;");
    
    // 테이블 생성
    if (database_create_tables(db) != SUCCESS) {
        fprintf(stderr, "테이블 생성 실패\n");
        sqlite3_close(db);
        return NULL;
    }
    
    return db;
}

void database_close(sqlite3 *db) {
    if (db) {
        sqlite3_close(db);
    }
}

int database_create_tables(sqlite3 *db) {
    if (!db) {
        fprintf(stderr, "유효하지 않은 데이터베이스 연결입니다.\n");
        return FAILURE;
    }
    
    // 도서 테이블 생성
    const char *create_books_table = 
        "CREATE TABLE IF NOT EXISTS books ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT NOT NULL,"
        "author TEXT NOT NULL,"
        "isbn TEXT UNIQUE,"
        "publisher TEXT,"
        "publication_year INTEGER,"
        "total_copies INTEGER DEFAULT 1,"
        "available_copies INTEGER DEFAULT 1,"
        "category TEXT,"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ");";
    
    if (database_execute_query(db, create_books_table) != SUCCESS) {
        return FAILURE;
    }
    
    // 회원 테이블 생성
    const char *create_members_table = 
        "CREATE TABLE IF NOT EXISTS members ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "email TEXT UNIQUE NOT NULL,"
        "phone TEXT,"
        "address TEXT,"
        "registration_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "is_active INTEGER DEFAULT 1,"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ");";
    
    if (database_execute_query(db, create_members_table) != SUCCESS) {
        return FAILURE;
    }
    
    // 대출 테이블 생성
    const char *create_loans_table = 
        "CREATE TABLE IF NOT EXISTS loans ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "book_id INTEGER NOT NULL,"
        "member_id INTEGER NOT NULL,"
        "loan_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "due_date TIMESTAMP NOT NULL,"
        "return_date TIMESTAMP NULL,"
        "is_returned INTEGER DEFAULT 0,"
        "renewal_count INTEGER DEFAULT 0,"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE,"
        "FOREIGN KEY (member_id) REFERENCES members(id) ON DELETE CASCADE"
        ");";
    
    if (database_execute_query(db, create_loans_table) != SUCCESS) {
        return FAILURE;
    }
    
    // 인덱스 생성
    const char *create_indexes[] = {
        "CREATE INDEX IF NOT EXISTS idx_books_title ON books(title);",
        "CREATE INDEX IF NOT EXISTS idx_books_author ON books(author);",
        "CREATE INDEX IF NOT EXISTS idx_books_isbn ON books(isbn);",
        "CREATE INDEX IF NOT EXISTS idx_members_email ON members(email);",
        "CREATE INDEX IF NOT EXISTS idx_loans_book_id ON loans(book_id);",
        "CREATE INDEX IF NOT EXISTS idx_loans_member_id ON loans(member_id);",
        "CREATE INDEX IF NOT EXISTS idx_loans_return_date ON loans(return_date);",
        NULL
    };
    
    for (int i = 0; create_indexes[i] != NULL; i++) {
        if (database_execute_query(db, create_indexes[i]) != SUCCESS) {
            return FAILURE;
        }
    }
    
    return SUCCESS;
}

int database_begin_transaction(sqlite3 *db) {
    if (!db) {
        fprintf(stderr, "유효하지 않은 데이터베이스 연결입니다.\n");
        return FAILURE;
    }
    
    return database_execute_query(db, "BEGIN TRANSACTION;");
}

int database_commit_transaction(sqlite3 *db) {
    if (!db) {
        fprintf(stderr, "유효하지 않은 데이터베이스 연결입니다.\n");
        return FAILURE;
    }
    
    return database_execute_query(db, "COMMIT;");
}

int database_rollback_transaction(sqlite3 *db) {
    if (!db) {
        fprintf(stderr, "유효하지 않은 데이터베이스 연결입니다.\n");
        return FAILURE;
    }
    
    return database_execute_query(db, "ROLLBACK;");
}

int database_execute_query(sqlite3 *db, const char *sql) {
    if (!db || !sql) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    char *error_message = NULL;
    int result = sqlite3_exec(db, sql, NULL, NULL, &error_message);
    
    if (result != SQLITE_OK) {
        fprintf(stderr, "SQL 실행 오류: %s\n", error_message);
        sqlite3_free(error_message);
        return FAILURE;
    }
    
    return SUCCESS;
}

int database_prepare_statement(sqlite3 *db, const char *sql, sqlite3_stmt **stmt) {
    if (!db || !sql || !stmt) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    int result = sqlite3_prepare_v2(db, sql, -1, stmt, NULL);
    
    if (result != SQLITE_OK) {
        fprintf(stderr, "SQL 준비 오류: %s\n", sqlite3_errmsg(db));
        return FAILURE;
    }
    
    return SUCCESS;
}

int database_backup(sqlite3 *db, const char *backup_path) {
    if (!db || !backup_path) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    sqlite3 *backup_db = NULL;
    sqlite3_backup *backup = NULL;
    int result = FAILURE;
    
    // 백업 데이터베이스 열기
    if (sqlite3_open(backup_path, &backup_db) != SQLITE_OK) {
        fprintf(stderr, "백업 데이터베이스 생성 실패: %s\n", sqlite3_errmsg(backup_db));
        goto cleanup;
    }
    
    // 백업 초기화
    backup = sqlite3_backup_init(backup_db, "main", db, "main");
    if (!backup) {
        fprintf(stderr, "백업 초기화 실패: %s\n", sqlite3_errmsg(backup_db));
        goto cleanup;
    }
    
    // 백업 실행
    if (sqlite3_backup_step(backup, -1) == SQLITE_DONE) {
        result = SUCCESS;
    } else {
        fprintf(stderr, "백업 실행 실패: %s\n", sqlite3_errmsg(backup_db));
    }
    
cleanup:
    if (backup) {
        sqlite3_backup_finish(backup);
    }
    if (backup_db) {
        sqlite3_close(backup_db);
    }
    
    return result;
}

int database_restore(sqlite3 *db, const char *backup_path) {
    if (!db || !backup_path) {
        fprintf(stderr, "유효하지 않은 매개변수입니다.\n");
        return FAILURE;
    }
    
    sqlite3 *backup_db = NULL;
    sqlite3_backup *backup = NULL;
    int result = FAILURE;
    
    // 백업 데이터베이스 열기
    if (sqlite3_open(backup_path, &backup_db) != SQLITE_OK) {
        fprintf(stderr, "백업 데이터베이스 열기 실패: %s\n", sqlite3_errmsg(backup_db));
        goto cleanup;
    }
    
    // 복원 초기화
    backup = sqlite3_backup_init(db, "main", backup_db, "main");
    if (!backup) {
        fprintf(stderr, "복원 초기화 실패: %s\n", sqlite3_errmsg(db));
        goto cleanup;
    }
    
    // 복원 실행
    if (sqlite3_backup_step(backup, -1) == SQLITE_DONE) {
        result = SUCCESS;
    } else {
        fprintf(stderr, "복원 실행 실패: %s\n", sqlite3_errmsg(db));
    }
    
cleanup:
    if (backup) {
        sqlite3_backup_finish(backup);
    }
    if (backup_db) {
        sqlite3_close(backup_db);
    }
    
    return result;
}

int database_get_last_insert_id(sqlite3 *db) {
    if (!db) {
        fprintf(stderr, "유효하지 않은 데이터베이스 연결입니다.\n");
        return FAILURE;
    }
    
    return (int)sqlite3_last_insert_rowid(db);
}
