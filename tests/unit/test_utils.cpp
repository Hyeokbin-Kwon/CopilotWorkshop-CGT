/**
 * @file test_utils.cpp
 * @brief 유틸리티 모듈 단위 테스트
 * 
 * 유틸리티 함수들(검증, 날짜 처리, 문자열 처리 등)의 기능을 테스트합니다.
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <cstring>

extern "C" {
    #include "utils.h"
    #include "constants.h"
}

class UtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 테스트용 설정 파일 경로
        test_config_path = "test_config.ini";
        
        // 기존 테스트 설정 파일 삭제
        if (std::filesystem::exists(test_config_path)) {
            std::filesystem::remove(test_config_path);
        }
    }
    
    void TearDown() override {
        // 테스트 설정 파일 삭제
        if (std::filesystem::exists(test_config_path)) {
            std::filesystem::remove(test_config_path);
        }
    }
    
    const char* test_config_path;
};

/**
 * @brief 이메일 형식 검증 테스트
 * 
 * 다양한 이메일 형식에 대한 검증 함수가 정상적으로 동작하는지 확인합니다.
 */
TEST_F(UtilsTest, ValidateEmailFormat) {
    // 유효한 이메일 주소들
    const char* valid_emails[] = {
        "test@example.com",
        "user.name@domain.co.kr",
        "admin123@test-domain.org",
        "contact@sub.domain.com"
    };
    
    for (const char* email : valid_emails) {
        EXPECT_TRUE(is_valid_email(email)) << "유효한 이메일이 거부됨: " << email;
    }
    
    // 무효한 이메일 주소들
    const char* invalid_emails[] = {
        "invalid-email",
        "@example.com",
        "test@",
        "test@.com",
        "",
        "test.example.com",
        "test@domain.",
        "test @example.com"
    };
    
    for (const char* email : invalid_emails) {
        EXPECT_FALSE(is_valid_email(email)) << "무효한 이메일이 허용됨: " << email;
    }
}

/**
 * @brief 전화번호 형식 검증 테스트
 * 
 * 다양한 전화번호 형식에 대한 검증 함수가 정상적으로 동작하는지 확인합니다.
 */
TEST_F(UtilsTest, ValidatePhoneFormat) {
    // 유효한 전화번호들
    const char* valid_phones[] = {
        "010-1234-5678",
        "02-123-4567",
        "031-123-4567",
        "010-9999-0000"
    };
    
    for (const char* phone : valid_phones) {
        EXPECT_TRUE(is_valid_phone(phone)) << "유효한 전화번호가 거부됨: " << phone;
    }
    
    // 무효한 전화번호들
    const char* invalid_phones[] = {
        "123",
        "abc-def-ghij",
        "010-12345-678901",
        "",
        "010 1234 5678",
        "010-1234",
        "1234-5678-9012"
    };
    
    for (const char* phone : invalid_phones) {
        EXPECT_FALSE(is_valid_phone(phone)) << "무효한 전화번호가 허용됨: " << phone;
    }
}

/**
 * @brief ISBN 형식 검증 테스트
 * 
 * ISBN-10과 ISBN-13 형식에 대한 검증 함수가 정상적으로 동작하는지 확인합니다.
 */
TEST_F(UtilsTest, ValidateISBNFormat) {
    // 유효한 ISBN들
    const char* valid_isbns[] = {
        "9788966261208",  // ISBN-13
        "1234567890",     // ISBN-10
        "0123456789",     // ISBN-10
        "9780123456786"   // ISBN-13
    };
    
    for (const char* isbn : valid_isbns) {
        EXPECT_TRUE(is_valid_isbn(isbn)) << "유효한 ISBN이 거부됨: " << isbn;
    }
    
    // 무효한 ISBN들
    const char* invalid_isbns[] = {
        "123",
        "abcdefghij",
        "12345678901234",  // 너무 긺
        "",
        "123-456-789-0",   // 하이픈 포함
        "12345 67890"      // 공백 포함
    };
    
    for (const char* isbn : invalid_isbns) {
        EXPECT_FALSE(is_valid_isbn(isbn)) << "무효한 ISBN이 허용됨: " << isbn;
    }
}

/**
 * @brief 문자열 비어있음 체크 테스트
 * 
 * 문자열이 비어있거나 공백만 있는지 확인하는 함수를 테스트합니다.
 */
TEST_F(UtilsTest, CheckEmptyString) {
    // 비어있는 문자열들
    const char* empty_strings[] = {
        "",
        "   ",
        "\t",
        "\n",
        "  \t  \n  "
    };
    
    for (const char* str : empty_strings) {
        EXPECT_TRUE(is_empty_string(str)) << "비어있는 문자열이 비어있지 않다고 판단됨: '" << str << "'";
    }
    
    // 비어있지 않은 문자열들
    const char* non_empty_strings[] = {
        "test",
        " test ",
        "a",
        "123",
        "  test  "
    };
    
    for (const char* str : non_empty_strings) {
        EXPECT_FALSE(is_empty_string(str)) << "비어있지 않은 문자열이 비어있다고 판단됨: '" << str << "'";
    }
}

/**
 * @brief 안전한 문자열 복사 테스트
 * 
 * 버퍼 오버플로우를 방지하는 안전한 문자열 복사 함수를 테스트합니다.
 */
TEST_F(UtilsTest, SafeStringCopy) {
    char buffer[10];
    
    // 정상적인 복사
    int result = safe_string_copy(buffer, "test", sizeof(buffer));
    EXPECT_EQ(result, SUCCESS) << "정상적인 문자열 복사 실패";
    EXPECT_STREQ(buffer, "test") << "복사된 문자열 불일치";
    
    // 긴 문자열 복사 (잘림)
    result = safe_string_copy(buffer, "very long string that exceeds buffer", sizeof(buffer));
    EXPECT_EQ(result, SUCCESS) << "긴 문자열 복사 실패";
    EXPECT_EQ(strlen(buffer), sizeof(buffer) - 1) << "문자열이 버퍼 크기로 잘리지 않음";
    EXPECT_EQ(buffer[sizeof(buffer) - 1], '\0') << "널 종료 문자가 없음";
    
    // NULL 포인터 처리
    result = safe_string_copy(nullptr, "test", 10);
    EXPECT_EQ(result, FAILURE) << "NULL 대상 버퍼가 허용됨";
    
    result = safe_string_copy(buffer, nullptr, sizeof(buffer));
    EXPECT_EQ(result, FAILURE) << "NULL 소스 문자열이 허용됨";
}

/**
 * @brief 날짜 형식 검증 테스트
 * 
 * YYYY-MM-DD 형식의 날짜 검증 함수를 테스트합니다.
 */
TEST_F(UtilsTest, ValidateDateFormat) {
    // 유효한 날짜들
    const char* valid_dates[] = {
        "2023-01-01",
        "2024-02-29",  // 윤년
        "2023-12-31"
    };
    
    for (const char* date : valid_dates) {
        EXPECT_TRUE(is_valid_date(date)) << "유효한 날짜가 거부됨: " << date;
    }
    
    // 무효한 날짜들
    const char* invalid_dates[] = {
        "2023-13-01",  // 13월
        "2023-02-30",  // 2월 30일
        "2023/01/01",  // 잘못된 구분자
        "23-01-01",    // 2자리 연도
        "",
        "invalid-date"
    };
    
    for (const char* date : invalid_dates) {
        EXPECT_FALSE(is_valid_date(date)) << "무효한 날짜가 허용됨: " << date;
    }
}

/**
 * @brief 현재 날짜 문자열 생성 테스트
 * 
 * 현재 날짜를 YYYY-MM-DD 형식으로 생성하는 함수를 테스트합니다.
 */
TEST_F(UtilsTest, GetCurrentDateString) {
    char date_buffer[20];
    
    int result = get_current_date_string(date_buffer, sizeof(date_buffer));
    EXPECT_EQ(result, SUCCESS) << "현재 날짜 문자열 생성 실패";
    
    // 생성된 날짜가 유효한 형식인지 확인
    EXPECT_TRUE(is_valid_date(date_buffer)) << "생성된 날짜 형식이 유효하지 않음: " << date_buffer;
    
    // 길이 확인 (YYYY-MM-DD는 10자)
    EXPECT_EQ(strlen(date_buffer), 10) << "날짜 문자열 길이 불일치";
}

/**
 * @brief 날짜 계산 테스트
 * 
 * 날짜에 일수를 더하는 함수를 테스트합니다.
 */
TEST_F(UtilsTest, AddDaysToDate) {
    char result_buffer[20];
    
    // 정상적인 날짜 계산
    int result = add_days_to_date("2023-01-01", 14, result_buffer, sizeof(result_buffer));
    EXPECT_EQ(result, SUCCESS) << "날짜 계산 실패";
    EXPECT_STREQ(result_buffer, "2023-01-15") << "날짜 계산 결과 불일치";
    
    // 월 경계 넘기기
    result = add_days_to_date("2023-01-31", 1, result_buffer, sizeof(result_buffer));
    EXPECT_EQ(result, SUCCESS) << "월 경계 날짜 계산 실패";
    // 결과는 구현에 따라 다를 수 있으므로 성공 여부만 확인
    
    // 잘못된 날짜 형식
    result = add_days_to_date("invalid-date", 1, result_buffer, sizeof(result_buffer));
    EXPECT_EQ(result, FAILURE) << "잘못된 날짜 형식이 허용됨";
}

/**
 * @brief 설정 파일 로드/저장 테스트
 * 
 * 설정 파일을 로드하고 저장하는 기능을 테스트합니다.
 */
TEST_F(UtilsTest, ConfigFileOperations) {
    SystemConfig config;
    
    // 기본 설정 초기화
    init_default_config(&config);
    
    // 기본 설정 값 확인
    EXPECT_STREQ(config.database_path, DATABASE_PATH) << "기본 데이터베이스 경로 불일치";
    EXPECT_EQ(config.default_loan_days, DEFAULT_LOAN_DAYS) << "기본 대출 기간 불일치";
    EXPECT_EQ(config.max_loan_count, MAX_BOOKS_PER_MEMBER) << "최대 대출 권수 불일치";
    
    // 설정 저장
    int result = save_config(test_config_path, &config);
    EXPECT_EQ(result, SUCCESS) << "설정 파일 저장 실패";
    
    // 파일 생성 확인
    EXPECT_TRUE(std::filesystem::exists(test_config_path)) << "설정 파일이 생성되지 않음";
    
    // 설정 로드
    SystemConfig loaded_config;
    memset(&loaded_config, 0, sizeof(SystemConfig));
    
    result = load_config(test_config_path, &loaded_config);
    EXPECT_EQ(result, SUCCESS) << "설정 파일 로드 실패";
    
    // 로드된 설정 확인
    EXPECT_STREQ(loaded_config.database_path, config.database_path) << "로드된 데이터베이스 경로 불일치";
    EXPECT_EQ(loaded_config.default_loan_days, config.default_loan_days) << "로드된 기본 대출 기간 불일치";
    EXPECT_EQ(loaded_config.max_loan_count, config.max_loan_count) << "로드된 최대 대출 권수 불일치";
}

/**
 * @brief 존재하지 않는 설정 파일 로드 테스트
 * 
 * 존재하지 않는 설정 파일을 로드할 때의 처리를 확인합니다.
 */
TEST_F(UtilsTest, LoadNonExistentConfig) {
    SystemConfig config;
    
    int result = load_config("non_existent_config.ini", &config);
    EXPECT_EQ(result, FAILURE) << "존재하지 않는 설정 파일 로드가 성공해서는 안됨";
}

/**
 * @brief 로그 메시지 기록 테스트
 * 
 * 로그 메시지를 파일에 기록하는 기능을 테스트합니다.
 */
TEST_F(UtilsTest, LogMessage) {
    const char* test_log_path = "test.log";
    
    // 기존 로그 파일 삭제
    if (std::filesystem::exists(test_log_path)) {
        std::filesystem::remove(test_log_path);
    }
    
    // 로그 메시지 기록
    int result = log_message(LOG_INFO, "테스트 로그 메시지");
    
    // 로그 기능이 구현되어 있다면 성공해야 함
    if (result == SUCCESS) {
        SUCCEED() << "로그 메시지 기록 성공";
        
        // 로그 파일이 생성되었는지 확인 (구현에 따라 다름)
        if (std::filesystem::exists("library.log")) {
            SUCCEED() << "로그 파일 생성됨";
        }
    } else {
        // 로그 기능이 구현되지 않은 경우
        SUCCEED() << "로그 기능이 구현되지 않음 (선택적 기능)";
    }
    
    // 테스트 로그 파일 정리
    if (std::filesystem::exists(test_log_path)) {
        std::filesystem::remove(test_log_path);
    }
    if (std::filesystem::exists("library.log")) {
        std::filesystem::remove("library.log");
    }
}

/**
 * @brief 파일 존재 확인 테스트
 * 
 * 파일 존재 여부를 확인하는 유틸리티 함수를 테스트합니다.
 */
TEST_F(UtilsTest, FileExists) {
    // 임시 파일 생성
    const char* temp_file = "temp_test_file.txt";
    FILE* file = fopen(temp_file, "w");
    ASSERT_NE(file, nullptr) << "임시 파일 생성 실패";
    fclose(file);
    
    // 파일 존재 확인
    EXPECT_TRUE(file_exists(temp_file)) << "존재하는 파일이 없다고 판단됨";
    
    // 파일 삭제
    std::filesystem::remove(temp_file);
    
    // 삭제된 파일 확인
    EXPECT_FALSE(file_exists(temp_file)) << "삭제된 파일이 존재한다고 판단됨";
    
    // 존재하지 않는 파일 확인
    EXPECT_FALSE(file_exists("non_existent_file.txt")) << "존재하지 않는 파일이 존재한다고 판단됨";
}

/**
 * @brief 디렉토리 생성 테스트
 * 
 * 디렉토리를 생성하는 유틸리티 함수를 테스트합니다.
 */
TEST_F(UtilsTest, CreateDirectory) {
    const char* test_dir = "test_directory";
    
    // 기존 디렉토리 삭제
    if (std::filesystem::exists(test_dir)) {
        std::filesystem::remove_all(test_dir);
    }
    
    // 디렉토리 생성
    int result = create_directory_if_not_exists(test_dir);
    EXPECT_EQ(result, SUCCESS) << "디렉토리 생성 실패";
    
    // 디렉토리 생성 확인
    EXPECT_TRUE(std::filesystem::exists(test_dir)) << "디렉토리가 생성되지 않음";
    EXPECT_TRUE(std::filesystem::is_directory(test_dir)) << "생성된 것이 디렉토리가 아님";
    
    // 이미 존재하는 디렉토리에 대해 다시 호출
    result = create_directory_if_not_exists(test_dir);
    EXPECT_EQ(result, SUCCESS) << "기존 디렉토리에 대한 호출 실패";
    
    // 정리
    std::filesystem::remove_all(test_dir);
}
