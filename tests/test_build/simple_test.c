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
    printf("?ŒìŠ¤??1: ?°ì´?°ë² ?´ìŠ¤ ì´ˆê¸°??ë°??¤í‚¤ë§??ì„±\n");
    
    sqlite3 *db = database_init("test_simple.db");
    if (!db) {
        printf("  FAIL: ?°ì´?°ë² ?´ìŠ¤ ì´ˆê¸°???¤íŒ¨\n");
        return 0;
    }
    
    if (create_schema(db) != SUCCESS) {
        printf("  FAIL: ?¤í‚¤ë§??ì„± ?¤íŒ¨\n");
        database_close(db);
        return 0;
    }
    
    printf("  PASS: ?°ì´?°ë² ?´ìŠ¤ ì´ˆê¸°??ë°??¤í‚¤ë§??ì„± ?±ê³µ\n");
    database_close(db);
    return 1;
}

int test_book_operations() {
    printf("?ŒìŠ¤??2: ?„ì„œ ê´€ë¦?ê¸°ëŠ¥\n");
    
    sqlite3 *db = database_init("test_simple.db");
    if (!db) return 0;
    
    Book book;
    memset(
    strcpy(book.title, "?ŒìŠ¤???„ì„œ");
    strcpy(book.author, "?ŒìŠ¤???€??^);
    strcpy(book.isbn, "1234567890123");
    strcpy(book.publisher, "?ŒìŠ¤?¸ì¶œ?ì‚¬");
    strcpy(book.category, "?ŒìŠ¤??^);
    book.publication_year = 2023;
    book.is_available = TRUE;
    
    if (add_book(db, 
        printf("  FAIL: ?„ì„œ ì¶”ê? ?¤íŒ¨\n");
        database_close(db);
        return 0;
    }
    
    Book retrieved_book;
    if (get_book_by_id(db, book.id, 
        printf("  FAIL: ?„ì„œ ì¡°íšŒ ?¤íŒ¨\n");
        database_close(db);
        return 0;
    }
    
    if (strcmp(retrieved_book.title, book.title) != 0) {
        printf("  FAIL: ì¡°íšŒ???„ì„œ ?œëª© ë¶ˆì¼ì¹?n");
        database_close(db);
        return 0;
    }
    
    printf("  PASS: ?„ì„œ ì¶”ê? ë°?ì¡°íšŒ ?±ê³µ\n");
    database_close(db);
    return 1;
}

int main() {
    printf("?„ì„œê´€ ê´€ë¦??œìŠ¤??ê°„ë‹¨ ?ŒìŠ¤???œì‘\n");
    printf("=====================================\n\n");
    
    int passed = 0;
    int total = 2;
    
    passed += test_database_operations();
    passed += test_book_operations();
    
    printf("\n=====================================\n");
    printf("?ŒìŠ¤??ê²°ê³¼: %d/%d ?µê³¼\n", passed, total);
    
    if (passed == total) {
        printf("ëª¨ë“  ?ŒìŠ¤?¸ê? ?±ê³µ?ˆìŠµ?ˆë‹¤! \n");
        return 0;
    } else {
        printf("?¼ë? ?ŒìŠ¤?¸ê? ?¤íŒ¨?ˆìŠµ?ˆë‹¤.\n");
        return 1;
    }
}
