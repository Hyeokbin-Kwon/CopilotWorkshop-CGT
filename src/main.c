#include "../include/main.h"

// 전역 변수
sqlite3 *g_database = NULL;
SystemConfig g_config;

int main(int argc, char *argv[]) {
    printf("=================================================\n");
    printf("        도서관 관리 시스템\n");
    printf("        Library Management System\n");
    printf("=================================================\n");
    
    // 애플리케이션 초기화
    if (initialize_application() != SUCCESS) {
        print_error_message("애플리케이션 초기화 실패");
        return EXIT_FAILURE;
    }
    
    print_success_message("시스템이 성공적으로 시작되었습니다.");
    
    // 메인 메뉴 루프
    handle_main_menu();
    
    // 정리 작업
    cleanup_application();
    
    printf("\n프로그램을 종료합니다. 안녕히 가세요!\n");
    return EXIT_SUCCESS;
}

int initialize_application(void) {
    // 설정 파일 로드
    init_default_config(&g_config);
    if (load_config("config.ini", &g_config) != SUCCESS) {
        print_warning_message("설정 파일을 찾을 수 없습니다. 기본 설정을 사용합니다.");
    }
    
    // 로깅 초기화
    if (init_logging("library.log") != SUCCESS) {
        print_warning_message("로그 파일 초기화 실패");
    }
    
    log_message(LOG_INFO, "애플리케이션 시작");
    
    // 데이터베이스 초기화
    if (database_init(g_config.database_path, &g_database) != SUCCESS) {
        log_message(LOG_ERROR, "데이터베이스 초기화 실패: %s", g_config.database_path);
        return FAILURE;
    }
    
    log_message(LOG_INFO, "데이터베이스 연결 성공: %s", g_config.database_path);
    
    return SUCCESS;
}

void cleanup_application(void) {
    if (g_database) {
        database_close(g_database);
        g_database = NULL;
        log_message(LOG_INFO, "데이터베이스 연결 종료");
    }
    
    close_logging();
}

void show_main_menu(void) {
    clear_screen();
    print_header("메인 메뉴");
    
    printf("1. 도서 관리\n");
    printf("2. 회원 관리\n");
    printf("3. 대출 관리\n");
    printf("4. 보고서\n");
    printf("5. 시스템 설정\n");
    printf("0. 종료\n");
    
    print_separator();
}

void handle_main_menu(void) {
    int choice;
    
    while (1) {
        show_main_menu();
        
        choice = get_menu_choice(0, 5, "메뉴를 선택하세요");
        
        switch (choice) {
            case MAIN_BOOK_MANAGEMENT:
                handle_book_menu();
                break;
            case MAIN_MEMBER_MANAGEMENT:
                handle_member_menu();
                break;
            case MAIN_LOAN_MANAGEMENT:
                handle_loan_menu();
                break;
            case MAIN_REPORTS:
                handle_report_menu();
                break;
            case MAIN_SYSTEM_SETTINGS:
                handle_system_menu();
                break;
            case MAIN_EXIT:
                if (get_yes_no_input("정말 종료하시겠습니까? (y/n): ")) {
                    return;
                }
                break;
            default:
                print_error_message("잘못된 선택입니다.");
                pause_for_user();
                break;
        }
    }
}

void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pause_for_user(void) {
    printf("\n계속하려면 Enter 키를 누르세요...");
    getchar();
}

void print_header(const char *title) {
    if (!title) return;
    
    printf("\n");
    print_separator();
    printf("  %s\n", title);
    print_separator();
    printf("\n");
}

void print_separator(void) {
    printf("=================================================\n");
}

void show_book_menu(void) {
    clear_screen();
    print_header("도서 관리");
    
    printf("1. 도서 추가\n");
    printf("2. 도서 검색\n");
    printf("3. 도서 수정\n");
    printf("4. 도서 삭제\n");
    printf("5. 전체 도서 목록\n");
    printf("0. 메인 메뉴로 돌아가기\n");
    
    print_separator();
}

void handle_book_menu(void) {
    int choice;
    
    while (1) {
        show_book_menu();
        
        choice = get_menu_choice(0, 5, "메뉴를 선택하세요");
        
        switch (choice) {
            case BOOK_ADD:
                add_book_interactive();
                break;
            case BOOK_SEARCH:
                search_books_interactive();
                break;
            case BOOK_UPDATE:
                update_book_interactive();
                break;
            case BOOK_DELETE:
                delete_book_interactive();
                break;
            case BOOK_LIST_ALL:
                list_all_books();
                break;
            case BOOK_BACK:
                return;
            default:
                print_error_message("잘못된 선택입니다.");
                pause_for_user();
                break;
        }
    }
}

void add_book_interactive(void) {
    clear_screen();
    print_header("도서 추가");
    
    Book book;
    init_book(&book);
    
    char input[512];
    
    // 제목 입력
    if (get_user_input(input, sizeof(input), "제목: ") == SUCCESS && !is_empty_string(input)) {
        safe_string_copy(book.title, input, sizeof(book.title));
    } else {
        print_error_message("제목은 필수 입력 사항입니다.");
        pause_for_user();
        return;
    }
    
    // 저자 입력
    if (get_user_input(input, sizeof(input), "저자: ") == SUCCESS && !is_empty_string(input)) {
        safe_string_copy(book.author, input, sizeof(book.author));
    } else {
        print_error_message("저자는 필수 입력 사항입니다.");
        pause_for_user();
        return;
    }
    
    // ISBN 입력 (선택사항)
    if (get_user_input(input, sizeof(input), "ISBN (선택사항): ") == SUCCESS && !is_empty_string(input)) {
        if (is_valid_isbn(input)) {
            safe_string_copy(book.isbn, input, sizeof(book.isbn));
        } else {
            print_warning_message("유효하지 않은 ISBN 형식입니다. 건너뜁니다.");
        }
    }
    
    // 출판사 입력
    if (get_user_input(input, sizeof(input), "출판사: ") == SUCCESS && !is_empty_string(input)) {
        safe_string_copy(book.publisher, input, sizeof(book.publisher));
    }
    
    // 카테고리 입력
    if (get_user_input(input, sizeof(input), "카테고리: ") == SUCCESS && !is_empty_string(input)) {
        safe_string_copy(book.category, input, sizeof(book.category));
    }
    
    // 총 권수 입력
    int total_copies;
    if (get_integer_input(&total_copies, "총 권수: ", 1, 999) == SUCCESS) {
        book.total_copies = total_copies;
        book.available_copies = total_copies;
    }
    
    // 도서 추가
    int book_id = add_book(g_database, &book);
    if (book_id > 0) {
        print_success_message("도서가 성공적으로 추가되었습니다.");
        printf("도서 ID: %d\n", book_id);
        log_message(LOG_INFO, "도서 추가 성공: ID=%d, 제목=%s", book_id, book.title);
    } else {
        print_error_message("도서 추가에 실패했습니다.");
    }
    
    pause_for_user();
}

void search_books_interactive(void) {
    clear_screen();
    print_header("도서 검색");
    
    printf("1. 제목으로 검색\n");
    printf("2. 저자로 검색\n");
    printf("3. ISBN으로 검색\n");
    printf("4. 카테고리로 검색\n");
    printf("0. 돌아가기\n");
    
    int choice = get_menu_choice(0, 4, "검색 방법을 선택하세요");
    if (choice == 0) return;
    
    char search_term[256];
    if (get_user_input(search_term, sizeof(search_term), "검색어: ") != SUCCESS || is_empty_string(search_term)) {
        print_error_message("검색어를 입력해주세요.");
        pause_for_user();
        return;
    }
    
    BookSearchResult result;
    if (init_book_search_result(&result) != SUCCESS) {
        print_error_message("검색 결과 초기화 실패");
        pause_for_user();
        return;
    }
    
    int search_result = FAILURE;
    
    switch (choice) {
        case 1:
            search_result = search_books_by_title(g_database, search_term, &result);
            break;
        case 2:
            search_result = search_books_by_author(g_database, search_term, &result);
            break;
        case 3:
            search_result = search_books_by_isbn(g_database, search_term, &result);
            break;
        case 4:
            search_result = search_books_by_category(g_database, search_term, &result);
            break;
    }
    
    if (search_result == SUCCESS) {
        print_book_list(&result);
    } else {
        print_error_message("검색 중 오류가 발생했습니다.");
    }
    
    free_book_search_result(&result);
    pause_for_user();
}

void list_all_books(void) {
    clear_screen();
    print_header("전체 도서 목록");
    
    BookSearchResult result;
    if (init_book_search_result(&result) != SUCCESS) {
        print_error_message("목록 초기화 실패");
        pause_for_user();
        return;
    }
    
    if (get_all_books(g_database, &result) == SUCCESS) {
        print_book_list(&result);
    } else {
        print_error_message("도서 목록 조회 실패");
    }
    
    free_book_search_result(&result);
    pause_for_user();
}

void update_book_interactive(void) {
    clear_screen();
    print_header("도서 정보 수정");
    
    int book_id;
    if (get_integer_input(&book_id, "수정할 도서 ID: ", 1, 999999) != SUCCESS) {
        return;
    }
    
    Book book;
    if (get_book_by_id(g_database, book_id, &book) != SUCCESS) {
        print_error_message("해당 ID의 도서를 찾을 수 없습니다.");
        pause_for_user();
        return;
    }
    
    printf("\n현재 도서 정보:\n");
    print_book(&book);
    
    if (!get_yes_no_input("\n이 도서를 수정하시겠습니까? (y/n): ")) {
        return;
    }
    
    char input[512];
    
    // 제목 수정
    printf("\n현재 제목: %s\n", book.title);
    if (get_user_input(input, sizeof(input), "새 제목 (엔터로 건너뛰기): ") == SUCCESS && !is_empty_string(input)) {
        safe_string_copy(book.title, input, sizeof(book.title));
    }
    
    // 저자 수정
    printf("현재 저자: %s\n", book.author);
    if (get_user_input(input, sizeof(input), "새 저자 (엔터로 건너뛰기): ") == SUCCESS && !is_empty_string(input)) {
        safe_string_copy(book.author, input, sizeof(book.author));
    }
    
    // ISBN 수정
    printf("현재 ISBN: %s\n", book.isbn);
    if (get_user_input(input, sizeof(input), "새 ISBN (엔터로 건너뛰기): ") == SUCCESS && !is_empty_string(input)) {
        if (is_valid_isbn(input)) {
            safe_string_copy(book.isbn, input, sizeof(book.isbn));
        } else {
            print_warning_message("유효하지 않은 ISBN 형식입니다. 건너뜁니다.");
        }
    }
    
    if (update_book(g_database, &book) == SUCCESS) {
        print_success_message("도서 정보가 성공적으로 수정되었습니다.");
        log_message(LOG_INFO, "도서 수정 성공: ID=%d", book_id);
    } else {
        print_error_message("도서 정보 수정에 실패했습니다.");
    }
    
    pause_for_user();
}

void delete_book_interactive(void) {
    clear_screen();
    print_header("도서 삭제");
    
    int book_id;
    if (get_integer_input(&book_id, "삭제할 도서 ID: ", 1, 999999) != SUCCESS) {
        return;
    }
    
    Book book;
    if (get_book_by_id(g_database, book_id, &book) != SUCCESS) {
        print_error_message("해당 ID의 도서를 찾을 수 없습니다.");
        pause_for_user();
        return;
    }
    
    printf("\n삭제할 도서 정보:\n");
    print_book(&book);
    
    print_warning_message("주의: 이 작업은 되돌릴 수 없습니다.");
    if (!get_yes_no_input("정말 이 도서를 삭제하시겠습니까? (y/n): ")) {
        return;
    }
    
    if (delete_book(g_database, book_id) == SUCCESS) {
        print_success_message("도서가 성공적으로 삭제되었습니다.");
        log_message(LOG_INFO, "도서 삭제 성공: ID=%d, 제목=%s", book_id, book.title);
    } else {
        print_error_message("도서 삭제에 실패했습니다.");
    }
    
    pause_for_user();
}
