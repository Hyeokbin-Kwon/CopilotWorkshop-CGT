#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "constants.h"
#include "database.h"
#include "book.h"
#include "member.h"
#include "loan.h"
#include "utils.h"

// 메뉴 타입 정의
typedef enum {
    MAIN_MENU = 0,
    BOOK_MENU = 1,
    MEMBER_MENU = 2,
    LOAN_MENU = 3,
    REPORT_MENU = 4,
    SYSTEM_MENU = 5
} MenuType;

// 메인 메뉴 선택지
typedef enum {
    MAIN_EXIT = 0,
    MAIN_BOOK_MANAGEMENT = 1,
    MAIN_MEMBER_MANAGEMENT = 2,
    MAIN_LOAN_MANAGEMENT = 3,
    MAIN_REPORTS = 4,
    MAIN_SYSTEM_SETTINGS = 5
} MainMenuChoice;

// 도서 관리 메뉴 선택지
typedef enum {
    BOOK_BACK = 0,
    BOOK_ADD = 1,
    BOOK_SEARCH = 2,
    BOOK_UPDATE = 3,
    BOOK_DELETE = 4,
    BOOK_LIST_ALL = 5
} BookMenuChoice;

// 회원 관리 메뉴 선택지
typedef enum {
    MEMBER_BACK = 0,
    MEMBER_ADD = 1,
    MEMBER_SEARCH = 2,
    MEMBER_UPDATE = 3,
    MEMBER_DELETE = 4,
    MEMBER_LIST_ALL = 5
} MemberMenuChoice;

// 대출 관리 메뉴 선택지
typedef enum {
    LOAN_BACK = 0,
    LOAN_BORROW = 1,
    LOAN_RETURN = 2,
    LOAN_EXTEND = 3,
    LOAN_HISTORY = 4,
    LOAN_OVERDUE = 5
} LoanMenuChoice;

// 보고서 메뉴 선택지
typedef enum {
    REPORT_BACK = 0,
    REPORT_STATISTICS = 1,
    REPORT_POPULAR_BOOKS = 2,
    REPORT_MEMBER_ACTIVITY = 3,
    REPORT_OVERDUE_LIST = 4
} ReportMenuChoice;

// 시스템 설정 메뉴 선택지
typedef enum {
    SYSTEM_BACK = 0,
    SYSTEM_BACKUP = 1,
    SYSTEM_RESTORE = 2,
    SYSTEM_CONFIG = 3,
    SYSTEM_LOG = 4
} SystemMenuChoice;

// 전역 변수
extern sqlite3 *g_database;
extern SystemConfig g_config;

// 메인 함수들
int main(int argc, char *argv[]);
int initialize_application(void);
void cleanup_application(void);

// 메뉴 출력 함수들
void show_main_menu(void);
void show_book_menu(void);
void show_member_menu(void);
void show_loan_menu(void);
void show_report_menu(void);
void show_system_menu(void);

// 메뉴 처리 함수들
void handle_main_menu(void);
void handle_book_menu(void);
void handle_member_menu(void);
void handle_loan_menu(void);
void handle_report_menu(void);
void handle_system_menu(void);

// 도서 관리 기능 함수들
void add_book_interactive(void);
void search_books_interactive(void);
void update_book_interactive(void);
void delete_book_interactive(void);
void list_all_books_interactive(void);

// 회원 관리 기능 함수들
void add_member_interactive(void);
void search_members_interactive(void);
void update_member_interactive(void);
void delete_member_interactive(void);
void list_all_members_interactive(void);

// 대출 관리 기능 함수들
void borrow_book_interactive(void);
void return_book_interactive(void);
void extend_loan_interactive(void);
void show_loan_history_interactive(void);
void show_overdue_loans(void);

// 보고서 기능 함수들
void show_library_statistics(void);
void show_popular_books_report(void);
void show_member_activity_report(void);
void show_overdue_report(void);

// 시스템 관리 기능 함수들
void backup_database_interactive(void);
void restore_database_interactive(void);
void configure_system_interactive(void);
void show_system_log(void);

// 유틸리티 함수들
void clear_screen(void);
void pause_for_user(void);
void print_header(const char *title);
void print_separator(void);

#endif // MAIN_H
