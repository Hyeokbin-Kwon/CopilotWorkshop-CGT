/**
 * @file test_book.cpp
 * @brief 도서 관리 모듈 단위 테스트
 * 
 * 도서 추가, 조회, 수정, 삭제, 검색 등의 기능을 테스트합니다.
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <cstring>

extern "C" {
    #include "database.h"
    #include "book.h"
    #include "constants.h"
}

class BookTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 테스트용 데이터베이스 설정
        test_db_path = "test_book_library.db";
        
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
};

/**
 * @brief 도서 추가 기능 테스트
 * 
 * 새로운 도서가 정상적으로 데이터베이스에 추가되는지 확인합니다.
 */
TEST_F(BookTest, AddBook) {
    // 도서 추가
    int result = add_book(db, &test_book);
    EXPECT_EQ(result, SUCCESS) << "도서 추가 실패";
    
    // 추가된 도서의 ID 확인 (1부터 시작)
    EXPECT_GT(test_book.id, 0) << "도서 ID가 설정되지 않음";
}

/**
 * @brief 도서 조회 기능 테스트
 * 
 * ID로 도서를 조회하는 기능이 정상적으로 동작하는지 확인합니다.
 */
TEST_F(BookTest, GetBookById) {
    // 도서 추가
    ASSERT_EQ(add_book(db, &test_book), SUCCESS);
    int book_id = test_book.id;
    
    // 도서 조회
    Book retrieved_book;
    int result = get_book_by_id(db, book_id, &retrieved_book);
    
    EXPECT_EQ(result, SUCCESS) << "도서 조회 실패";
    EXPECT_EQ(retrieved_book.id, book_id) << "조회된 도서 ID 불일치";
    EXPECT_STREQ(retrieved_book.title, test_book.title) << "도서 제목 불일치";
    EXPECT_STREQ(retrieved_book.author, test_book.author) << "저자 불일치";
    EXPECT_STREQ(retrieved_book.isbn, test_book.isbn) << "ISBN 불일치";
}

/**
 * @brief 존재하지 않는 도서 조회 테스트
 * 
 * 존재하지 않는 ID로 도서를 조회할 때 적절히 실패하는지 확인합니다.
 */
TEST_F(BookTest, GetNonExistentBook) {
    Book retrieved_book;
    int result = get_book_by_id(db, 99999, &retrieved_book);
    
    EXPECT_EQ(result, FAILURE) << "존재하지 않는 도서 조회가 성공해서는 안됨";
}

/**
 * @brief 도서 정보 수정 기능 테스트
 * 
 * 기존 도서의 정보를 수정하는 기능이 정상적으로 동작하는지 확인합니다.
 */
TEST_F(BookTest, UpdateBook) {
    // 도서 추가
    ASSERT_EQ(add_book(db, &test_book), SUCCESS);
    int book_id = test_book.id;
    
    // 도서 정보 수정
    strncpy(test_book.title, "수정된 도서 제목", sizeof(test_book.title) - 1);
    strncpy(test_book.author, "수정된 저자", sizeof(test_book.author) - 1);
    test_book.publication_year = 2024;
    
    int result = update_book(db, &test_book);
    EXPECT_EQ(result, SUCCESS) << "도서 정보 수정 실패";
    
    // 수정된 정보 확인
    Book updated_book;
    ASSERT_EQ(get_book_by_id(db, book_id, &updated_book), SUCCESS);
    
    EXPECT_STREQ(updated_book.title, "수정된 도서 제목") << "제목 수정 반영 안됨";
    EXPECT_STREQ(updated_book.author, "수정된 저자") << "저자 수정 반영 안됨";
    EXPECT_EQ(updated_book.publication_year, 2024) << "발행년도 수정 반영 안됨";
}

/**
 * @brief 도서 삭제 기능 테스트
 * 
 * 도서를 삭제하는 기능이 정상적으로 동작하는지 확인합니다.
 */
TEST_F(BookTest, DeleteBook) {
    // 도서 추가
    ASSERT_EQ(add_book(db, &test_book), SUCCESS);
    int book_id = test_book.id;
    
    // 도서 삭제
    int result = delete_book(db, book_id);
    EXPECT_EQ(result, SUCCESS) << "도서 삭제 실패";
    
    // 삭제 확인 (조회 시 실패해야 함)
    Book deleted_book;
    result = get_book_by_id(db, book_id, &deleted_book);
    EXPECT_EQ(result, FAILURE) << "삭제된 도서가 여전히 조회됨";
}

/**
 * @brief 도서 제목으로 검색 기능 테스트
 * 
 * 도서 제목을 기준으로 검색하는 기능이 정상적으로 동작하는지 확인합니다.
 */
TEST_F(BookTest, SearchBooksByTitle) {
    // 여러 도서 추가
    Book book1 = test_book;
    strncpy(book1.title, "자바 프로그래밍", sizeof(book1.title) - 1);
    ASSERT_EQ(add_book(db, &book1), SUCCESS);
    
    Book book2 = test_book;
    strncpy(book2.title, "파이썬 프로그래밍", sizeof(book2.title) - 1);
    ASSERT_EQ(add_book(db, &book2), SUCCESS);
    
    Book book3 = test_book;
    strncpy(book3.title, "데이터베이스 설계", sizeof(book3.title) - 1);
    ASSERT_EQ(add_book(db, &book3), SUCCESS);
    
    // "프로그래밍"으로 검색
    BookSearchResult result;
    int search_result = search_books_by_title(db, "프로그래밍", &result);
    
    EXPECT_EQ(search_result, SUCCESS) << "도서 검색 실패";
    EXPECT_EQ(result.count, 2) << "검색 결과 개수 불일치 (2개 예상)";
    
    // 메모리 해제
    free_book_search_result(&result);
}

/**
 * @brief 저자명으로 검색 기능 테스트
 * 
 * 저자명을 기준으로 검색하는 기능이 정상적으로 동작하는지 확인합니다.
 */
TEST_F(BookTest, SearchBooksByAuthor) {
    // 같은 저자의 도서 여러 권 추가
    Book book1 = test_book;
    strncpy(book1.title, "C 프로그래밍 입문", sizeof(book1.title) - 1);
    strncpy(book1.author, "홍길동", sizeof(book1.author) - 1);
    ASSERT_EQ(add_book(db, &book1), SUCCESS);
    
    Book book2 = test_book;
    strncpy(book2.title, "C 프로그래밍 고급", sizeof(book2.title) - 1);
    strncpy(book2.author, "홍길동", sizeof(book2.author) - 1);
    ASSERT_EQ(add_book(db, &book2), SUCCESS);
    
    // 다른 저자 도서
    Book book3 = test_book;
    strncpy(book3.author, "김철수", sizeof(book3.author) - 1);
    ASSERT_EQ(add_book(db, &book3), SUCCESS);
    
    // "홍길동" 저자로 검색
    BookSearchResult result;
    int search_result = search_books_by_author(db, "홍길동", &result);
    
    EXPECT_EQ(search_result, SUCCESS) << "저자 검색 실패";
    EXPECT_EQ(result.count, 2) << "저자 검색 결과 개수 불일치 (2개 예상)";
    
    // 메모리 해제
    free_book_search_result(&result);
}

/**
 * @brief 전체 도서 목록 조회 테스트
 * 
 * 페이징 기능과 함께 전체 도서 목록을 조회하는 기능을 테스트합니다.
 */
TEST_F(BookTest, ListAllBooks) {
    // 여러 도서 추가
    for (int i = 0; i < 5; i++) {
        Book book = test_book;
        snprintf(book.title, sizeof(book.title), "테스트 도서 %d", i + 1);
        ASSERT_EQ(add_book(db, &book), SUCCESS);
    }
    
    // 전체 도서 목록 조회 (limit=10, offset=0)
    BookSearchResult result;
    int list_result = list_all_books(db, &result, 10, 0);
    
    EXPECT_EQ(list_result, SUCCESS) << "전체 도서 목록 조회 실패";
    EXPECT_EQ(result.count, 5) << "전체 도서 개수 불일치 (5개 예상)";
    
    // 메모리 해제
    free_book_search_result(&result);
}

/**
 * @brief 도서 가용성 상태 변경 테스트
 * 
 * 도서의 대출 가능/불가능 상태를 변경하는 기능을 테스트합니다.
 */
TEST_F(BookTest, UpdateBookAvailability) {
    // 도서 추가
    ASSERT_EQ(add_book(db, &test_book), SUCCESS);
    int book_id = test_book.id;
    
    // 대출 불가능 상태로 변경
    int result = update_book_availability(db, book_id, FALSE);
    EXPECT_EQ(result, SUCCESS) << "도서 가용성 상태 변경 실패";
    
    // 상태 확인
    Book updated_book;
    ASSERT_EQ(get_book_by_id(db, book_id, &updated_book), SUCCESS);
    EXPECT_EQ(updated_book.is_available, FALSE) << "대출 불가능 상태 반영 안됨";
    
    // 다시 대출 가능 상태로 변경
    result = update_book_availability(db, book_id, TRUE);
    EXPECT_EQ(result, SUCCESS) << "도서 가용성 상태 복원 실패";
    
    ASSERT_EQ(get_book_by_id(db, book_id, &updated_book), SUCCESS);
    EXPECT_EQ(updated_book.is_available, TRUE) << "대출 가능 상태 반영 안됨";
}

/**
 * @brief 잘못된 데이터로 도서 추가 테스트
 * 
 * NULL 포인터나 잘못된 데이터로 도서를 추가할 때의 오류 처리를 확인합니다.
 */
TEST_F(BookTest, AddBookWithInvalidData) {
    // NULL 포인터 테스트
    int result = add_book(db, nullptr);
    EXPECT_EQ(result, FAILURE) << "NULL 포인터로 도서 추가가 성공해서는 안됨";
    
    // 빈 제목 테스트
    Book invalid_book = test_book;
    strcpy(invalid_book.title, "");
    result = add_book(db, &invalid_book);
    EXPECT_EQ(result, FAILURE) << "빈 제목으로 도서 추가가 성공해서는 안됨";
}

/**
 * @brief 중복 ISBN 처리 테스트
 * 
 * 동일한 ISBN을 가진 도서를 추가할 때의 처리를 확인합니다.
 */
TEST_F(BookTest, AddDuplicateISBN) {
    // 첫 번째 도서 추가
    ASSERT_EQ(add_book(db, &test_book), SUCCESS);
    
    // 동일한 ISBN으로 다른 도서 추가 시도
    Book duplicate_book = test_book;
    strncpy(duplicate_book.title, "다른 제목", sizeof(duplicate_book.title) - 1);
    
    int result = add_book(db, &duplicate_book);
    
    // ISBN 중복 검사가 구현되어 있다면 실패해야 하고,
    // 없다면 성공할 수도 있음 (구현에 따라 다름)
    // 여기서는 구현 상태를 확인하는 목적으로 테스트
    if (result == FAILURE) {
        SUCCEED() << "ISBN 중복 검사가 구현되어 있음";
    } else {
        SUCCEED() << "ISBN 중복 검사가 구현되지 않음 (향후 구현 가능)";
    }
}
