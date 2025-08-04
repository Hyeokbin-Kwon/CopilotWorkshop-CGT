cmake_minimum_required(VERSION 3.14)
project(library_management_system C CXX)

# C17 표준 사용
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# C++17 표준 사용 (GoogleTest를 위해)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 디버그 정보 포함
set(CMAKE_BUILD_TYPE Debug)

# 컴파일러 플래그 설정
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -g)
endif()

# 헤더 파일 디렉토리 설정
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src/external/sqlite)

# SQLite 라이브러리 빌드
add_library(sqlite3 STATIC
    src/external/sqlite/sqlite3.c
)

# 메인 라이브러리 소스 파일들 (나중에 추가될 예정)
set(LIBRARY_SOURCES
    # src/database.c
    # src/book.c
    # src/member.c
    # src/loan.c
    # src/utils.c
)

# 메인 라이브러리 생성 (소스가 추가되면 활성화)
# add_library(library_system STATIC ${LIBRARY_SOURCES})
# target_link_libraries(library_system sqlite3)

# 메인 실행 파일 (나중에 추가될 예정)
# add_executable(library_management src/main.c)
# target_link_libraries(library_management library_system sqlite3)

# GoogleTest 설정
enable_testing()
add_subdirectory(src/external/googletest)

# 테스트 빌드 (소스가 추가되면 활성화)
# add_executable(library_tests
#     tests/test_database.cpp
#     tests/test_book.cpp
#     tests/test_member.cpp
#     tests/test_loan.cpp
#     tests/test_utils.cpp
# )
# target_link_libraries(library_tests library_system sqlite3 gtest gtest_main)

# 테스트 등록
# add_test(NAME library_tests COMMAND library_tests)

# 출력 디렉토리 설정
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
