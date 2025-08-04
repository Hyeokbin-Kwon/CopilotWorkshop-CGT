#!/bin/bash

# 테스트 빌드 및 실행 스크립트
# Windows PowerShell용

echo "==================================================" 
echo "        도서관 관리 시스템 테스트 실행"
echo "        Library Management System Tests"
echo "=================================================="

# 테스트 빌드 디렉토리 생성
$TEST_BUILD_DIR = "test_build"
if (Test-Path $TEST_BUILD_DIR) {
    Remove-Item -Recurse -Force $TEST_BUILD_DIR
}
New-Item -ItemType Directory -Path $TEST_BUILD_DIR | Out-Null

# 공통 소스 파일들
$SOURCES = @(
    "src/database.c",
    "src/book.c", 
    "src/member.c",
    "src/loan.c",
    "src/utils.c",
    "src/external/sqlite/sqlite3.c"
)

$INCLUDE_DIRS = @(
    "-Iinclude",
    "-Isrc/external/sqlite"
)

# 각 테스트 개별 컴파일 (GoogleTest 없이 간단한 테스트)
Write-Host "C++ 테스트는 GoogleTest가 필요하므로 건너뜁니다..."
Write-Host "대신 C 기반 간단한 기능 테스트를 수행합니다."

# 간단한 기능 테스트를 위한 C 프로그램 생성
@"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "database.h"
#include "book.h"
#include "member.h"
#include "loan.h"
#include "utils.h"

int test_database_operations() {
    printf("테스트 1: 데이터베이스 초기화 및 스키마 생성\n");
    
    sqlite3 *db = database_init("test_simple.db");
    if (!db) {
        printf("  FAIL: 데이터베이스 초기화 실패\n");
        return 0;
    }
    
    if (create_schema(db) != SUCCESS) {
        printf("  FAIL: 스키마 생성 실패\n");
        database_close(db);
        return 0;
    }
    
    printf("  PASS: 데이터베이스 초기화 및 스키마 생성 성공\n");
    database_close(db);
    return 1;
}

int test_book_operations() {
    printf("테스트 2: 도서 관리 기능\n");
    
    sqlite3 *db = database_init("test_simple.db");
    if (!db) return 0;
    
    Book book;
    memset(&book, 0, sizeof(Book));
    strcpy(book.title, "테스트 도서");
    strcpy(book.author, "테스트 저자");
    strcpy(book.isbn, "1234567890123");
    strcpy(book.publisher, "테스트출판사");
    strcpy(book.category, "테스트");
    book.publication_year = 2023;
    book.is_available = TRUE;
    
    if (add_book(db, &book) != SUCCESS) {
        printf("  FAIL: 도서 추가 실패\n");
        database_close(db);
        return 0;
    }
    
    Book retrieved_book;
    if (get_book_by_id(db, book.id, &retrieved_book) != SUCCESS) {
        printf("  FAIL: 도서 조회 실패\n");
        database_close(db);
        return 0;
    }
    
    if (strcmp(retrieved_book.title, book.title) != 0) {
        printf("  FAIL: 조회된 도서 제목 불일치\n");
        database_close(db);
        return 0;
    }
    
    printf("  PASS: 도서 추가 및 조회 성공\n");
    database_close(db);
    return 1;
}

int test_member_operations() {
    printf("테스트 3: 회원 관리 기능\n");
    
    sqlite3 *db = database_init("test_simple.db");
    if (!db) return 0;
    
    Member member;
    memset(&member, 0, sizeof(Member));
    strcpy(member.name, "홍길동");
    strcpy(member.email, "hong@example.com");
    strcpy(member.phone, "010-1234-5678");
    strcpy(member.address, "서울시 강남구");
    member.is_active = TRUE;
    
    if (add_member(db, &member) != SUCCESS) {
        printf("  FAIL: 회원 추가 실패\n");
        database_close(db);
        return 0;
    }
    
    Member retrieved_member;
    if (get_member_by_id(db, member.id, &retrieved_member) != SUCCESS) {
        printf("  FAIL: 회원 조회 실패\n");
        database_close(db);
        return 0;
    }
    
    if (strcmp(retrieved_member.name, member.name) != 0) {
        printf("  FAIL: 조회된 회원 이름 불일치\n");
        database_close(db);
        return 0;
    }
    
    printf("  PASS: 회원 추가 및 조회 성공\n");
    database_close(db);
    return 1;
}

int main() {
    printf("도서관 관리 시스템 간단 테스트 시작\n");
    printf("=====================================\n\n");
    
    int passed = 0;
    int total = 3;
    
    passed += test_database_operations();
    passed += test_book_operations();
    passed += test_member_operations();
    
    printf("\n=====================================\n");
    printf("테스트 결과: %d/%d 통과\n", passed, total);
    
    if (passed == total) {
        printf("모든 테스트가 성공했습니다! ✅\n");
        return 0;
    } else {
        printf("일부 테스트가 실패했습니다. ❌\n");
        return 1;
    }
}
"@ | Out-File -FilePath "test_build/simple_test.c" -Encoding UTF8

Write-Host "간단한 테스트 프로그램을 컴파일합니다..."

# 테스트 프로그램 컴파일
$gcc_command = "gcc -o test_build/simple_test.exe test_build/simple_test.c " + 
               ($SOURCES -join " ") + " " +
               ($INCLUDE_DIRS -join " ")

try {
    Invoke-Expression $gcc_command
    Write-Host "컴파일 성공!"
    
    Write-Host "`n테스트를 실행합니다..."
    & "test_build/simple_test.exe"
    
} catch {
    Write-Host "컴파일 실패: $_"
    exit 1
}

Write-Host "`n=================================================="
Write-Host "테스트 완료"
Write-Host "=================================================="
