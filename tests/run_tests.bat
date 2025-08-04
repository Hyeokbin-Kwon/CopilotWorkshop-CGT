@echo off
echo ==================================================
echo         도서관 관리 시스템 테스트 실행
echo         Library Management System Tests
echo ==================================================

REM 테스트 빌드 디렉토리 생성
if exist test_build rmdir /s /q test_build
mkdir test_build

echo 간단한 테스트 프로그램을 생성합니다...

REM 간단한 테스트 프로그램 생성 (C 코드)
(
echo #include ^<stdio.h^>
echo #include ^<stdlib.h^>
echo #include ^<string.h^>
echo #include ^<assert.h^>
echo #include "database.h"
echo #include "book.h"
echo #include "member.h"
echo #include "loan.h"
echo #include "utils.h"
echo.
echo int test_database_operations^(^) {
echo     printf^("테스트 1: 데이터베이스 초기화 및 스키마 생성\n"^);
echo.    
echo     sqlite3 *db = database_init^("test_simple.db"^);
echo     if ^(!db^) {
echo         printf^("  FAIL: 데이터베이스 초기화 실패\n"^);
echo         return 0;
echo     }
echo.    
echo     if ^(create_schema^(db^) != SUCCESS^) {
echo         printf^("  FAIL: 스키마 생성 실패\n"^);
echo         database_close^(db^);
echo         return 0;
echo     }
echo.    
echo     printf^("  PASS: 데이터베이스 초기화 및 스키마 생성 성공\n"^);
echo     database_close^(db^);
echo     return 1;
echo }
echo.
echo int test_book_operations^(^) {
echo     printf^("테스트 2: 도서 관리 기능\n"^);
echo.    
echo     sqlite3 *db = database_init^("test_simple.db"^);
echo     if ^(!db^) return 0;
echo.    
echo     Book book;
echo     memset^(&book, 0, sizeof^(Book^)^);
echo     strcpy^(book.title, "테스트 도서"^);
echo     strcpy^(book.author, "테스트 저자"^);
echo     strcpy^(book.isbn, "1234567890123"^);
echo     strcpy^(book.publisher, "테스트출판사"^);
echo     strcpy^(book.category, "테스트"^);
echo     book.publication_year = 2023;
echo     book.is_available = TRUE;
echo.    
echo     if ^(add_book^(db, &book^) != SUCCESS^) {
echo         printf^("  FAIL: 도서 추가 실패\n"^);
echo         database_close^(db^);
echo         return 0;
echo     }
echo.    
echo     Book retrieved_book;
echo     if ^(get_book_by_id^(db, book.id, &retrieved_book^) != SUCCESS^) {
echo         printf^("  FAIL: 도서 조회 실패\n"^);
echo         database_close^(db^);
echo         return 0;
echo     }
echo.    
echo     if ^(strcmp^(retrieved_book.title, book.title^) != 0^) {
echo         printf^("  FAIL: 조회된 도서 제목 불일치\n"^);
echo         database_close^(db^);
echo         return 0;
echo     }
echo.    
echo     printf^("  PASS: 도서 추가 및 조회 성공\n"^);
echo     database_close^(db^);
echo     return 1;
echo }
echo.
echo int main^(^) {
echo     printf^("도서관 관리 시스템 간단 테스트 시작\n"^);
echo     printf^("=====================================\n\n"^);
echo.    
echo     int passed = 0;
echo     int total = 2;
echo.    
echo     passed += test_database_operations^(^);
echo     passed += test_book_operations^(^);
echo.    
echo     printf^("\n=====================================\n"^);
echo     printf^("테스트 결과: %%d/%%d 통과\n", passed, total^);
echo.    
echo     if ^(passed == total^) {
echo         printf^("모든 테스트가 성공했습니다! \n"^);
echo         return 0;
echo     } else {
echo         printf^("일부 테스트가 실패했습니다.\n"^);
echo         return 1;
echo     }
echo }
) > test_build\simple_test.c

echo 테스트 프로그램을 컴파일합니다...

REM 테스트 프로그램 컴파일
gcc -o test_build\simple_test.exe test_build\simple_test.c ..\src\database.c ..\src\book.c ..\src\member.c ..\src\loan.c ..\src\utils.c ..\src\external\sqlite\sqlite3.c -I..\include -I..\src\external\sqlite

if %errorlevel% neq 0 (
    echo 컴파일 실패!
    pause
    exit /b 1
)

echo 컴파일 성공!
echo.
echo 테스트를 실행합니다...
test_build\simple_test.exe

echo.
echo ==================================================
echo 테스트 완료
echo ==================================================
pause
