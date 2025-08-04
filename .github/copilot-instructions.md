<!-- ---
applyTo: "**"
--- -->

# GitHub Copilot Workshop - C Library Management System

## 프로젝트 개요
이 프로젝트는 C 언어 기반 도서관 관리 시스템 개발을 위한 **워크샵 템플릿**입니다. SQLite3 Amalgamation과 Google Test가 사전 구성되어 있으며, AI 코딩 도구 사용을 위한 상세한 가이드라인을 제공합니다.

## 아키텍처 패턴
- **모듈화 설계**: 각 도메인별 독립적인 헤더/소스 파일 페어
  - `book.h/book.c` - 도서 관리 (CRUD, 검색)
  - `member.h/member.c` - 회원 관리 (등록, 상태 추적)  
  - `loan.h/loan.c` - 대출/반납 로직
  - `database.h/database.c` - SQLite 추상화 레이어
- **정적 라이브러리**: 핵심 기능을 `library_core`로 분리하여 재사용성 확보
- **C/C++ 하이브리드 테스트**: C 함수를 C++ Google Test로 테스트

## 빌드 및 의존성 관리

### 사전 구성된 외부 라이브러리
- **SQLite3**: `src/external/sqlite/` (sqlite3.c, sqlite3.h 포함)
- **Google Test**: `src/external/googletest/` (전체 프레임워크 포함)

### CMake 설정 패턴
```cmake
# SQLite3 정적 라이브러리 생성
add_library(sqlite3 STATIC external/sqlite/sqlite3.c)
target_include_directories(sqlite3 PUBLIC external/sqlite)

# Google Test 링크
find_package(GTest REQUIRED)
target_link_libraries(tests GTest::gtest GTest::gtest_main)
```

## 데이터베이스 설계
- **초기화 시퀀스**: `initialize_database()` → `create_tables()` → `create_indexes()`
- **연결 관리**: 전역 연결 객체 패턴 (`get_db_connection()`)
- **트랜잭션 보장**: 대출/반납 등 비즈니스 로직에서 필수
- **성능 최적화**: 주요 검색 필드에 인덱스 생성 (`idx_books_title`, `idx_members_name`)

## 테스트 전략

### Google Test 설정
```cpp
// C 헤더 포함 패턴
extern "C" {
    #include "../include/book.h"
    #include "../include/database.h"
}

class BookTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 인메모리 DB로 격리된 테스트 환경
        ASSERT_EQ(0, initialize_database(":memory:"));
    }
};
```

## AI 도구 통합

### 커스텀 프롬프트 활용
- `.github/prompts/create_module.prompt.md` - 새 모듈 생성 자동화
- `.github/prompts/code_review.prompt.md` - 코드 리뷰 체크리스트
- `.github/prompts/refactor_code.prompt.md` - 리팩토링 가이드

### 컨텍스트별 지침서
- `.github/instructions/c.instructions.md` - C 코딩 스타일 (K&R, 한글 주석)
- `.github/instructions/database.instructions.md` - SQLite 스키마 및 최적화
- `.github/instructions/test.instructions.md` - Google Test 패턴

### Chat 모드
- **Plan 모드**: 기능 구현 계획 수립 전용
- **Azure 모드**: 클라우드 통합 시나리오

## 핵심 코딩 컨벤션
- **에러 처리**: 모든 함수는 성공 시 0, 실패 시 -1 반환
- **메모리 관리**: 동적 할당 시 해제 책임 명시
- **문서화**: Doxygen 스타일 한글 주석 필수
- **함수 명명**: 동사+명사 조합 (`add_book`, `process_loan_return`)

## 개발 워크플로우
1. **설계**: `.github/chatmodes/plan.chatmode.md` 활용하여 기능 계획 수립
2. **구현**: 커스텀 프롬프트로 모듈 스켈레톤 생성
3. **테스트**: 인메모리 DB 기반 단위 테스트 작성
4. **통합**: CMake 빌드 시스템으로 전체 빌드 검증