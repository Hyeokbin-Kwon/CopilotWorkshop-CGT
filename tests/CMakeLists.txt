# 테스트용 CMakeLists.txt
cmake_minimum_required(VERSION 3.14)

# GoogleTest 찾기
find_package(GTest REQUIRED)

# 테스트 디렉토리 설정
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

# 공통 include 디렉토리
include_directories(${INCLUDE_DIR})
include_directories(${SRC_DIR}/external/sqlite)

# 소스 파일들
set(LIBRARY_SOURCES
    ${SRC_DIR}/database.c
    ${SRC_DIR}/book.c
    ${SRC_DIR}/member.c
    ${SRC_DIR}/loan.c
    ${SRC_DIR}/utils.c
    ${SRC_DIR}/external/sqlite/sqlite3.c
)

# 각 테스트 실행 파일 생성
function(create_test test_name test_source)
    add_executable(${test_name} ${test_source} ${LIBRARY_SOURCES})
    target_link_libraries(${test_name} GTest::gtest GTest::gtest_main)
    
    # Windows에서 필요한 라이브러리
    if(WIN32)
        target_link_libraries(${test_name} ws2_32)
    endif()
    
    # CTest에 등록
    gtest_discover_tests(${test_name})
endfunction()

# 단위 테스트들
create_test(test_database unit/test_database.cpp)
create_test(test_book unit/test_book.cpp)
create_test(test_member unit/test_member.cpp)
create_test(test_loan unit/test_loan.cpp)
create_test(test_utils unit/test_utils.cpp)

# 통합 테스트들
create_test(test_integration integration/test_integration.cpp)

# 테스트 활성화
enable_testing()
