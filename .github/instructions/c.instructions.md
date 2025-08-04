---
applyTo: "**/*.c, **/*.h, **/*.cpp, **/*.hpp, **/*.cxx, **/*.hxx"
---

# C 언어 코딩 스타일 가이드 - 도서관 관리 시스템

## 명명 규칙
- **함수명**: 동사로 시작하는 snake_case (예: `add_book`, `get_member_by_id`, `process_loan_book`)
- **변수명**: snake_case (예: `book_id`, `member_name`, `loan_date`)
- **상수명**: UPPER_SNAKE_CASE (예: `MAX_INPUT_SIZE`, `DEFAULT_LOAN_DAYS`)
- **구조체명**: CamelCase (예: `Book`, `Member`, `Loan`)
- **파일명**: snake_case with extensions (예: `book.h`, `member.c`, `database.c`)

## 코드 구조
- **헤더 파일**: 구조체 정의, 함수 프로토타입, 상수 선언
- **소스 파일**: 함수 구현, 정적 함수 정의
- **모듈화**: 기능별로 독립적인 헤더/소스 파일 페어
  - `book.h/book.c` - 도서 관리
  - `member.h/member.c` - 회원 관리
  - `loan.h/loan.c` - 대출/반납 관리
  - `database.h/database.c` - 데이터베이스 추상화

## 포맷팅
- **들여쓰기**: 공백 4개 사용
- **중괄호**: K&R 스타일 (같은 줄에 시작)
- **함수 간 간격**: 함수 정의 사이에 한 줄 공백
- **문자열**: 큰따옴표 사용

## 주석
- **언어**: 한글로 작성
- **함수 주석**: Doxygen 스타일 사용
```c
/**
 * @brief 새 도서를 데이터베이스에 추가합니다.
 * 
 * @param book 추가할 도서 정보
 * @return int 성공 시 0, 실패 시 -1 반환
 */
int add_book(const Book *book);
```

## include 구문
- **위치**: 파일 맨 위에 작성
- **순서**: 시스템 헤더, 외부 라이브러리, 프로젝트 헤더 순
- **형식**: 한 줄에 하나씩 작성
```c
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "../include/book.h"
```

## 에러 처리
- **반환값**: 성공 시 0, 실패 시 -1 반환
- **에러 메시지**: stderr에 한글로 출력
- **NULL 체크**: 포인터 매개변수는 반드시 NULL 체크
```c
if (book == NULL) {
    fprintf(stderr, "유효하지 않은 도서 정보입니다.\n");
    return -1;
}
```

## 메모리 관리
- **SQLite 리소스**: `sqlite3_finalize()`, `sqlite3_free()` 사용
- **문자열 복사**: `strncpy()` 사용 후 널 종료 보장
```c
strncpy(book->title, title, sizeof(book->title) - 1);
book->title[sizeof(book->title) - 1] = '\0';
```

## 데이터베이스 패턴
- **준비된 구문**: `prepare_statement()` 사용으로 SQL 인젝션 방지
- **트랜잭션**: 중요한 작업은 BEGIN/COMMIT/ROLLBACK 사용
- **매개변수 바인딩**: `sqlite3_bind_*()` 함수 사용

## 상수 정의
- **매크로**: `#define`으로 상수 정의
- **위치**: 파일 상단 또는 헤더 파일
```c
#define MAX_INPUT_SIZE 1024
#define DEFAULT_LOAN_DAYS 14
#define DATABASE_PATH "database/library.db"
```

## 조건부 컴파일
- **함수 충돌 해결**: `#ifdef` 사용
```c
#ifdef FIX_POPULAR_BOOKS_CONFLICT
int get_loan_popular_books(sqlite3 *db, int *book_ids, int *loan_counts, int max_books);
#else
int get_popular_books(sqlite3 *db, int *book_ids, int *loan_counts, int max_books);
#endif
```

## 콜백 함수
- **정적 함수**: 파일 내부에서만 사용하는 콜백은 static으로 선언
```c
static int member_callback(void *data, int argc, char **argv, char **azColName);
```

## 구조체 초기화
- **memset 사용**: 구조체 초기화 시 `memset(&struct, 0, sizeof(struct))` 사용
- **지정 초기화**: 가능한 경우 designated initializer 사용---
applyTo: "**/*.c, **/*.h, **/*.cpp, **/*.hpp, **/*.cxx, **/*.hxx"
---
- 함수, 변수, 속성 => 소문자 + 언더바 조합
- class, exception => 첫 문자는 대문자 + 파스칼 표기법
- 보호해야하는 인스턴스 속성 => 언더바로 시작
- private 인스턴스 속성 => 더블 언더바로 시작
- 함수와 클래스 정의 사이에는 두줄을 띄운다
- 클래스 정의와 메서드 정의 사이에는 한줄을 띄운다

- 공백(whitespace) 4개를 사용
- 모든 import구문은 파일 맨 위에 작성
- 한줄에 하나의 import문을 작성
- 문자열은 큰 따옴표로 작성
- 주석은 한글로 작성

- 파일 이름은 소문자와 밑줄(_)로 작성



