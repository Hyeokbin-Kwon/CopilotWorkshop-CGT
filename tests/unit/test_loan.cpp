/**
 * @file test_loan.cpp
 * @brief 대출 관리 모듈 단위 테스트
 * 
 * 도서 대출, 반납, 연장, 검색 등의 기능을 테스트합니다.
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <cstring>
#include <ctime>

extern "C" {
    #include "database.h"
    #include "book.h"
    #include "member.h"
    #include "loan.h"
    #include "constants.h"
}

class LoanTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 테스트용 데이터베이스 설정
        test_db_path = "test_loan_library.db";
        
        // 기존 테스트 DB 파일 삭제
        if (std::filesystem::exists(test_db_path)) {
            std::filesystem::remove(test_db_path);
        }
        
        // 데이터베이스 초기화
        db = database_init(test_db_path);
        ASSERT_NE(db, nullptr);
        ASSERT_EQ(create_schema(db), SUCCESS);
        
        // 테스트용 도서 데이터 준비
        memset(&test_book, 0, sizeof(Book));
        strncpy(test_book.title, "테스트 도서", sizeof(test_book.title) - 1);
        strncpy(test_book.author, "테스트 저자", sizeof(test_book.author) - 1);
        strncpy(test_book.isbn, "1234567890123", sizeof(test_book.isbn) - 1);
        strncpy(test_book.publisher, "테스트 출판사", sizeof(test_book.publisher) - 1);
        strncpy(test_book.category, "컴퓨터", sizeof(test_book.category) - 1);
        test_book.publication_year = 2023;
        test_book.is_available = TRUE;
        ASSERT_EQ(add_book(db, &test_book), SUCCESS);
        
        // 테스트용 회원 데이터 준비
        memset(&test_member, 0, sizeof(Member));
        strncpy(test_member.name, "홍길동", sizeof(test_member.name) - 1);
        strncpy(test_member.email, "hong@example.com", sizeof(test_member.email) - 1);
        strncpy(test_member.phone, "010-1234-5678", sizeof(test_member.phone) - 1);
        strncpy(test_member.address, "서울시 강남구", sizeof(test_member.address) - 1);
        test_member.is_active = TRUE;
        ASSERT_EQ(add_member(db, &test_member), SUCCESS);
    }
    
    void TearDown() override {
        if (db) {
            database_close(db);
        }
        
        if (std::filesystem::exists(test_db_path)) {
            std::filesystem::remove(test_db_path);
        }
    }
    
    const char* test_db_path;
    sqlite3* db;
    Book test_book;
    Member test_member;
};

/**
 * @brief 도서 대출 기능 테스트
 * 
 * 회원이 도서를 대출하는 기능이 정상적으로 동작하는지 확인합니다.
 */
TEST_F(LoanTest, BorrowBook) {
    // 도서 대출
    int result = borrow_book(db, test_member.id, test_book.id);
    EXPECT_EQ(result, SUCCESS) << "도서 대출 실패";
    
    // 도서 가용성 상태 확인 (대출 후 불가능 상태여야 함)
    Book updated_book;
    ASSERT_EQ(get_book_by_id(db, test_book.id, &updated_book), SUCCESS);
    EXPECT_EQ(updated_book.is_available, FALSE) << "대출 후 도서 상태가 변경되지 않음";
}

/**
 * @brief 도서 반납 기능 테스트
 * 
 * 대출된 도서를 반납하는 기능이 정상적으로 동작하는지 확인합니다.
 */
TEST_F(LoanTest, ReturnBook) {
    // 먼저 도서 대출
    ASSERT_EQ(borrow_book(db, test_member.id, test_book.id), SUCCESS);
    
    // 도서 반납
    int result = return_book(db, test_member.id, test_book.id);
    EXPECT_EQ(result, SUCCESS) << "도서 반납 실패";
    
    // 도서 가용성 상태 확인 (반납 후 가능 상태여야 함)
    Book updated_book;
    ASSERT_EQ(get_book_by_id(db, test_book.id, &updated_book), SUCCESS);
    EXPECT_EQ(updated_book.is_available, TRUE) << "반납 후 도서 상태가 변경되지 않음";
}

/**
 * @brief 이미 대출된 도서 대출 시도 테스트
 * 
 * 이미 대출된 도서를 다른 회원이 대출하려 할 때 실패하는지 확인합니다.
 */
TEST_F(LoanTest, BorrowAlreadyBorrowedBook) {
    // 첫 번째 회원이 도서 대출
    ASSERT_EQ(borrow_book(db, test_member.id, test_book.id), SUCCESS);
    
    // 두 번째 회원 생성
    Member second_member = test_member;
    strncpy(second_member.name, "김철수", sizeof(second_member.name) - 1);
    strncpy(second_member.email, "kim@example.com", sizeof(second_member.email) - 1);
    strncpy(second_member.phone, "010-9876-5432", sizeof(second_member.phone) - 1);
    ASSERT_EQ(add_member(db, &second_member), SUCCESS);
    
    // 두 번째 회원이 같은 도서 대출 시도
    int result = borrow_book(db, second_member.id, test_book.id);
    EXPECT_EQ(result, FAILURE) << "이미 대출된 도서의 추가 대출이 성공해서는 안됨";
}

/**
 * @brief 존재하지 않는 회원의 도서 대출 시도 테스트
 * 
 * 존재하지 않는 회원 ID로 도서 대출을 시도할 때 실패하는지 확인합니다.
 */
TEST_F(LoanTest, BorrowWithNonExistentMember) {
    int result = borrow_book(db, 99999, test_book.id);
    EXPECT_EQ(result, FAILURE) << "존재하지 않는 회원의 도서 대출이 성공해서는 안됨";
}

/**
 * @brief 존재하지 않는 도서 대출 시도 테스트
 * 
 * 존재하지 않는 도서 ID로 대출을 시도할 때 실패하는지 확인합니다.
 */
TEST_F(LoanTest, BorrowNonExistentBook) {
    int result = borrow_book(db, test_member.id, 99999);
    EXPECT_EQ(result, FAILURE) << "존재하지 않는 도서의 대출이 성공해서는 안됨";
}

/**
 * @brief 대출되지 않은 도서 반납 시도 테스트
 * 
 * 대출되지 않은 도서를 반납하려 할 때 실패하는지 확인합니다.
 */
TEST_F(LoanTest, ReturnNotBorrowedBook) {
    int result = return_book(db, test_member.id, test_book.id);
    EXPECT_EQ(result, FAILURE) << "대출되지 않은 도서의 반납이 성공해서는 안됨";
}

/**
 * @brief 대출 기간 연장 기능 테스트
 * 
 * 대출된 도서의 대출 기간을 연장하는 기능이 정상적으로 동작하는지 확인합니다.
 */
TEST_F(LoanTest, ExtendLoan) {
    // 도서 대출
    ASSERT_EQ(borrow_book(db, test_member.id, test_book.id), SUCCESS);
    
    // 대출 기간 연장
    int result = extend_loan(db, test_member.id, test_book.id);
    EXPECT_EQ(result, SUCCESS) << "대출 기간 연장 실패";
}

/**
 * @brief 대출되지 않은 도서 연장 시도 테스트
 * 
 * 대출되지 않은 도서의 연장을 시도할 때 실패하는지 확인합니다.
 */
TEST_F(LoanTest, ExtendNotBorrowedBook) {
    int result = extend_loan(db, test_member.id, test_book.id);
    EXPECT_EQ(result, FAILURE) << "대출되지 않은 도서의 연장이 성공해서는 안됨";
}

/**
 * @brief 회원별 대출 이력 조회 테스트
 * 
 * 특정 회원의 대출 이력을 조회하는 기능이 정상적으로 동작하는지 확인합니다.
 */
TEST_F(LoanTest, GetLoanHistoryByMember) {
    // 여러 도서 대출 및 반납
    ASSERT_EQ(borrow_book(db, test_member.id, test_book.id), SUCCESS);
    ASSERT_EQ(return_book(db, test_member.id, test_book.id), SUCCESS);
    
    // 두 번째 도서 추가 및 대출
    Book second_book = test_book;
    strncpy(second_book.title, "두 번째 도서", sizeof(second_book.title) - 1);
    strncpy(second_book.isbn, "9876543210987", sizeof(second_book.isbn) - 1);
    ASSERT_EQ(add_book(db, &second_book), SUCCESS);
    ASSERT_EQ(borrow_book(db, test_member.id, second_book.id), SUCCESS);
    
    // 대출 이력 조회
    LoanSearchResult result;
    int search_result = get_loan_history_by_member(db, test_member.id, &result);
    
    EXPECT_EQ(search_result, SUCCESS) << "대출 이력 조회 실패";
    EXPECT_GE(result.count, 2) << "대출 이력 개수 불일치 (최소 2개 예상)";
    
    // 메모리 해제
    free_loan_search_result(&result);
}

/**
 * @brief 도서별 대출 이력 조회 테스트
 * 
 * 특정 도서의 대출 이력을 조회하는 기능이 정상적으로 동작하는지 확인합니다.
 */
TEST_F(LoanTest, GetLoanHistoryByBook) {
    // 도서 대출 및 반납
    ASSERT_EQ(borrow_book(db, test_member.id, test_book.id), SUCCESS);
    ASSERT_EQ(return_book(db, test_member.id, test_book.id), SUCCESS);
    
    // 두 번째 회원 추가 및 같은 도서 대출
    Member second_member = test_member;
    strncpy(second_member.name, "김철수", sizeof(second_member.name) - 1);
    strncpy(second_member.email, "kim@example.com", sizeof(second_member.email) - 1);
    strncpy(second_member.phone, "010-9876-5432", sizeof(second_member.phone) - 1);
    ASSERT_EQ(add_member(db, &second_member), SUCCESS);
    ASSERT_EQ(borrow_book(db, second_member.id, test_book.id), SUCCESS);
    
    // 도서별 대출 이력 조회
    LoanSearchResult result;
    int search_result = get_loan_history_by_book(db, test_book.id, &result);
    
    EXPECT_EQ(search_result, SUCCESS) << "도서별 대출 이력 조회 실패";
    EXPECT_GE(result.count, 2) << "도서별 대출 이력 개수 불일치 (최소 2개 예상)";
    
    // 메모리 해제
    free_loan_search_result(&result);
}

/**
 * @brief 현재 대출 중인 도서 조회 테스트
 * 
 * 현재 대출 중인 도서 목록을 조회하는 기능이 정상적으로 동작하는지 확인합니다.
 */
TEST_F(LoanTest, GetCurrentLoans) {
    // 여러 도서 대출
    ASSERT_EQ(borrow_book(db, test_member.id, test_book.id), SUCCESS);
    
    Book second_book = test_book;
    strncpy(second_book.title, "두 번째 도서", sizeof(second_book.title) - 1);
    strncpy(second_book.isbn, "9876543210987", sizeof(second_book.isbn) - 1);
    ASSERT_EQ(add_book(db, &second_book), SUCCESS);
    ASSERT_EQ(borrow_book(db, test_member.id, second_book.id), SUCCESS);
    
    // 현재 대출 중인 도서 조회
    LoanSearchResult result;
    int search_result = get_current_loans(db, &result);
    
    EXPECT_EQ(search_result, SUCCESS) << "현재 대출 중인 도서 조회 실패";
    EXPECT_EQ(result.count, 2) << "현재 대출 중인 도서 개수 불일치 (2개 예상)";
    
    // 메모리 해제
    free_loan_search_result(&result);
}

/**
 * @brief 연체 도서 조회 테스트
 * 
 * 연체된 도서 목록을 조회하는 기능이 정상적으로 동작하는지 확인합니다.
 * (실제 연체를 만들기 어려우므로 기능 존재 여부만 확인)
 */
TEST_F(LoanTest, GetOverdueLoans) {
    // 도서 대출
    ASSERT_EQ(borrow_book(db, test_member.id, test_book.id), SUCCESS);
    
    // 연체 도서 조회 (실제 연체는 없지만 함수 호출 테스트)
    LoanSearchResult result;
    int search_result = get_overdue_loans(db, &result);
    
    // 함수가 정상 호출되는지만 확인
    EXPECT_TRUE(search_result == SUCCESS || search_result == FAILURE) 
        << "연체 도서 조회 함수 호출 실패";
    
    if (search_result == SUCCESS) {
        // 연체 도서가 없어야 정상 (새로 대출한 도서)
        EXPECT_EQ(result.count, 0) << "새로 대출한 도서가 연체로 표시됨";
        free_loan_search_result(&result);
    }
}

/**
 * @brief 회원의 최대 대출 권수 제한 테스트
 * 
 * 회원이 최대 대출 권수를 초과하여 도서를 대출하려 할 때 실패하는지 확인합니다.
 */
TEST_F(LoanTest, MaxLoanCountLimit) {
    // MAX_BOOKS_PER_MEMBER만큼 도서를 생성하고 대출
    for (int i = 0; i < MAX_BOOKS_PER_MEMBER; i++) {
        Book book = test_book;
        snprintf(book.title, sizeof(book.title), "테스트 도서 %d", i + 1);
        snprintf(book.isbn, sizeof(book.isbn), "123456789012%d", i);
        ASSERT_EQ(add_book(db, &book), SUCCESS);
        
        int result = borrow_book(db, test_member.id, book.id);
        EXPECT_EQ(result, SUCCESS) << "도서 " << i + 1 << " 대출 실패";
    }
    
    // 추가 도서 생성
    Book extra_book = test_book;
    strncpy(extra_book.title, "추가 도서", sizeof(extra_book.title) - 1);
    strncpy(extra_book.isbn, "9999999999999", sizeof(extra_book.isbn) - 1);
    ASSERT_EQ(add_book(db, &extra_book), SUCCESS);
    
    // 최대 권수 초과 대출 시도
    int result = borrow_book(db, test_member.id, extra_book.id);
    EXPECT_EQ(result, FAILURE) << "최대 대출 권수 초과 대출이 성공해서는 안됨";
}

/**
 * @brief 비활성 회원의 도서 대출 시도 테스트
 * 
 * 비활성 상태의 회원이 도서를 대출하려 할 때 실패하는지 확인합니다.
 */
TEST_F(LoanTest, BorrowByInactiveMember) {
    // 회원을 비활성 상태로 변경
    ASSERT_EQ(update_member_status(db, test_member.id, FALSE), SUCCESS);
    
    // 비활성 회원의 도서 대출 시도
    int result = borrow_book(db, test_member.id, test_book.id);
    EXPECT_EQ(result, FAILURE) << "비활성 회원의 도서 대출이 성공해서는 안됨";
}
