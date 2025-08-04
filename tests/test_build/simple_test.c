#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "database.h"
#include "book.h"
#include "member.h"
#include "loan.h"
#include "utils.h"

int test_database_operations() {
    printf("?�스??1: ?�이?�베?�스 초기??�??�키�??�성\n");
    
    sqlite3 *db = database_init("test_simple.db");
    if (!db) {
        printf("  FAIL: ?�이?�베?�스 초기???�패\n");
        return 0;
    }
    
    if (create_schema(db) != SUCCESS) {
        printf("  FAIL: ?�키�??�성 ?�패\n");
        database_close(db);
        return 0;
    }
    
    printf("  PASS: ?�이?�베?�스 초기??�??�키�??�성 ?�공\n");
    database_close(db);
    return 1;
}

int test_book_operations() {
    printf("?�스??2: ?�서 관�?기능\n");
    
    sqlite3 *db = database_init("test_simple.db");
    if (!db) return 0;
    
    Book book;
    memset(
    strcpy(book.title, "?�스???�서");
    strcpy(book.author, "?�스???�??^);
    strcpy(book.isbn, "1234567890123");
    strcpy(book.publisher, "?�스?�출?�사");
    strcpy(book.category, "?�스??^);
    book.publication_year = 2023;
    book.is_available = TRUE;
    
    if (add_book(db, 
        printf("  FAIL: ?�서 추�? ?�패\n");
        database_close(db);
        return 0;
    }
    
    Book retrieved_book;
    if (get_book_by_id(db, book.id, 
        printf("  FAIL: ?�서 조회 ?�패\n");
        database_close(db);
        return 0;
    }
    
    if (strcmp(retrieved_book.title, book.title) != 0) {
        printf("  FAIL: 조회???�서 ?�목 불일�?n");
        database_close(db);
        return 0;
    }
    
    printf("  PASS: ?�서 추�? �?조회 ?�공\n");
    database_close(db);
    return 1;
}

int main() {
    printf("?�서관 관�??�스??간단 ?�스???�작\n");
    printf("=====================================\n\n");
    
    int passed = 0;
    int total = 2;
    
    passed += test_database_operations();
    passed += test_book_operations();
    
    printf("\n=====================================\n");
    printf("?�스??결과: %d/%d ?�과\n", passed, total);
    
    if (passed == total) {
        printf("모든 ?�스?��? ?�공?�습?�다! \n");
        return 0;
    } else {
        printf("?��? ?�스?��? ?�패?�습?�다.\n");
        return 1;
    }
}
