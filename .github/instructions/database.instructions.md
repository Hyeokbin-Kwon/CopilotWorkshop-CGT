---
applyTo: "**/*.sqlite3, **/*.db, **/*.sqlite"
---

# SQLite3 Amalgamation 데이터베이스 (C 기반 도서 대출 관리 프로그램)

## 1. 데이터베이스 파일 위치
- SQLite3 Amalgamation 파일은 프로젝트의 `src/external/sqlite/` 디렉토리에 위치합니다.

- 데이터베이스 파일은 프로젝트 내의 별도 디렉토리(`database/`)에 저장합니다.
  - 예: `database/library.db`

## 2. 테이블 설계

다음과 같은 테이블을 사용합니다.

- **Books**: 도서 정보 저장
  - `book_id` (INTEGER PRIMARY KEY AUTOINCREMENT)
  - `title` (TEXT NOT NULL)
  - `author` (TEXT)
  - `publisher` (TEXT)
  - `publication_year` (INTEGER)
  - `isbn` (TEXT UNIQUE)
  - `genre` (TEXT)

- **Members**: 회원 정보 저장
  - `member_id` (INTEGER PRIMARY KEY AUTOINCREMENT)
  - `name` (TEXT NOT NULL)
  - `phone` (TEXT)
  - `address` (TEXT)
  - `registration_date` (TEXT)

- **Loans**: 도서 대출 정보 저장
  - `loan_id` (INTEGER PRIMARY KEY AUTOINCREMENT)
  - `book_id` (INTEGER, FOREIGN KEY REFERENCES Books(book_id))
  - `member_id` (INTEGER, FOREIGN KEY REFERENCES Members(member_id))
  - `loan_date` (TEXT)
  - `due_date` (TEXT)

- **Returns**: 도서 반납 정보 저장
  - `return_id` (INTEGER PRIMARY KEY AUTOINCREMENT)
  - `loan_id` (INTEGER, FOREIGN KEY REFERENCES Loans(loan_id))
  - `return_date` (TEXT)

## 3. 데이터 타입 선택

- `TEXT`: 문자열 데이터 저장 (VARCHAR)
- `INTEGER`: 정수 데이터 저장
- 날짜는 `TEXT` 타입으로 저장하며, SQLite의 날짜 관련 함수를 사용하여 처리합니다.

## 4. 정규화

- 데이터 중복을 최소화하고 데이터 일관성을 유지하기 위해 정규화를 수행합니다.
- 외래 키를 사용하여 테이블 간 관계를 명확히 합니다.

## 5. 인덱싱

다음 필드에 인덱스를 생성하여 검색 성능을 향상시킵니다.

```sql
CREATE INDEX idx_books_title ON Books(title);
CREATE INDEX idx_members_name ON Members(name);
CREATE INDEX idx_loans_book_id ON Loans(book_id);
CREATE INDEX idx_loans_member_id ON Loans(member_id);
```

## 6. 외래 키 제약 조건
Loans.book_id는 Books(book_id)를 참조합니다.
Loans.member_id는 Members(member_id)를 참조합니다.
Returns.loan_id는 Loans.loan_id를 참조합니다.

## 7. 트랜잭션 관리
도서 대출 및 반납과 같은 데이터 변경 작업은 트랜잭션을 사용하여 데이터 일관성을 유지합니다.
```
BEGIN TRANSACTION;
-- SQL statements
COMMIT;
```

## 8. SQLite C API 사용법 예시
다음은 SQLite C API를 사용하여 데이터베이스 연결 및 쿼리를 수행하는 예제입니다.
```
#include <stdio.h>
#include <sqlite3.h>

int main() {
    sqlite3 *db;
    int rc = sqlite3_open("database/library.db", &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS Books ("
                      "book_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "title TEXT NOT NULL,"
                      "author TEXT,"
                      "isbn TEXT UNIQUE);";

    char *err_msg = NULL;
    if (sqlite3_exec(db, sql, 0, 0, NULL) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    sqlite3_close(db);
    return 0;
}
```