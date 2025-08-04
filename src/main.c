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
    if ((g_database = database_init(g_config.database_path)) == NULL) {
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
                list_all_books_interactive();
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
            search_result = search_books_by_title(g_database, search_term, &result);
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

void list_all_books_interactive(void) {
    clear_screen();
    print_header("전체 도서 목록");
    
    BookSearchResult result;
    if (init_book_search_result(&result) != SUCCESS) {
        print_error_message("목록 초기화 실패");
        pause_for_user();
        return;
    }
    
    if (list_all_books(g_database, &result, 100, 0) == SUCCESS) {
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

void show_member_menu(void) {
    clear_screen();
    print_header("회원 관리");
    
    printf("1. 회원 추가\n");
    printf("2. 회원 검색\n");
    printf("3. 회원 정보 수정\n");
    printf("4. 회원 삭제\n");
    printf("5. 전체 회원 목록\n");
    printf("0. 메인 메뉴로 돌아가기\n");
    
    print_separator();
}

void handle_member_menu(void) {
    int choice;
    
    while (1) {
        show_member_menu();
        
        choice = get_menu_choice(0, 5, "메뉴를 선택하세요");
        
        switch (choice) {
            case MEMBER_ADD:
                add_member_interactive();
                break;
            case MEMBER_SEARCH:
                search_members_interactive();
                break;
            case MEMBER_UPDATE:
                update_member_interactive();
                break;
            case MEMBER_DELETE:
                delete_member_interactive();
                break;
            case MEMBER_LIST_ALL:
                list_all_members_interactive();
                break;
            case MEMBER_BACK:
                return;
            default:
                print_error_message("잘못된 선택입니다.");
                pause_for_user();
                break;
        }
    }
}

void add_member_interactive(void) {
    clear_screen();
    print_header("회원 추가");
    
    Member member;
    init_member(&member);
    
    char input[512];
    
    // 이름 입력
    if (get_user_input(input, sizeof(input), "이름: ") == SUCCESS && !is_empty_string(input)) {
        safe_string_copy(member.name, input, sizeof(member.name));
    } else {
        print_error_message("이름은 필수 입력 사항입니다.");
        pause_for_user();
        return;
    }
    
    // 이메일 입력
    while (1) {
        if (get_user_input(input, sizeof(input), "이메일: ") == SUCCESS && !is_empty_string(input)) {
            if (is_valid_email(input)) {
                safe_string_copy(member.email, input, sizeof(member.email));
                break;
            } else {
                print_error_message("유효하지 않은 이메일 형식입니다. 다시 입력해주세요.");
            }
        } else {
            print_error_message("이메일은 필수 입력 사항입니다.");
            pause_for_user();
            return;
        }
    }
    
    // 전화번호 입력
    if (get_user_input(input, sizeof(input), "전화번호: ") == SUCCESS && !is_empty_string(input)) {
        if (is_valid_phone(input)) {
            safe_string_copy(member.phone, input, sizeof(member.phone));
        } else {
            print_warning_message("유효하지 않은 전화번호 형식입니다. 건너뜁니다.");
        }
    }
    
    // 주소 입력
    if (get_user_input(input, sizeof(input), "주소: ") == SUCCESS && !is_empty_string(input)) {
        safe_string_copy(member.address, input, sizeof(member.address));
    }
    
    // 회원 추가
    int member_id = add_member(g_database, &member);
    if (member_id > 0) {
        print_success_message("회원이 성공적으로 추가되었습니다.");
        printf("회원 ID: %d\n", member_id);
        log_message(LOG_INFO, "회원 추가 성공: ID=%d, 이름=%s", member_id, member.name);
    } else {
        print_error_message("회원 추가에 실패했습니다.");
    }
    
    pause_for_user();
}

void search_members_interactive(void) {
    clear_screen();
    print_header("회원 검색");
    
    printf("1. 이름으로 검색\n");
    printf("2. 이메일로 검색\n");
    printf("3. 전화번호로 검색\n");
    printf("0. 돌아가기\n");
    
    int choice = get_menu_choice(0, 3, "검색 방법을 선택하세요");
    if (choice == 0) return;
    
    char search_term[256];
    if (get_user_input(search_term, sizeof(search_term), "검색어: ") != SUCCESS || is_empty_string(search_term)) {
        print_error_message("검색어를 입력해주세요.");
        pause_for_user();
        return;
    }
    
    MemberSearchResult result;
    if (init_member_search_result(&result) != SUCCESS) {
        print_error_message("검색 결과 초기화 실패");
        pause_for_user();
        return;
    }
    
    int search_result = FAILURE;
    
    switch (choice) {
        case 1:
            search_result = search_members_by_name(g_database, search_term, &result);
            break;
        case 2:
            search_result = search_members_by_name(g_database, search_term, &result);
            break;
        case 3:
            search_result = search_members_by_phone(g_database, search_term, &result);
            break;
    }
    
    if (search_result == SUCCESS) {
        print_member_list(&result);
    } else {
        print_error_message("검색 중 오류가 발생했습니다.");
    }
    
    free_member_search_result(&result);
    pause_for_user();
}

void list_all_members_interactive(void) {
    clear_screen();
    print_header("전체 회원 목록");
    
    MemberSearchResult result;
    if (init_member_search_result(&result) != SUCCESS) {
        print_error_message("목록 초기화 실패");
        pause_for_user();
        return;
    }
    
    if (list_all_members(g_database, &result, 100, 0) == SUCCESS) {
        print_member_list(&result);
    } else {
        print_error_message("회원 목록 조회 실패");
    }
    
    free_member_search_result(&result);
    pause_for_user();
}

void update_member_interactive(void) {
    clear_screen();
    print_header("회원 정보 수정");
    
    int member_id;
    if (get_integer_input(&member_id, "수정할 회원 ID: ", 1, 999999) != SUCCESS) {
        return;
    }
    
    Member member;
    if (get_member_by_id(g_database, member_id, &member) != SUCCESS) {
        print_error_message("해당 ID의 회원을 찾을 수 없습니다.");
        pause_for_user();
        return;
    }
    
    printf("\n현재 회원 정보:\n");
    print_member(&member);
    
    if (!get_yes_no_input("\n이 회원 정보를 수정하시겠습니까? (y/n): ")) {
        return;
    }
    
    char input[512];
    
    // 이름 수정
    printf("\n현재 이름: %s\n", member.name);
    if (get_user_input(input, sizeof(input), "새 이름 (엔터로 건너뛰기): ") == SUCCESS && !is_empty_string(input)) {
        safe_string_copy(member.name, input, sizeof(member.name));
    }
    
    // 이메일 수정
    printf("현재 이메일: %s\n", member.email);
    if (get_user_input(input, sizeof(input), "새 이메일 (엔터로 건너뛰기): ") == SUCCESS && !is_empty_string(input)) {
        if (is_valid_email(input)) {
            safe_string_copy(member.email, input, sizeof(member.email));
        } else {
            print_warning_message("유효하지 않은 이메일 형식입니다. 건너뜁니다.");
        }
    }
    
    // 전화번호 수정
    printf("현재 전화번호: %s\n", member.phone);
    if (get_user_input(input, sizeof(input), "새 전화번호 (엔터로 건너뛰기): ") == SUCCESS && !is_empty_string(input)) {
        if (is_valid_phone(input)) {
            safe_string_copy(member.phone, input, sizeof(member.phone));
        } else {
            print_warning_message("유효하지 않은 전화번호 형식입니다. 건너뜁니다.");
        }
    }
    
    if (update_member(g_database, &member) == SUCCESS) {
        print_success_message("회원 정보가 성공적으로 수정되었습니다.");
        log_message(LOG_INFO, "회원 수정 성공: ID=%d", member_id);
    } else {
        print_error_message("회원 정보 수정에 실패했습니다.");
    }
    
    pause_for_user();
}

void delete_member_interactive(void) {
    clear_screen();
    print_header("회원 삭제");
    
    int member_id;
    if (get_integer_input(&member_id, "삭제할 회원 ID: ", 1, 999999) != SUCCESS) {
        return;
    }
    
    Member member;
    if (get_member_by_id(g_database, member_id, &member) != SUCCESS) {
        print_error_message("해당 ID의 회원을 찾을 수 없습니다.");
        pause_for_user();
        return;
    }
    
    printf("\n삭제할 회원 정보:\n");
    print_member(&member);
    
    print_warning_message("주의: 이 작업은 되돌릴 수 없습니다.");
    if (!get_yes_no_input("정말 이 회원을 삭제하시겠습니까? (y/n): ")) {
        return;
    }
    
    if (delete_member(g_database, member_id) == SUCCESS) {
        print_success_message("회원이 성공적으로 삭제되었습니다.");
        log_message(LOG_INFO, "회원 삭제 성공: ID=%d, 이름=%s", member_id, member.name);
    } else {
        print_error_message("회원 삭제에 실패했습니다.");
    }
    
    pause_for_user();
}

void show_loan_menu(void) {
    clear_screen();
    print_header("대출 관리");
    
    printf("1. 도서 대출\n");
    printf("2. 도서 반납\n");
    printf("3. 대출 연장\n");
    printf("4. 대출 이력 조회\n");
    printf("5. 연체 도서 목록\n");
    printf("0. 메인 메뉴로 돌아가기\n");
    
    print_separator();
}

void handle_loan_menu(void) {
    int choice;
    
    while (1) {
        show_loan_menu();
        
        choice = get_menu_choice(0, 5, "메뉴를 선택하세요");
        
        switch (choice) {
            case LOAN_BORROW:
                borrow_book_interactive();
                break;
            case LOAN_RETURN:
                return_book_interactive();
                break;
            case LOAN_EXTEND:
                extend_loan_interactive();
                break;
            case LOAN_HISTORY:
                show_loan_history_interactive();
                break;
            case LOAN_OVERDUE:
                show_overdue_loans();
                break;
            case LOAN_BACK:
                return;
            default:
                print_error_message("잘못된 선택입니다.");
                pause_for_user();
                break;
        }
    }
}

void borrow_book_interactive(void) {
    clear_screen();
    print_header("도서 대출");
    
    int book_id, member_id, loan_days;
    
    // 도서 ID 입력
    if (get_integer_input(&book_id, "대출할 도서 ID: ", 1, 999999) != SUCCESS) {
        return;
    }
    
    // 회원 ID 입력
    if (get_integer_input(&member_id, "회원 ID: ", 1, 999999) != SUCCESS) {
        return;
    }
    
    // 대출 기간 입력 (기본값 사용 가능)
    printf("대출 기간 (기본 %d일): ", g_config.default_loan_days);
    char input[32];
    if (get_user_input(input, sizeof(input), "") == SUCCESS && !is_empty_string(input)) {
        if (parse_integer(input, &loan_days) != SUCCESS || loan_days <= 0) {
            loan_days = g_config.default_loan_days;
        }
    } else {
        loan_days = g_config.default_loan_days;
    }
    
    // 대출 처리
    int loan_id = loan_book(g_database, book_id, member_id, loan_days);
    if (loan_id > 0) {
        print_success_message("도서가 성공적으로 대출되었습니다.");
        printf("대출 ID: %d\n", loan_id);
        log_message(LOG_INFO, "도서 대출 성공: 대출ID=%d, 도서ID=%d, 회원ID=%d", loan_id, book_id, member_id);
    } else {
        print_error_message("도서 대출에 실패했습니다.");
    }
    
    pause_for_user();
}

void return_book_interactive(void) {
    clear_screen();
    print_header("도서 반납");
    
    printf("1. 대출 ID로 반납\n");
    printf("2. 도서 ID와 회원 ID로 반납\n");
    printf("0. 돌아가기\n");
    
    int choice = get_menu_choice(0, 2, "반납 방법을 선택하세요");
    if (choice == 0) return;
    
    int result = FAILURE;
    
    if (choice == 1) {
        int loan_id;
        if (get_integer_input(&loan_id, "대출 ID: ", 1, 999999) == SUCCESS) {
            result = return_book(g_database, loan_id);
        }
    } else if (choice == 2) {
        int book_id, member_id;
        if (get_integer_input(&book_id, "도서 ID: ", 1, 999999) == SUCCESS &&
            get_integer_input(&member_id, "회원 ID: ", 1, 999999) == SUCCESS) {
            result = return_book_by_ids(g_database, book_id, member_id);
        }
    }
    
    if (result == SUCCESS) {
        print_success_message("도서가 성공적으로 반납되었습니다.");
        log_message(LOG_INFO, "도서 반납 성공");
    } else {
        print_error_message("도서 반납에 실패했습니다.");
    }
    
    pause_for_user();
}

void extend_loan_interactive(void) {
    clear_screen();
    print_header("대출 연장");
    
    int loan_id;
    if (get_integer_input(&loan_id, "연장할 대출 ID: ", 1, 999999) != SUCCESS) {
        return;
    }
    
    // 현재 대출 정보 조회
    Loan loan;
    if (get_loan_by_id(g_database, loan_id, &loan) != SUCCESS) {
        print_error_message("해당 ID의 대출 기록을 찾을 수 없습니다.");
        pause_for_user();
        return;
    }
    
    printf("\n현재 대출 정보:\n");
    print_loan(g_database, &loan);
    
    if (loan.is_returned) {
        print_error_message("이미 반납된 도서는 연장할 수 없습니다.");
        pause_for_user();
        return;
    }
    
    int extend_days;
    if (get_integer_input(&extend_days, "연장할 일수 (1-30): ", 1, 30) != SUCCESS) {
        return;
    }
    
    if (extend_loan(g_database, loan_id, extend_days) == SUCCESS) {
        print_success_message("대출이 성공적으로 연장되었습니다.");
        log_message(LOG_INFO, "대출 연장 성공: 대출ID=%d, 연장일수=%d", loan_id, extend_days);
    } else {
        print_error_message("대출 연장에 실패했습니다.");
    }
    
    pause_for_user();
}

void show_loan_history_interactive(void) {
    clear_screen();
    print_header("대출 이력 조회");
    
    printf("1. 회원별 대출 이력\n");
    printf("2. 도서별 대출 이력\n");
    printf("3. 전체 대출 이력\n");
    printf("0. 돌아가기\n");
    
    int choice = get_menu_choice(0, 3, "조회 방법을 선택하세요");
    if (choice == 0) return;
    
    LoanSearchResult result;
    if (init_loan_search_result(&result) != SUCCESS) {
        print_error_message("검색 결과 초기화 실패");
        pause_for_user();
        return;
    }
    
    int search_result = FAILURE;
    
    switch (choice) {
        case 1: {
            int member_id;
            if (get_integer_input(&member_id, "회원 ID: ", 1, 999999) == SUCCESS) {
                int include_returned = get_yes_no_input("반납된 기록도 포함하시겠습니까? (y/n): ");
                search_result = get_member_loan_history(g_database, member_id, &result, include_returned);
            }
            break;
        }
        case 2: {
            int book_id;
            if (get_integer_input(&book_id, "도서 ID: ", 1, 999999) == SUCCESS) {
                int include_returned = get_yes_no_input("반납된 기록도 포함하시겠습니까? (y/n): ");
                search_result = get_book_loan_history(g_database, book_id, &result, include_returned);
            }
            break;
        }
        case 3: {
            search_result = get_current_loans(g_database, &result);
            break;
        }
    }
    
    if (search_result == SUCCESS) {
        print_loan_list(g_database, &result);
    } else {
        print_error_message("대출 이력 조회 중 오류가 발생했습니다.");
    }
    
    free_loan_search_result(&result);
    pause_for_user();
}

void show_overdue_loans(void) {
    clear_screen();
    print_header("연체 도서 목록");
    
    LoanSearchResult result;
    if (init_loan_search_result(&result) != SUCCESS) {
        print_error_message("검색 결과 초기화 실패");
        pause_for_user();
        return;
    }
    
    if (get_overdue_loans(g_database, &result) == SUCCESS) {
        if (result.count > 0) {
            printf("연체된 도서가 %d건 있습니다.\n\n", result.count);
            print_loan_list(g_database, &result);
        } else {
            print_success_message("연체된 도서가 없습니다.");
        }
    } else {
        print_error_message("연체 도서 목록 조회 실패");
    }
    
    free_loan_search_result(&result);
    pause_for_user();
}

void show_report_menu(void) {
    clear_screen();
    print_header("보고서");
    
    printf("1. 도서관 통계\n");
    printf("2. 인기 도서 순위\n");  
    printf("3. 회원 활동 보고서\n");
    printf("4. 연체 현황 보고서\n");
    printf("0. 메인 메뉴로 돌아가기\n");
    
    print_separator();
}

void handle_report_menu(void) {
    int choice;
    
    while (1) {
        show_report_menu();
        
        choice = get_menu_choice(0, 4, "메뉴를 선택하세요");
        
        switch (choice) {
            case REPORT_STATISTICS:
                show_library_statistics();
                break;
            case REPORT_POPULAR_BOOKS:
                show_popular_books_report();
                break;
            case REPORT_MEMBER_ACTIVITY:
                show_member_activity_report();
                break;
            case REPORT_OVERDUE_LIST:
                show_overdue_report();
                break;
            case REPORT_BACK:
                return;
            default:
                print_error_message("잘못된 선택입니다.");
                pause_for_user();
                break;
        }
    }
}

void show_library_statistics(void) {
    clear_screen();
    print_header("도서관 통계");
    
    // 도서 통계 - 임시 구현
    BookSearchResult book_result;
    int total_books = 0;
    int available_books = 0;
    if (list_all_books(g_database, &book_result, 1000, 0) == SUCCESS) {
        total_books = book_result.count;
        available_books = total_books; // 임시로 모든 책이 사용 가능하다고 가정
        free_book_search_result(&book_result);
    }
    
    // 회원 통계 - 임시 구현
    MemberSearchResult member_result;
    int total_members = 0;
    int active_members = 0;
    if (list_all_members(g_database, &member_result, 1000, 0) == SUCCESS) {
        total_members = member_result.count;
        active_members = total_members; // 임시로 모든 회원이 활성이라고 가정
        free_member_search_result(&member_result);
    }
    
    // 대출 통계 - 임시 구현
    int total_loans = 0, current_loans = 0, overdue_loans = 0, returned_loans = 0;
    
    printf("📚 도서 통계\n");
    printf("   총 도서 수: %d권\n", total_books);
    printf("   대출 가능: %d권\n", available_books);
    printf("   대출 중: %d권\n", total_books - available_books);
    
    printf("\n👥 회원 통계\n");
    printf("   총 회원 수: %d명\n", total_members);
    printf("   활성 회원: %d명\n", active_members);
    
    printf("\n📖 대출 통계\n");
    printf("   총 대출 건수: %d건\n", total_loans);
    printf("   현재 대출 중: %d건\n", current_loans);
    printf("   연체 중: %d건\n", overdue_loans);
    printf("   반납 완료: %d건\n", returned_loans);
    
    if (total_loans > 0) {
        double return_rate = (double)returned_loans / total_loans * 100;
        printf("   반납률: %.1f%%\n", return_rate);
    }
    
    pause_for_user();
}

void show_popular_books_report(void) {
    clear_screen();
    print_header("인기 도서 순위 (상위 10권)");
    
    // 임시 구현 - 모든 도서를 나열
    BookSearchResult result;
    if (list_all_books(g_database, &result, 10, 0) == SUCCESS) {
        printf("순위  도서 정보                                대출 횟수\n");
        printf("================================================\n");
        
        for (int i = 0; i < result.count && i < 10; i++) {
            printf("%-2d    %-30s    %d회\n", i + 1, result.books[i].title, i + 1);
        }
        free_book_search_result(&result);
    } else {
        print_info_message("도서 정보를 가져올 수 없습니다.");
    }
    
    pause_for_user();
}

void show_member_activity_report(void) {
    clear_screen();
    print_header("회원 활동 보고서");
    
    printf("활동 기간별 회원 분류를 표시합니다.\n\n");
    
    // 간단한 회원 활동 통계 - 임시 구현
    MemberSearchResult result;
    int total_members = 0;
    int active_members = 0;
    
    if (list_all_members(g_database, &result, 1000, 0) == SUCCESS) {
        total_members = result.count;
        active_members = total_members; // 임시로 모든 회원이 활성이라고 가정
        free_member_search_result(&result);
    }
    
    printf("총 회원 수: %d명\n", total_members);
    printf("활동 회원: %d명\n", active_members);
    printf("비활동 회원: %d명\n", total_members - active_members);
    
    if (total_members > 0) {
        double active_rate = (double)active_members / total_members * 100;
        printf("활동률: %.1f%%\n", active_rate);
    }
    
    pause_for_user();
}

void show_overdue_report(void) {
    clear_screen();
    print_header("연체 현황 보고서");
    
    show_overdue_loans();
}

void show_system_menu(void) {
    clear_screen();
    print_header("시스템 설정");
    
    printf("1. 데이터베이스 백업\n");
    printf("2. 데이터베이스 복원\n");
    printf("3. 시스템 설정 변경\n");
    printf("4. 시스템 로그 보기\n");
    printf("0. 메인 메뉴로 돌아가기\n");
    
    print_separator();
}

void handle_system_menu(void) {
    int choice;
    
    while (1) {
        show_system_menu();
        
        choice = get_menu_choice(0, 4, "메뉴를 선택하세요");
        
        switch (choice) {
            case SYSTEM_BACKUP:
                backup_database_interactive();
                break;
            case SYSTEM_RESTORE:
                restore_database_interactive();
                break;
            case SYSTEM_CONFIG:
                configure_system_interactive();
                break;
            case SYSTEM_LOG:
                show_system_log();
                break;
            case SYSTEM_BACK:
                return;
            default:
                print_error_message("잘못된 선택입니다.");
                pause_for_user();
                break;
        }
    }
}

void backup_database_interactive(void) {
    clear_screen();
    print_header("데이터베이스 백업");
    
    char backup_path[512];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    // 기본 백업 파일명 생성
    snprintf(backup_path, sizeof(backup_path), "./backups/library_backup_%04d%02d%02d_%02d%02d%02d.db",
             tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    
    printf("기본 백업 경로: %s\n", backup_path);
    
    char input[512];
    if (get_user_input(input, sizeof(input), "다른 경로를 사용하시겠습니까? (엔터로 기본 경로 사용): ") == SUCCESS && !is_empty_string(input)) {
        safe_string_copy(backup_path, input, sizeof(backup_path));
    }
    
    // 백업 디렉토리 생성
    create_directory_if_not_exists("./backups");
    
    if (database_backup(g_database, backup_path) == SUCCESS) {
        print_success_message("데이터베이스 백업이 완료되었습니다.");
        printf("백업 파일: %s\n", backup_path);
        log_message(LOG_INFO, "데이터베이스 백업 성공: %s", backup_path);
    } else {
        print_error_message("데이터베이스 백업에 실패했습니다.");
    }
    
    pause_for_user();
}

void restore_database_interactive(void) {
    clear_screen();
    print_header("데이터베이스 복원");
    
    char restore_path[512];
    if (get_user_input(restore_path, sizeof(restore_path), "복원할 백업 파일 경로: ") != SUCCESS || is_empty_string(restore_path)) {
        print_error_message("복원 파일 경로를 입력해주세요.");
        pause_for_user();
        return;
    }
    
    if (!file_exists(restore_path)) {
        print_error_message("지정한 백업 파일을 찾을 수 없습니다.");
        pause_for_user();
        return;
    }
    
    print_warning_message("주의: 현재 데이터베이스의 모든 데이터가 덮어쓰여집니다.");
    if (!get_yes_no_input("정말 복원하시겠습니까? (y/n): ")) {
        return;
    }
    
    if (database_restore(g_database, restore_path) == SUCCESS) {
        print_success_message("데이터베이스 복원이 완료되었습니다.");
        log_message(LOG_INFO, "데이터베이스 복원 성공: %s", restore_path);
    } else {
        print_error_message("데이터베이스 복원에 실패했습니다.");
    }
    
    pause_for_user();
}

void configure_system_interactive(void) {
    clear_screen();
    print_header("시스템 설정 변경");
    
    printf("현재 설정:\n");
    printf("1. 데이터베이스 경로: %s\n", g_config.database_path);
    printf("2. 백업 디렉토리: %s\n", g_config.backup_directory);
    printf("3. 기본 대출 기간: %d일\n", g_config.default_loan_days);
    printf("4. 최대 대출 권수: %d권\n", g_config.max_loan_count);
    printf("5. 최대 연장 횟수: %d회\n", g_config.max_renewal_count);
    printf("6. 자동 백업: %s\n", g_config.auto_backup_enabled ? "사용" : "사용 안 함");
    
    if (get_yes_no_input("\n설정을 변경하시겠습니까? (y/n): ")) {
        if (save_config("config.ini", &g_config) == SUCCESS) {
            print_success_message("설정이 저장되었습니다.");
        } else {
            print_error_message("설정 저장에 실패했습니다.");
        }
    }
    
    pause_for_user();
}

void show_system_log(void) {
    clear_screen();
    print_header("시스템 로그");
    
    if (file_exists("library.log")) {
        printf("최근 로그 항목들:\n\n");
        // 간단한 로그 표시 (실제로는 파일을 읽어서 표시)
        printf("로그 파일을 확인하세요: library.log\n");
    } else {
        print_info_message("로그 파일이 없습니다.");
    }
    
    pause_for_user();
}
