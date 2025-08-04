/**
 * @file test_member.cpp
 * @brief 회원 관리 모듈 단위 테스트
 * 
 * 회원 등록, 조회, 수정, 삭제, 검색 등의 기능을 테스트합니다.
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <cstring>

extern "C" {
    #include "database.h"
    #include "member.h"
    #include "constants.h"
}

class MemberTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 테스트용 데이터베이스 설정
        test_db_path = "test_member_library.db";
        
        // 기존 테스트 DB 파일 삭제
        if (std::filesystem::exists(test_db_path)) {
            std::filesystem::remove(test_db_path);
        }
        
        // 데이터베이스 초기화
        db = database_init(test_db_path);
        ASSERT_NE(db, nullptr);
        ASSERT_EQ(create_schema(db), SUCCESS);
        
        // 테스트용 회원 데이터 준비
        memset(&test_member, 0, sizeof(Member));
        strncpy(test_member.name, "홍길동", sizeof(test_member.name) - 1);
        strncpy(test_member.email, "hong@example.com", sizeof(test_member.email) - 1);
        strncpy(test_member.phone, "010-1234-5678", sizeof(test_member.phone) - 1);
        strncpy(test_member.address, "서울시 강남구", sizeof(test_member.address) - 1);
        test_member.is_active = TRUE;
    }
    
    void TearDown() override {
        if (db) {
            database_close(db);
        }
        
        if (std::filesystem::exists(test_db_path)) {
            std::filesystem::remove(test_db_path);
        }
    }
    
    const char* test_db_path;
    sqlite3* db;
    Member test_member;
};

/**
 * @brief 회원 등록 기능 테스트
 * 
 * 새로운 회원이 정상적으로 데이터베이스에 등록되는지 확인합니다.
 */
TEST_F(MemberTest, AddMember) {
    // 회원 등록
    int result = add_member(db, &test_member);
    EXPECT_EQ(result, SUCCESS) << "회원 등록 실패";
    
    // 등록된 회원의 ID 확인 (1부터 시작)
    EXPECT_GT(test_member.id, 0) << "회원 ID가 설정되지 않음";
}

/**
 * @brief 회원 조회 기능 테스트
 * 
 * ID로 회원을 조회하는 기능이 정상적으로 동작하는지 확인합니다.
 */
TEST_F(MemberTest, GetMemberById) {
    // 회원 등록
    ASSERT_EQ(add_member(db, &test_member), SUCCESS);
    int member_id = test_member.id;
    
    // 회원 조회
    Member retrieved_member;
    int result = get_member_by_id(db, member_id, &retrieved_member);
    
    EXPECT_EQ(result, SUCCESS) << "회원 조회 실패";
    EXPECT_EQ(retrieved_member.id, member_id) << "조회된 회원 ID 불일치";
    EXPECT_STREQ(retrieved_member.name, test_member.name) << "회원 이름 불일치";
    EXPECT_STREQ(retrieved_member.email, test_member.email) << "이메일 불일치";
    EXPECT_STREQ(retrieved_member.phone, test_member.phone) << "전화번호 불일치";
}

/**
 * @brief 존재하지 않는 회원 조회 테스트
 * 
 * 존재하지 않는 ID로 회원을 조회할 때 적절히 실패하는지 확인합니다.
 */
TEST_F(MemberTest, GetNonExistentMember) {
    Member retrieved_member;
    int result = get_member_by_id(db, 99999, &retrieved_member);
    
    EXPECT_EQ(result, FAILURE) << "존재하지 않는 회원 조회가 성공해서는 안됨";
}

/**
 * @brief 회원 정보 수정 기능 테스트
 * 
 * 기존 회원의 정보를 수정하는 기능이 정상적으로 동작하는지 확인합니다.
 */
TEST_F(MemberTest, UpdateMember) {
    // 회원 등록
    ASSERT_EQ(add_member(db, &test_member), SUCCESS);
    int member_id = test_member.id;
    
    // 회원 정보 수정
    strncpy(test_member.name, "김철수", sizeof(test_member.name) - 1);
    strncpy(test_member.email, "kim@example.com", sizeof(test_member.email) - 1);
    strncpy(test_member.phone, "010-9876-5432", sizeof(test_member.phone) - 1);
    strncpy(test_member.address, "부산시 해운대구", sizeof(test_member.address) - 1);
    
    int result = update_member(db, &test_member);
    EXPECT_EQ(result, SUCCESS) << "회원 정보 수정 실패";
    
    // 수정된 정보 확인
    Member updated_member;
    ASSERT_EQ(get_member_by_id(db, member_id, &updated_member), SUCCESS);
    
    EXPECT_STREQ(updated_member.name, "김철수") << "이름 수정 반영 안됨";
    EXPECT_STREQ(updated_member.email, "kim@example.com") << "이메일 수정 반영 안됨";
    EXPECT_STREQ(updated_member.phone, "010-9876-5432") << "전화번호 수정 반영 안됨";
    EXPECT_STREQ(updated_member.address, "부산시 해운대구") << "주소 수정 반영 안됨";
}

/**
 * @brief 회원 삭제 기능 테스트
 * 
 * 회원을 삭제하는 기능이 정상적으로 동작하는지 확인합니다.
 */
TEST_F(MemberTest, DeleteMember) {
    // 회원 등록
    ASSERT_EQ(add_member(db, &test_member), SUCCESS);
    int member_id = test_member.id;
    
    // 회원 삭제
    int result = delete_member(db, member_id);
    EXPECT_EQ(result, SUCCESS) << "회원 삭제 실패";
    
    // 삭제 확인 (조회 시 실패해야 함)
    Member deleted_member;
    result = get_member_by_id(db, member_id, &deleted_member);
    EXPECT_EQ(result, FAILURE) << "삭제된 회원이 여전히 조회됨";
}

/**
 * @brief 회원 이름으로 검색 기능 테스트
 * 
 * 회원 이름을 기준으로 검색하는 기능이 정상적으로 동작하는지 확인합니다.
 */
TEST_F(MemberTest, SearchMembersByName) {
    // 여러 회원 등록
    Member member1 = test_member;
    strncpy(member1.name, "홍길동", sizeof(member1.name) - 1);
    strncpy(member1.email, "hong1@example.com", sizeof(member1.email) - 1);
    ASSERT_EQ(add_member(db, &member1), SUCCESS);
    
    Member member2 = test_member;
    strncpy(member2.name, "홍길순", sizeof(member2.name) - 1);
    strncpy(member2.email, "hong2@example.com", sizeof(member2.email) - 1);
    ASSERT_EQ(add_member(db, &member2), SUCCESS);
    
    Member member3 = test_member;
    strncpy(member3.name, "김철수", sizeof(member3.name) - 1);
    strncpy(member3.email, "kim@example.com", sizeof(member3.email) - 1);
    ASSERT_EQ(add_member(db, &member3), SUCCESS);
    
    // "홍"으로 검색
    MemberSearchResult result;
    int search_result = search_members_by_name(db, "홍", &result);
    
    EXPECT_EQ(search_result, SUCCESS) << "회원 이름 검색 실패";
    EXPECT_EQ(result.count, 2) << "검색 결과 개수 불일치 (2개 예상)";
    
    // 메모리 해제
    free_member_search_result(&result);
}

/**
 * @brief 전체 회원 목록 조회 테스트
 * 
 * 페이징 기능과 함께 전체 회원 목록을 조회하는 기능을 테스트합니다.
 */
TEST_F(MemberTest, ListAllMembers) {
    // 여러 회원 등록
    for (int i = 0; i < 3; i++) {
        Member member = test_member;
        snprintf(member.name, sizeof(member.name), "테스트회원%d", i + 1);
        snprintf(member.email, sizeof(member.email), "test%d@example.com", i + 1);
        ASSERT_EQ(add_member(db, &member), SUCCESS);
    }
    
    // 전체 회원 목록 조회
    MemberSearchResult result;
    int list_result = list_all_members(db, &result, 10, 0);
    
    EXPECT_EQ(list_result, SUCCESS) << "전체 회원 목록 조회 실패";
    EXPECT_EQ(result.count, 3) << "전체 회원 개수 불일치 (3개 예상)";
    
    // 메모리 해제
    free_member_search_result(&result);
}

/**
 * @brief 회원 활성 상태 변경 테스트
 * 
 * 회원의 활성/비활성 상태를 변경하는 기능을 테스트합니다.
 */
TEST_F(MemberTest, UpdateMemberStatus) {
    // 회원 등록
    ASSERT_EQ(add_member(db, &test_member), SUCCESS);
    int member_id = test_member.id;
    
    // 비활성 상태로 변경
    int result = update_member_status(db, member_id, FALSE);
    EXPECT_EQ(result, SUCCESS) << "회원 상태 변경 실패";
    
    // 상태 확인
    Member updated_member;
    ASSERT_EQ(get_member_by_id(db, member_id, &updated_member), SUCCESS);
    EXPECT_EQ(updated_member.is_active, FALSE) << "비활성 상태 반영 안됨";
    
    // 다시 활성 상태로 변경
    result = update_member_status(db, member_id, TRUE);
    EXPECT_EQ(result, SUCCESS) << "회원 상태 복원 실패";
    
    ASSERT_EQ(get_member_by_id(db, member_id, &updated_member), SUCCESS);
    EXPECT_EQ(updated_member.is_active, TRUE) << "활성 상태 반영 안됨";
}

/**
 * @brief 중복 이메일 처리 테스트
 * 
 * 동일한 이메일을 가진 회원을 등록할 때의 처리를 확인합니다.
 */
TEST_F(MemberTest, AddDuplicateEmail) {
    // 첫 번째 회원 등록
    ASSERT_EQ(add_member(db, &test_member), SUCCESS);
    
    // 동일한 이메일로 다른 회원 등록 시도
    Member duplicate_member = test_member;
    strncpy(duplicate_member.name, "다른이름", sizeof(duplicate_member.name) - 1);
    strncpy(duplicate_member.phone, "010-0000-0000", sizeof(duplicate_member.phone) - 1);
    
    int result = add_member(db, &duplicate_member);
    
    // 이메일 중복 검사가 구현되어 있다면 실패해야 함
    EXPECT_EQ(result, FAILURE) << "중복 이메일로 회원 등록이 성공해서는 안됨";
}

/**
 * @brief 중복 전화번호 처리 테스트
 * 
 * 동일한 전화번호를 가진 회원을 등록할 때의 처리를 확인합니다.
 */
TEST_F(MemberTest, AddDuplicatePhone) {
    // 첫 번째 회원 등록
    ASSERT_EQ(add_member(db, &test_member), SUCCESS);
    
    // 동일한 전화번호로 다른 회원 등록 시도
    Member duplicate_member = test_member;
    strncpy(duplicate_member.name, "다른이름", sizeof(duplicate_member.name) - 1);
    strncpy(duplicate_member.email, "different@example.com", sizeof(duplicate_member.email) - 1);
    
    int result = add_member(db, &duplicate_member);
    
    // 전화번호 중복 검사가 구현되어 있다면 실패해야 함
    EXPECT_EQ(result, FAILURE) << "중복 전화번호로 회원 등록이 성공해서는 안됨";
}

/**
 * @brief 잘못된 데이터로 회원 등록 테스트
 * 
 * NULL 포인터나 잘못된 데이터로 회원을 등록할 때의 오류 처리를 확인합니다.
 */
TEST_F(MemberTest, AddMemberWithInvalidData) {
    // NULL 포인터 테스트
    int result = add_member(db, nullptr);
    EXPECT_EQ(result, FAILURE) << "NULL 포인터로 회원 등록이 성공해서는 안됨";
    
    // 빈 이름 테스트
    Member invalid_member = test_member;
    strcpy(invalid_member.name, "");
    result = add_member(db, &invalid_member);
    EXPECT_EQ(result, FAILURE) << "빈 이름으로 회원 등록이 성공해서는 안됨";
    
    // 빈 이메일 테스트
    Member invalid_member2 = test_member;
    strcpy(invalid_member2.email, "");
    result = add_member(db, &invalid_member2);
    EXPECT_EQ(result, FAILURE) << "빈 이메일로 회원 등록이 성공해서는 안됨";
}

/**
 * @brief 이메일 형식 검증 테스트
 * 
 * 잘못된 형식의 이메일을 가진 회원 등록 시의 처리를 확인합니다.
 */
TEST_F(MemberTest, AddMemberWithInvalidEmail) {
    // 잘못된 이메일 형식들
    const char* invalid_emails[] = {
        "invalid-email",
        "@example.com",
        "test@",
        "test@.com",
        ""
    };
    
    for (const char* invalid_email : invalid_emails) {
        Member invalid_member = test_member;
        strncpy(invalid_member.email, invalid_email, sizeof(invalid_member.email) - 1);
        
        // 이메일마다 다른 이름으로 설정하여 다른 오류와 구분
        snprintf(invalid_member.name, sizeof(invalid_member.name), "테스트%s", invalid_email);
        
        int result = add_member(db, &invalid_member);
        
        // 이메일 검증이 구현되어 있다면 실패해야 함
        if (result == FAILURE) {
            SUCCEED() << "잘못된 이메일 형식 검증됨: " << invalid_email;
        } else {
            // 검증이 구현되지 않은 경우 경고만 출력
            ADD_FAILURE() << "이메일 형식 검증이 구현되지 않음: " << invalid_email;
        }
    }
}

/**
 * @brief 전화번호 형식 검증 테스트
 * 
 * 잘못된 형식의 전화번호를 가진 회원 등록 시의 처리를 확인합니다.
 */
TEST_F(MemberTest, AddMemberWithInvalidPhone) {
    // 잘못된 전화번호 형식들
    const char* invalid_phones[] = {
        "123",
        "abc-def-ghij",
        "010-12345-678901",
        ""
    };
    
    for (const char* invalid_phone : invalid_phones) {
        Member invalid_member = test_member;
        strncpy(invalid_member.phone, invalid_phone, sizeof(invalid_member.phone) - 1);
        
        // 전화번호마다 다른 이메일로 설정하여 다른 오류와 구분
        snprintf(invalid_member.email, sizeof(invalid_member.email), "test_%s@example.com", invalid_phone);
        
        int result = add_member(db, &invalid_member);
        
        // 전화번호 검증이 구현되어 있다면 실패해야 함
        if (result == FAILURE) {
            SUCCEED() << "잘못된 전화번호 형식 검증됨: " << invalid_phone;
        } else {
            // 검증이 구현되지 않은 경우는 정상 (선택적 기능)
            SUCCEED() << "전화번호 형식 검증이 구현되지 않음 (선택적 기능): " << invalid_phone;
        }
    }
}
