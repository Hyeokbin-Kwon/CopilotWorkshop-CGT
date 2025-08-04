#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "database.h"
#include "book.h"
#include "member.h"
#include "loan.h"
#include "utils.h"

int test_database_operations() {
    printf("테스트 1: 데이터베이스 초기화\n");
    
    sqlite3 *db = database_init("test_simple.db");
    if (!db) {
        printf("  FAIL: 데이터베이스 초기화 실패\n");
        return 0;
    }
    
    printf("  PASS: 데이터베이스 초기화 성공\n");
    database_close(db);
    return 1;
}

int test_book_operations() {
    printf("테스트 2: 도서 관리 기능\n");
    
    sqlite3 *db = database_init("test_simple.db");
    if (!db) return 0;
    
    Book book;
    memset(&book, 0, sizeof(Book));
    strcpy(book.title, "Test Book");
    strcpy(book.author, "Test Author");
    strcpy(book.isbn, "1234567890123");
    strcpy(book.publisher, "Test Publisher");
    strcpy(book.category, "Test");
    book.publication_year = 2023;
    book.total_copies = 1;
    book.available_copies = 1;
    
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
    strcpy(member.name, "Test Member");
    strcpy(member.email, "test@example.com");
    strcpy(member.phone, "010-1234-5678");
    strcpy(member.address, "Test Address");
    member.is_active = 1;
    
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
    printf("Library Management System Simple Tests\n");
    printf("=====================================\n\n");
    
    int passed = 0;
    int total = 3;
    
    passed += test_database_operations();
    passed += test_book_operations();
    passed += test_member_operations();
    
    printf("\n=====================================\n");
    printf("Test Results: %d/%d passed\n", passed, total);
    
    if (passed == total) {
        printf("All tests passed! ✅\n");
        return 0;
    } else {
        printf("Some tests failed. ❌\n");
        return 1;
    }
}
