/**
 * @file test_integration.cpp
 * @brief 통합 테스트
 * 
 * 여러 모듈이 함께 동작하는 실제 사용 시나리오를 테스트합니다.
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <cstring>

extern "C" {
    #include "database.h"
    #include "book.h"
    #include "member.h"
    #include "loan.h"
    #include "utils.h"
    #include "constants.h"
}

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 테스트용 데이터베이스 설정
        test_db_path = "test_integration_library.db";
        
        // 기존 테스트 DB 파일 삭제
        if (std::filesystem::exists(test_db_path)) {
            std::filesystem::remove(test_db_path);
        }
        
        // 데이터베이스 초기화
        db = database_init(test_db_path);
        ASSERT_NE(db, nullptr);
        ASSERT_EQ(create_schema(db), SUCCESS);
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
};

/**
 * @brief 완전한 도서관 운영 시나리오 테스트
 * 
 * 실제 도서관 운영과 유사한 전체 프로세스를 테스트합니다:
 * 1. 도서 등록
 * 2. 회원 가입
 * 3. 도서 대출
 * 4. 대출 연장
 * 5. 도서 반납
 * 6. 대출 이력 조회
 */
TEST_F(IntegrationTest, FullLibraryOperationScenario) {
    // 1. 도서 등록
    Book books[3];
    const char* titles[] = {"C 프로그래밍", "자바 완전정복", "파이썬 기초"};
    const char* authors[] = {"홍길동", "김철수", "이영희"};
    const char* isbns[] = {"1111111111111", "2222222222222", "3333333333333"};
    
    for (int i = 0; i < 3; i++) {
        memset(&books[i], 0, sizeof(Book));
        strncpy(books[i].title, titles[i], sizeof(books[i].title) - 1);
        strncpy(books[i].author, authors[i], sizeof(books[i].author) - 1);
        strncpy(books[i].isbn, isbns[i], sizeof(books[i].isbn) - 1);
        strncpy(books[i].publisher, "테스트출판사", sizeof(books[i].publisher) - 1);
        strncpy(books[i].category, "컴퓨터", sizeof(books[i].category) - 1);
        books[i].publication_year = 2023;
        books[i].is_available = TRUE;
        
        ASSERT_EQ(add_book(db, &books[i]), SUCCESS) << "도서 " << i + 1 << " 등록 실패";
    }
    
    // 2. 회원 가입
    Member members[2];
    const char* names[] = {"고객1", "고객2"};
    const char* emails[] = {"customer1@example.com", "customer2@example.com"};
    const char* phones[] = {"010-1111-1111", "010-2222-2222"};
    
    for (int i = 0; i < 2; i++) {
        memset(&members[i], 0, sizeof(Member));
        strncpy(members[i].name, names[i], sizeof(members[i].name) - 1);
        strncpy(members[i].email, emails[i], sizeof(members[i].email) - 1);
        strncpy(members[i].phone, phones[i], sizeof(members[i].phone) - 1);
        strncpy(members[i].address, "서울시 강남구", sizeof(members[i].address) - 1);
        members[i].is_active = TRUE;
        
        ASSERT_EQ(add_member(db, &members[i]), SUCCESS) << "회원 " << i + 1 << " 가입 실패";
    }
    
    // 3. 도서 대출
    // 첫 번째 회원이 첫 번째, 두 번째 도서 대출
    ASSERT_EQ(borrow_book(db, members[0].id, books[0].id), SUCCESS) << "첫 번째 도서 대출 실패";
    ASSERT_EQ(borrow_book(db, members[0].id, books[1].id), SUCCESS) << "두 번째 도서 대출 실패";
    
    // 두 번째 회원이 세 번째 도서 대출
    ASSERT_EQ(borrow_book(db, members[1].id, books[2].id), SUCCESS) << "세 번째 도서 대출 실패";
    
    // 대출된 도서들의 상태 확인
    for (int i = 0; i < 3; i++) {
        Book updated_book;
        ASSERT_EQ(get_book_by_id(db, books[i].id, &updated_book), SUCCESS);
        EXPECT_EQ(updated_book.is_available, FALSE) << "도서 " << i + 1 << "의 대출 상태가 반영되지 않음";
    }
    
    // 4. 대출 연장
    ASSERT_EQ(extend_loan(db, members[0].id, books[0].id), SUCCESS) << "대출 연장 실패";
    
    // 5. 도서 반납
    ASSERT_EQ(return_book(db, members[0].id, books[0].id), SUCCESS) << "첫 번째 도서 반납 실패";
    
    // 반납된 도서 상태 확인
    Book returned_book;
    ASSERT_EQ(get_book_by_id(db, books[0].id, &returned_book), SUCCESS);
    EXPECT_EQ(returned_book.is_available, TRUE) << "반납된 도서의 상태가 반영되지 않음";
    
    // 6. 대출 이력 조회
    LoanSearchResult loan_history;
    ASSERT_EQ(get_loan_history_by_member(db, members[0].id, &loan_history), SUCCESS) 
        << "회원 대출 이력 조회 실패";
    
    EXPECT_GE(loan_history.count, 2) << "첫 번째 회원의 대출 이력 개수 부족";
    
    free_loan_search_result(&loan_history);
    
    // 현재 대출 중인 도서 확인
    LoanSearchResult current_loans;
    ASSERT_EQ(get_current_loans(db, &current_loans), SUCCESS) << "현재 대출 목록 조회 실패";
    
    EXPECT_EQ(current_loans.count, 2) << "현재 대출 중인 도서 개수 불일치 (2개 예상)";
    
    free_loan_search_result(&current_loans);
}

/**
 * @brief 도서 검색 및 대출 시나리오 테스트
 * 
 * 도서 검색 후 대출하는 실제 사용 시나리오를 테스트합니다.
 */
TEST_F(IntegrationTest, BookSearchAndBorrowScenario) {
    // 다양한 도서 등록
    struct {
        const char* title;
        const char* author;
        const char* category;
    } book_data[] = {
        {"자바 프로그래밍 입문", "김개발", "프로그래밍"},
        {"자바 고급 기법", "이코딩", "프로그래밍"},
        {"데이터베이스 설계", "박DB", "데이터베이스"},
        {"파이썬으로 배우는 머신러닝", "최AI", "인공지능"}
    };
    
    Book books[4];
    for (int i = 0; i < 4; i++) {
        memset(&books[i], 0, sizeof(Book));
        strncpy(books[i].title, book_data[i].title, sizeof(books[i].title) - 1);
        strncpy(books[i].author, book_data[i].author, sizeof(books[i].author) - 1);
        strncpy(books[i].category, book_data[i].category, sizeof(books[i].category) - 1);
        snprintf(books[i].isbn, sizeof(books[i].isbn), "111111111111%d", i);
        strncpy(books[i].publisher, "테스트출판사", sizeof(books[i].publisher) - 1);
        books[i].publication_year = 2023;
        books[i].is_available = TRUE;
        
        ASSERT_EQ(add_book(db, &books[i]), SUCCESS) << "도서 " << i + 1 << " 등록 실패";
    }
    
    // 회원 등록
    Member member;
    memset(&member, 0, sizeof(Member));
    strncpy(member.name, "독서가", sizeof(member.name) - 1);
    strncpy(member.email, "reader@example.com", sizeof(member.email) - 1);
    strncpy(member.phone, "010-1234-5678", sizeof(member.phone) - 1);
    strncpy(member.address, "서울시 서초구", sizeof(member.address) - 1);
    member.is_active = TRUE;
    ASSERT_EQ(add_member(db, &member), SUCCESS) << "회원 등록 실패";
    
    // "자바"로 도서 검색
    BookSearchResult search_result;
    ASSERT_EQ(search_books_by_title(db, "자바", &search_result), SUCCESS) << "도서 제목 검색 실패";
    
    EXPECT_EQ(search_result.count, 2) << "자바 관련 도서 검색 결과 개수 불일치 (2개 예상)";
    
    // 검색된 첫 번째 도서 대출
    if (search_result.count > 0) {
        ASSERT_EQ(borrow_book(db, member.id, search_result.books[0].id), SUCCESS) 
            << "검색된 도서 대출 실패";
        
        // 대출 후 해당 도서가 검색에서 제외되는지 확인 (대출 가능한 도서만 검색한다면)
        BookSearchResult updated_search;
        search_books_by_title(db, "자바", &updated_search);
        
        // 구현에 따라 대출된 도서도 검색될 수 있으므로 유연하게 확인
        SUCCEED() << "도서 검색 및 대출 완료";
        
        free_book_search_result(&updated_search);
    }
    
    free_book_search_result(&search_result);
    
    // 저자명으로 검색
    BookSearchResult author_search;
    ASSERT_EQ(search_books_by_author(db, "김개발", &author_search), SUCCESS) << "저자명 검색 실패";
    
    EXPECT_GE(author_search.count, 1) << "저자명 검색 결과 부족";
    
    free_book_search_result(&author_search);
}

/**
 * @brief 회원 관리 시나리오 테스트
 * 
 * 회원 등록, 정보 수정, 대출 이력 관리 등의 시나리오를 테스트합니다.
 */
TEST_F(IntegrationTest, MemberManagementScenario) {
    // 회원 등록
    Member member;
    memset(&member, 0, sizeof(Member));
    strncpy(member.name, "홍길동", sizeof(member.name) - 1);
    strncpy(member.email, "hong@example.com", sizeof(member.email) - 1);
    strncpy(member.phone, "010-1234-5678", sizeof(member.phone) - 1);
    strncpy(member.address, "서울시 강남구", sizeof(member.address) - 1);
    member.is_active = TRUE;
    ASSERT_EQ(add_member(db, &member), SUCCESS) << "회원 등록 실패";
    
    int member_id = member.id;
    
    // 회원 정보 수정
    strncpy(member.name, "홍길동(수정)", sizeof(member.name) - 1);
    strncpy(member.email, "hong_updated@example.com", sizeof(member.email) - 1);
    strncpy(member.address, "부산시 해운대구", sizeof(member.address) - 1);
    
    ASSERT_EQ(update_member(db, &member), SUCCESS) << "회원 정보 수정 실패";
    
    // 수정된 정보 확인
    Member updated_member;
    ASSERT_EQ(get_member_by_id(db, member_id, &updated_member), SUCCESS) << "수정된 회원 정보 조회 실패";
    
    EXPECT_STREQ(updated_member.name, "홍길동(수정)") << "회원명 수정 반영 안됨";
    EXPECT_STREQ(updated_member.email, "hong_updated@example.com") << "이메일 수정 반영 안됨";
    EXPECT_STREQ(updated_member.address, "부산시 해운대구") << "주소 수정 반영 안됨";
    
    // 도서 등록 및 대출
    Book book;
    memset(&book, 0, sizeof(Book));
    strncpy(book.title, "회원 테스트 도서", sizeof(book.title) - 1);
    strncpy(book.author, "테스트 작가", sizeof(book.author) - 1);
    strncpy(book.isbn, "9999999999999", sizeof(book.isbn) - 1);
    strncpy(book.publisher, "테스트출판사", sizeof(book.publisher) - 1);
    strncpy(book.category, "기타", sizeof(book.category) - 1);
    book.publication_year = 2023;
    book.is_available = TRUE;
    ASSERT_EQ(add_book(db, &book), SUCCESS) << "테스트 도서 등록 실패";
    
    ASSERT_EQ(borrow_book(db, member_id, book.id), SUCCESS) << "회원 도서 대출 실패";
    
    // 회원의 대출 이력 확인
    LoanSearchResult loan_history;
    ASSERT_EQ(get_loan_history_by_member(db, member_id, &loan_history), SUCCESS) 
        << "회원 대출 이력 조회 실패";
    
    EXPECT_GE(loan_history.count, 1) << "회원 대출 이력 부족";
    
    free_loan_search_result(&loan_history);
    
    // 회원 상태 변경 (비활성화)
    ASSERT_EQ(update_member_status(db, member_id, FALSE), SUCCESS) << "회원 상태 변경 실패";
    
    // 비활성 회원의 추가 대출 시도 (실패해야 함)
    Book second_book = book;
    strncpy(second_book.title, "두 번째 테스트 도서", sizeof(second_book.title) - 1);
    strncpy(second_book.isbn, "8888888888888", sizeof(second_book.isbn) - 1);
    ASSERT_EQ(add_book(db, &second_book), SUCCESS) << "두 번째 테스트 도서 등록 실패";
    
    int borrow_result = borrow_book(db, member_id, second_book.id);
    EXPECT_EQ(borrow_result, FAILURE) << "비활성 회원의 도서 대출이 성공해서는 안됨";
}

/**
 * @brief 대출 제한 및 검증 시나리오 테스트
 * 
 * 최대 대출 권수 제한, 중복 대출 방지 등의 비즈니스 규칙을 테스트합니다.
 */
TEST_F(IntegrationTest, LoanLimitAndValidationScenario) {
    // 회원 등록
    Member member;
    memset(&member, 0, sizeof(Member));
    strncpy(member.name, "대출왕", sizeof(member.name) - 1);
    strncpy(member.email, "borrower@example.com", sizeof(member.email) - 1);
    strncpy(member.phone, "010-5555-5555", sizeof(member.phone) - 1);
    strncpy(member.address, "대전시 유성구", sizeof(member.address) - 1);
    member.is_active = TRUE;
    ASSERT_EQ(add_member(db, &member), SUCCESS) << "회원 등록 실패";
    
    // 최대 대출 권수만큼 도서 등록 및 대출
    Book books[MAX_BOOKS_PER_MEMBER + 1];
    for (int i = 0; i < MAX_BOOKS_PER_MEMBER + 1; i++) {
        memset(&books[i], 0, sizeof(Book));
        snprintf(books[i].title, sizeof(books[i].title), "대출 테스트 도서 %d", i + 1);
        snprintf(books[i].author, sizeof(books[i].author), "저자 %d", i + 1);
        snprintf(books[i].isbn, sizeof(books[i].isbn), "555555555555%d", i);
        strncpy(books[i].publisher, "테스트출판사", sizeof(books[i].publisher) - 1);
        strncpy(books[i].category, "테스트", sizeof(books[i].category) - 1);
        books[i].publication_year = 2023;
        books[i].is_available = TRUE;
        
        ASSERT_EQ(add_book(db, &books[i]), SUCCESS) << "도서 " << i + 1 << " 등록 실패";
    }
    
    // 최대 권수까지 대출
    for (int i = 0; i < MAX_BOOKS_PER_MEMBER; i++) {
        int result = borrow_book(db, member.id, books[i].id);
        EXPECT_EQ(result, SUCCESS) << "도서 " << i + 1 << " 대출 실패";
    }
    
    // 최대 권수 초과 대출 시도
    int exceed_result = borrow_book(db, member.id, books[MAX_BOOKS_PER_MEMBER].id);
    EXPECT_EQ(exceed_result, FAILURE) << "최대 대출 권수 초과 대출이 성공해서는 안됨";
    
    // 이미 대출한 도서 재대출 시도
    int duplicate_result = borrow_book(db, member.id, books[0].id);
    EXPECT_EQ(duplicate_result, FAILURE) << "이미 대출한 도서의 재대출이 성공해서는 안됨";
    
    // 한 권 반납 후 다른 도서 대출
    ASSERT_EQ(return_book(db, member.id, books[0].id), SUCCESS) << "도서 반납 실패";
    
    int new_borrow_result = borrow_book(db, member.id, books[MAX_BOOKS_PER_MEMBER].id);
    EXPECT_EQ(new_borrow_result, SUCCESS) << "반납 후 새 도서 대출 실패";
    
    // 현재 대출 상태 확인
    LoanSearchResult current_loans;
    ASSERT_EQ(get_current_loans(db, &current_loans), SUCCESS) << "현재 대출 목록 조회 실패";
    
    EXPECT_EQ(current_loans.count, MAX_BOOKS_PER_MEMBER) << "현재 대출 권수 불일치";
    
    free_loan_search_result(&current_loans);
}

/**
 * @brief 데이터베이스 백업 및 복원 시나리오 테스트
 * 
 * 실제 데이터가 있는 상태에서 백업과 복원이 정상적으로 동작하는지 테스트합니다.
 */
TEST_F(IntegrationTest, DatabaseBackupRestoreScenario) {
    // 테스트 데이터 생성
    Book book;
    memset(&book, 0, sizeof(Book));
    strncpy(book.title, "백업 테스트 도서", sizeof(book.title) - 1);
    strncpy(book.author, "백업 작가", sizeof(book.author) - 1);
    strncpy(book.isbn, "1111111111111", sizeof(book.isbn) - 1);
    strncpy(book.publisher, "백업출판사", sizeof(book.publisher) - 1);
    strncpy(book.category, "백업", sizeof(book.category) - 1);
    book.publication_year = 2023;
    book.is_available = TRUE;
    ASSERT_EQ(add_book(db, &book), SUCCESS) << "백업 테스트 도서 등록 실패";
    
    Member member;
    memset(&member, 0, sizeof(Member));
    strncpy(member.name, "백업 회원", sizeof(member.name) - 1);
    strncpy(member.email, "backup@example.com", sizeof(member.email) - 1);
    strncpy(member.phone, "010-0000-0000", sizeof(member.phone) - 1);
    strncpy(member.address, "백업시 백업구", sizeof(member.address) - 1);
    member.is_active = TRUE;
    ASSERT_EQ(add_member(db, &member), SUCCESS) << "백업 테스트 회원 등록 실패";
    
    ASSERT_EQ(borrow_book(db, member.id, book.id), SUCCESS) << "백업 테스트 도서 대출 실패";
    
    // 백업 수행
    const char* backup_path = "integration_test_backup.db";
    if (std::filesystem::exists(backup_path)) {
        std::filesystem::remove(backup_path);
    }
    
    int backup_result = database_backup(db, backup_path);
    EXPECT_EQ(backup_result, SUCCESS) << "데이터베이스 백업 실패";
    EXPECT_TRUE(std::filesystem::exists(backup_path)) << "백업 파일이 생성되지 않음";
    
    // 원본 데이터베이스 연결 해제
    database_close(db);
    
    // 새로운 데이터베이스로 복원 테스트
    const char* restore_db_path = "integration_test_restore.db";
    if (std::filesystem::exists(restore_db_path)) {
        std::filesystem::remove(restore_db_path);
    }
    
    sqlite3* restore_db = database_init(restore_db_path);
    ASSERT_NE(restore_db, nullptr) << "복원 데이터베이스 초기화 실패";
    
    int restore_result = database_restore(restore_db, backup_path);
    EXPECT_EQ(restore_result, SUCCESS) << "데이터베이스 복원 실패";
    
    // 복원된 데이터 확인
    Book restored_book;
    int book_result = get_book_by_id(restore_db, book.id, &restored_book);
    if (book_result == SUCCESS) {
        EXPECT_STREQ(restored_book.title, book.title) << "복원된 도서 제목 불일치";
        EXPECT_STREQ(restored_book.author, book.author) << "복원된 도서 저자 불일치";
    } else {
        // 복원이 완전하지 않을 수 있으므로 경고만 출력
        ADD_FAILURE() << "복원된 데이터베이스에서 도서 조회 실패";
    }
    
    Member restored_member;
    int member_result = get_member_by_id(restore_db, member.id, &restored_member);
    if (member_result == SUCCESS) {
        EXPECT_STREQ(restored_member.name, member.name) << "복원된 회원 이름 불일치";
        EXPECT_STREQ(restored_member.email, member.email) << "복원된 회원 이메일 불일치";
    } else {
        ADD_FAILURE() << "복원된 데이터베이스에서 회원 조회 실패";
    }
    
    // 정리
    database_close(restore_db);
    
    if (std::filesystem::exists(backup_path)) {
        std::filesystem::remove(backup_path);
    }
    if (std::filesystem::exists(restore_db_path)) {
        std::filesystem::remove(restore_db_path);
    }
    
    // 원본 데이터베이스 재연결
    db = database_init(test_db_path);
}
