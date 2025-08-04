<!-- Copilot 코드 리뷰 instructions -->

# C언어 코드 리뷰 가이드라인 - 도서관 관리 시스템

이 문서는 C언어 기반 도서관 관리 시스템의 코드 리뷰를 수행할 때 따라야 할 지침을 제공합니다.

## 1. 코드 스타일 및 명명 규칙
- C언어 스타일 가이드는 [C Instructions](./c.instructions.md) 를 따릅니다.
- 함수명: snake_case with verbs (`add_book`, `get_member_by_id`)
- 변수명: snake_case (`book_id`, `member_name`)
- 상수명: UPPER_SNAKE_CASE (`MAX_INPUT_SIZE`, `DEFAULT_LOAN_DAYS`)
- 구조체명: CamelCase (`Book`, `Member`, `Loan`)
- 파일명: snake_case with extensions (`book.h`, `member.c`)

## 2. 헤더 파일 및 모듈 구조
- 헤더 파일에 include guard (`#ifndef`, `#define`, `#endif`) 사용 확인
- 함수 프로토타입과 구조체 정의가 적절히 분리되어 있는지 확인
- 모듈간 의존성이 명확하고 순환 참조가 없는지 확인
- include 순서: 시스템 헤더 → 외부 라이브러리 → 프로젝트 헤더

## 3. 메모리 관리 및 리소스 해제
- SQLite 리소스 관리: `sqlite3_finalize()`, `sqlite3_free()` 적절히 사용
- 동적 메모리 할당 시 `malloc`/`free` 짝 확인
- 문자열 복사 시 `strncpy()` 사용 후 널 종료 보장 확인
- 데이터베이스 연결 해제 (`close_database()`) 확인

## 4. 에러 처리 및 입력 검증
- 모든 포인터 매개변수에 대한 NULL 체크 확인
- 함수 반환값: 성공 시 0, 실패 시 -1 일관성 확인
- 에러 메시지는 stderr에 한글로 출력 확인
- SQLite 함수 호출 후 반환값 확인 (`SQLITE_OK`, `SQLITE_DONE`, `SQLITE_ROW`)
```c
if (book == NULL) {
    fprintf(stderr, "유효하지 않은 도서 정보입니다.\n");
    return -1;
}
```

## 5. 데이터베이스 보안 및 성능
- SQL 인젝션 방지: `prepare_statement()` 사용 확인
- 매개변수 바인딩 (`sqlite3_bind_*()`) 사용 확인
- 트랜잭션 사용: BEGIN/COMMIT/ROLLBACK 적절히 사용
- 인덱스 활용 및 쿼리 최적화 확인
- 외래 키 제약 조건 (`PRAGMA foreign_keys = ON`) 활성화 확인

## 6. 기능 검증
- 도서 관리: 추가, 검색, 수정, 삭제 기능 정상 동작 확인
- 회원 관리: 등록, 조회, 수정, 삭제 기능 정상 동작 확인
- 대출/반납: 대출 처리, 반납 처리, 연체 계산 기능 확인
- 비즈니스 로직: 대출 중인 도서 삭제 불가, 미반납 대출 회원 삭제 불가 확인

## 7. 테스트 검증
- Google Test 기반 단위 테스트 존재 확인
- 인메모리 데이터베이스 (`:memory:`) 사용으로 테스트 격리 확인
- SetUp/TearDown 패턴으로 각 테스트 독립성 보장
- 테스트 커버리지가 주요 기능을 포함하는지 확인
- C++에서 C 함수 호출 시 `extern "C"` 패턴 사용 확인

## 8. 컴파일 및 빌드
- 조건부 컴파일 지시자 (`#ifdef FIX_POPULAR_BOOKS_CONFLICT`) 적절히 사용
- CMake 또는 Makefile로 빌드 가능 확인
- 컴파일 경고 없이 빌드 되는지 확인
- 정적 라이브러리 (`library_core`) 링크 확인

## 9. 주석 및 문서화
- Doxygen 스타일 함수 주석 작성 확인
- 주석이 한글로 작성되어 있는지 확인
- 복잡한 로직에 대한 설명 주석 존재 확인
- README.md 및 관련 문서 최신 상태 확인

## 10. 코드 품질
- 매직 넘버 사용 최소화, 상수 정의 사용 확인
- 긴 함수 분할 및 단일 책임 원칙 준수 확인
- 전역 변수 사용 최소화 (예: 데이터베이스 연결은 예외)
- 불필요한 코드, 디버깅 출력문 제거 확인
- 일관된 들여쓰기 (4칸 공백) 및 중괄호 스타일 확인

## 11. 플랫폼 호환성
- MinGW/UCRT64 환경 호환성 확인
- Windows/Linux 크로스 플랫폼 고려사항 확인
- 컴파일 정의 활용한 플랫폼별 분기 처리 확인

## 12. 특수 검증 사항
- 날짜 형식 일관성 ('YYYY-MM-DD') 확인
- 문자열 버퍼 크기 안전성 확인
- 콜백 함수 static 선언 적절성 확인
- 구조체 초기화 시 `memset()` 사용 확인

## 리뷰 체크리스트
- [ ] 코드 스타일 및 명명 규칙 준수
- [ ] NULL 포인터 체크 및 에러 처리
- [ ] SQLite 리소스 관리 적절성
- [ ] SQL 인젝션 방지 (prepared statement)
- [ ] 메모리 누수 없음
- [ ] 테스트 코드 존재 및 통과
- [ ] 컴파일 경고 없음
- [ ] 문서화 적절성
- [ ] 비즈니스 로직 정확성
- [ ] 플랫폼 호환성