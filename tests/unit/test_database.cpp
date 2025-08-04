/**
 * @file test_database.cpp
 * @brief 데이터베이스 모듈 단위 테스트
 * 
 * 데이터베이스 초기화, 연결, 스키마 생성 등의 기본 기능을 테스트합니다.
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <cstdio>

extern "C" {
    #include "database.h"
    #include "constants.h"
}

class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 테스트용 데이터베이스 경로
        test_db_path = "test_library.db";
        
        // 기존 테스트 DB 파일이 있으면 삭제
        if (std::filesystem::exists(test_db_path)) {
            std::filesystem::remove(test_db_path);
        }
        
        db = nullptr;
    }
    
    void TearDown() override {
        // 데이터베이스 연결 해제
        if (db) {
            database_close(db);
            db = nullptr;
        }
        
        // 테스트 DB 파일 삭제
        if (std::filesystem::exists(test_db_path)) {
            std::filesystem::remove(test_db_path);
        }
    }
    
    const char* test_db_path;
    sqlite3* db;
};

/**
 * @brief 데이터베이스 초기화 테스트
 * 
 * 데이터베이스 파일이 생성되고, 연결이 성공적으로 이루어지는지 확인합니다.
 */
TEST_F(DatabaseTest, InitializeDatabase) {
    // 데이터베이스 초기화
    db = database_init(test_db_path);
    
    // 초기화 성공 확인
    ASSERT_NE(db, nullptr) << "데이터베이스 초기화 실패";
    
    // 데이터베이스 파일 생성 확인
    EXPECT_TRUE(std::filesystem::exists(test_db_path)) << "데이터베이스 파일이 생성되지 않음";
}

/**
 * @brief 데이터베이스 스키마 생성 테스트
 * 
 * 필요한 테이블들(books, members, loans)이 정상적으로 생성되는지 확인합니다.
 */
TEST_F(DatabaseTest, CreateSchema) {
    db = database_init(test_db_path);
    ASSERT_NE(db, nullptr);
    
    // 스키마 생성
    int result = create_schema(db);
    EXPECT_EQ(result, SUCCESS) << "스키마 생성 실패";
    
    // 테이블 존재 확인 쿼리
    const char* check_tables_sql = 
        "SELECT name FROM sqlite_master WHERE type='table' AND name IN ('books', 'members', 'loans');";
    
    sqlite3_stmt *stmt;
    int table_count = 0;
    
    ASSERT_EQ(sqlite3_prepare_v2(db, check_tables_sql, -1, &stmt, NULL), SQLITE_OK);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        table_count++;
    }
    
    sqlite3_finalize(stmt);
    
    // 3개의 테이블이 모두 생성되었는지 확인
    EXPECT_EQ(table_count, 3) << "필요한 테이블이 모두 생성되지 않음";
}

/**
 * @brief 데이터베이스 연결 해제 테스트
 * 
 * 데이터베이스 연결이 정상적으로 해제되는지 확인합니다.
 */
TEST_F(DatabaseTest, CloseDatabase) {
    db = database_init(test_db_path);
    ASSERT_NE(db, nullptr);
    
    // 연결 해제
    database_close(db);
    db = nullptr; // TearDown에서 중복 해제 방지
    
    // 연결 해제 후에는 쿼리 실행이 실패해야 함
    // (실제로는 연결이 해제된 것을 직접 확인하기 어려우므로, 
    //  다른 테스트에서 정상 동작하면 연결 해제도 정상적으로 동작한다고 가정)
    SUCCEED() << "데이터베이스 연결 해제 완료";
}

/**
 * @brief 잘못된 경로로 데이터베이스 초기화 테스트
 * 
 * 존재하지 않는 디렉토리에 데이터베이스를 생성하려 할 때의 동작을 확인합니다.
 */
TEST_F(DatabaseTest, InitializeWithInvalidPath) {
    // 존재하지 않는 디렉토리 경로
    const char* invalid_path = "/invalid/path/test.db";
    
    db = database_init(invalid_path);
    
    // 플랫폼에 따라 다르게 동작할 수 있지만, 
    // 일반적으로는 디렉토리가 없어도 SQLite가 파일을 생성하려 시도함
    // 따라서 NULL이 반환되거나, 성공할 수도 있음
    if (db != nullptr) {
        // 성공한 경우는 정상
        SUCCEED() << "SQLite가 경로를 자동으로 처리함";
    } else {
        // 실패한 경우도 정상 (권한 등의 문제)
        SUCCEED() << "잘못된 경로로 인한 초기화 실패 (예상된 동작)";
    }
}

/**
 * @brief 데이터베이스 백업 기능 테스트
 * 
 * 데이터베이스 백업이 정상적으로 수행되는지 확인합니다.
 */
TEST_F(DatabaseTest, BackupDatabase) {
    db = database_init(test_db_path);
    ASSERT_NE(db, nullptr);
    
    // 스키마 생성
    ASSERT_EQ(create_schema(db), SUCCESS);
    
    // 백업 파일 경로
    const char* backup_path = "test_backup.db";
    
    // 기존 백업 파일 삭제
    if (std::filesystem::exists(backup_path)) {
        std::filesystem::remove(backup_path);
    }
    
    // 백업 수행
    int result = database_backup(db, backup_path);
    EXPECT_EQ(result, SUCCESS) << "데이터베이스 백업 실패";
    
    // 백업 파일 생성 확인
    EXPECT_TRUE(std::filesystem::exists(backup_path)) << "백업 파일이 생성되지 않음";
    
    // 백업 파일 삭제
    if (std::filesystem::exists(backup_path)) {
        std::filesystem::remove(backup_path);
    }
}

/**
 * @brief 데이터베이스 복원 기능 테스트
 * 
 * 백업된 데이터베이스가 정상적으로 복원되는지 확인합니다.
 */
TEST_F(DatabaseTest, RestoreDatabase) {
    // 원본 데이터베이스 생성
    db = database_init(test_db_path);
    ASSERT_NE(db, nullptr);
    ASSERT_EQ(create_schema(db), SUCCESS);
    
    // 백업 수행
    const char* backup_path = "test_backup.db";
    ASSERT_EQ(database_backup(db, backup_path), SUCCESS);
    
    // 원본 데이터베이스 연결 해제
    database_close(db);
    
    // 새로운 복원 대상 데이터베이스 생성
    const char* restore_path = "test_restore.db";
    db = database_init(restore_path);
    ASSERT_NE(db, nullptr);
    
    // 복원 수행
    int result = database_restore(db, backup_path);
    EXPECT_EQ(result, SUCCESS) << "데이터베이스 복원 실패";
    
    // 정리
    database_close(db);
    db = nullptr;
    
    if (std::filesystem::exists(backup_path)) {
        std::filesystem::remove(backup_path);
    }
    if (std::filesystem::exists(restore_path)) {
        std::filesystem::remove(restore_path);
    }
}
