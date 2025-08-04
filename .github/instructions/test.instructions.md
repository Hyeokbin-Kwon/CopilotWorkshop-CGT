---
applyTo: "**/*_test.cpp, **/test_*.cpp, **/tests/*.cpp"
---

# Google Test 기반 C/C++ 테스트 가이드 - 도서관 관리 시스템

## 테스트 프레임워크
- **주요 프레임워크**: Google Test (C++ 기반)
- **Google Test path**: 프로젝트의 `src/external/googletest` 디렉토리 
- **대상 언어**: C 함수를 C++에서 테스트
- **데이터베이스**: SQLite3 Amalgamation 프로젝트의 `src/external/sqlite/` 디렉토리 
- **빌드 시스템**: CMake

## 테스트 파일 구조

### 파일 명명 규칙
- 테스트 파일: `{module}_test.cpp` (예: `book_test.cpp`, `member_test.cpp`)
- 헤더 include: `extern "C"` 블록 사용
```cpp
extern "C" {
    #include "../include/book.h"
    #include "../include/database.h"
}
```

### include 순서
```cpp
#define GTEST_HAS_QUICK_EXIT 0
#include <gtest/gtest.h>
#include <string.h>

extern "C" {
    #include "../include/book.h"
    #include "../include/database.h"
}
```

## 테스트 클래스 작성

### 기본 구조
```cpp
class BookTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 인메모리 데이터베이스 초기화
        ASSERT_EQ(0, initialize_database(":memory:"));
        ASSERT_EQ(0, create_tables());
        ASSERT_EQ(0, create_indexes());
    }

    void TearDown() override {
        ASSERT_EQ(0, close_database());
    }
    
    // 테스트 헬퍼 함수
    Book createTestBook(const char* title, const char* author, const char* isbn) {
        Book book;
        memset(&book, 0, sizeof(Book));
        
        strncpy(book.title, title, sizeof(book.title) - 1);
        strncpy(book.author, author, sizeof(book.author) - 1);
        strncpy(book.isbn, isbn, sizeof(book.isbn) - 1);
        book.available = 1;
        
        return book;
    }
};
```

### 테스트 클래스 명명 규칙
- 클래스명: `{Module}Test` (예: `BookTest`, `MemberTest`, `LoanTest`)
- 기능별로 독립적인 테스트 클래스 생성

## 테스트 메서드 작성

### 테스트 메서드 명명 규칙
- 매크로: `TEST_F({클래스명}, {테스트명})`
- 테스트명: 테스트하는 기능을 설명하는 이름 (CamelCase)
```cpp
TEST_F(BookTest, AddBook) {
    // 테스트 내용
}

TEST_F(BookTest, GetBookByIsbn) {
    // 테스트 내용
}
```

### 어설션 패턴
```cpp
// 성공/실패 검증
EXPECT_EQ(0, add_book(&book));        // 성공 시 0
EXPECT_EQ(-1, invalid_function());    // 실패 시 -1

// 문자열 비교
EXPECT_STREQ("expected", actual_string);

// 필수 조건 검증 (실패 시 테스트 중단)
ASSERT_EQ(0, initialize_database(":memory:"));
```

## 테스트 데이터 관리

### 헬퍼 함수 패턴
```cpp
protected:
    Book createTestBook(const char* title, const char* author, const char* isbn) {
        Book book;
        memset(&book, 0, sizeof(Book));
        
        strncpy(book.title, title, sizeof(book.title) - 1);
        strncpy(book.author, author, sizeof(book.author) - 1);
        strncpy(book.isbn, isbn, sizeof(book.isbn) - 1);
        strncpy(book.genre, "Test Genre", sizeof(book.genre) - 1);
        strncpy(book.publisher, "Test Publisher", sizeof(book.publisher) - 1);
        book.publication_year = 2023;
        book.available = 1;
        
        return book;
    }
```

### 테스트 데이터 독립성
- 각 테스트는 SetUp/TearDown으로 독립적인 데이터베이스 환경
- 테스트 간 데이터 의존성 금지

## 테스트 시나리오 패턴

### 기본 CRUD 테스트
```cpp
TEST_F(BookTest, AddBook) {
    Book book = createTestBook("Test Book", "Test Author", "1234567890");
    EXPECT_EQ(0, add_book(&book));
    
    // 추가된 도서 확인
    Book retrieved;
    EXPECT_EQ(0, get_book_by_isbn("1234567890", &retrieved));
    EXPECT_STREQ("Test Book", retrieved.title);
}
```

### 에러 케이스 테스트
```cpp
TEST_F(BookTest, AddBookWithNullPointer) {
    EXPECT_EQ(-1, add_book(NULL));
}

TEST_F(BookTest, GetNonExistentBook) {
    Book book;
    EXPECT_EQ(-1, get_book_by_isbn("0000000000", &book));
}
```

### 비즈니스 로직 테스트
```cpp
TEST_F(BookTest, DeleteBookWithLoans) {
    // 도서 추가 및 대출 처리
    Book book = createTestBook("Loan Test", "Author", "1111111111");
    ASSERT_EQ(0, add_book(&book));
    
    // 대출 중인 도서 삭제 시도 (실패해야 함)
    EXPECT_EQ(-1, delete_book(book.id));
}
```

### 주석 및 문서화

### 테스트 주석
```cpp
// 도서 추가 기능 테스트
TEST_F(BookTest, AddBook) {
    // Given: 테스트 도서 생성
    Book book = createTestBook("Test Book", "Test Author", "1234567890");
    
    // When: 도서 추가 실행
    int result = add_book(&book);
    
    // Then: 성공 확인
    EXPECT_EQ(0, result);
}
```

### 테스트 클래스 문서화
- 테스트 클래스 상단에 테스트 범위 및 목적 명시
- 복잡한 테스트 시나리오에 대한 설명 추가

## 테스트 커버리지

### 필수 테스트 항목
- **도서 관리**: 추가, 조회, 수정, 삭제, 검색
- **회원 관리**: 등록, 조회, 수정, 삭제, 검색
- **대출/반납**: 대출 처리, 반납 처리, 연체 계산
- **에러 처리**: NULL 포인터, 잘못된 매개변수, 데이터베이스 오류

### 테스트 시나리오 유형
- **정상 케이스**: 유효한 입력으로 기능 정상 동작
- **경계 케이스**: 최대/최소값, 빈 문자열 등
- **에러 케이스**: 잘못된 입력, 시스템 오류 상황
- **비즈니스 로직**: 업무 규칙 준수 여부

## 특수 고려사항

### MinGW/UCRT64 호환성
- Google Test 컴파일 정의 필요
- 날짜/시간 관련 함수 호환성 확인

### C에서 C++ 테스트 호환성
- `extern "C"` 블록 사용 필수
- C 구조체 초기화 시 `memset()` 사용
- 문자열 처리 시 `strncpy()` 안전성 확인

### 데이터베이스 테스트
- 트랜잭션 테스트 (BEGIN/COMMIT/ROLLBACK)
- 외래 키 제약 조건 테스트
- SQL 인젝션 방지 테스트