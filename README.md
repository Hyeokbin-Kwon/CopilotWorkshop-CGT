# 도서관 관리 시스템 (Library Management System)

C 언어로 구현된 완전한 도서관 관리 시스템입니다. SQLite 데이터베이스를 사용하여 도서, 회원, 대출 정보를 관리하며, 직관적인 명령줄 인터페이스를 제공합니다.

## 🚀 주요 기능

### 📚 도서 관리
- 도서 등록, 수정, 삭제
- 제목, 저자, 카테고리별 검색
- ISBN 기반 도서 식별
- 전체 도서 목록 조회

### 👥 회원 관리  
- 회원 가입, 정보 수정, 탈퇴
- 이메일, 전화번호 중복 검사
- 회원 이름 검색
- 회원 활성/비활성 상태 관리

### 📖 대출 관리
- 도서 대출/반납 처리
- 대출 기간 연장 (최대 2회)
- 회원별/도서별 대출 이력 조회
- 현재 대출 현황 및 연체 관리
- 최대 대출 권수 제한 (5권)

### 📊 보고서 시스템
- 도서관 통계 (총 도서/회원/대출 현황)
- 인기 도서 순위 (대출 횟수 기준)
- 회원 활동 보고서
- 연체 현황 보고서

### ⚙️ 시스템 관리
- 데이터베이스 백업/복원
- 시스템 설정 변경
- 로그 관리
- 자동 백업 기능

## 🛠️ 빌드 및 설치

### 사전 요구사항
- GCC 컴파일러 (MinGW-w64 권장, Windows)
- CMake 3.14 이상 (선택사항)
- GoogleTest (테스트 실행 시)

### Windows에서 빌드

#### 방법 1: 직접 컴파일
```bash
# 모든 소스 파일을 한 번에 컴파일
gcc -o library_management.exe src/main.c src/database.c src/book.c src/member.c src/loan.c src/utils.c src/external/sqlite/sqlite3.c -Iinclude -Isrc/external/sqlite

# 실행
.\library_management.exe
```

#### 방법 2: 단계별 컴파일
```bash
# 각 모듈 개별 컴파일
gcc -c src/database.c -Iinclude -Isrc/external/sqlite -o database.o
gcc -c src/book.c -Iinclude -Isrc/external/sqlite -o book.o
gcc -c src/member.c -Iinclude -Isrc/external/sqlite -o member.o
gcc -c src/loan.c -Iinclude -Isrc/external/sqlite -o loan.o
gcc -c src/utils.c -Iinclude -Isrc/external/sqlite -o utils.o
gcc -c src/main.c -Iinclude -Isrc/external/sqlite -o main.o
gcc -c src/external/sqlite/sqlite3.c -Isrc/external/sqlite -o sqlite3.o

# 링킹
gcc database.o book.o member.o loan.o utils.o main.o sqlite3.o -o library_management.exe
```

### Linux/macOS에서 빌드
```bash
# 컴파일
gcc -o library_management src/main.c src/database.c src/book.c src/member.c src/loan.c src/utils.c src/external/sqlite/sqlite3.c -Iinclude -Isrc/external/sqlite -lm -lpthread -ldl

# 실행
./library_management
```

## 🧪 테스트 실행

### 기본 기능 테스트 (C)
```bash
cd tests
.\run_tests.bat
```

### 단위 테스트 (GoogleTest 필요)
```bash
# CMake로 빌드
mkdir build && cd build
cmake .. -DBUILD_TESTING=ON
make
ctest
```

## 📖 사용법

### 프로그램 실행
```bash
.\library_management.exe
```

### 메인 메뉴
```
=================================================
        도서관 관리 시스템
        Library Management System
=================================================

1. 도서 관리
2. 회원 관리  
3. 대출 관리
4. 보고서
5. 시스템 설정
0. 종료
```

### 주요 기능 사용 예시

#### 도서 등록
1. 메인 메뉴에서 "1. 도서 관리" 선택
2. "1. 도서 추가" 선택
3. 도서 정보 입력 (제목, 저자, ISBN, 출판사, 카테고리, 출판연도)

#### 회원 가입
1. 메인 메뉴에서 "2. 회원 관리" 선택
2. "1. 회원 등록" 선택  
3. 회원 정보 입력 (이름, 이메일, 전화번호, 주소)

#### 도서 대출
1. 메인 메뉴에서 "3. 대출 관리" 선택
2. "1. 도서 대출" 선택
3. 회원 ID와 도서 ID 입력

## 🗄️ 데이터베이스 스키마

### books 테이블
| 컬럼명 | 타입 | 설명 |
|--------|------|------|
| id | INTEGER PRIMARY KEY | 도서 ID |
| title | TEXT NOT NULL | 도서 제목 |
| author | TEXT NOT NULL | 저자 |
| isbn | TEXT UNIQUE | ISBN |
| publisher | TEXT | 출판사 |
| publication_year | INTEGER | 출판연도 |
| total_copies | INTEGER | 총 보유 권수 |
| available_copies | INTEGER | 대출 가능 권수 |
| category | TEXT | 카테고리 |
| created_at | DATETIME | 등록일 |
| updated_at | DATETIME | 수정일 |

### members 테이블
| 컬럼명 | 타입 | 설명 |
|--------|------|------|
| id | INTEGER PRIMARY KEY | 회원 ID |
| name | TEXT NOT NULL | 회원 이름 |
| email | TEXT UNIQUE NOT NULL | 이메일 |
| phone | TEXT UNIQUE | 전화번호 |
| address | TEXT | 주소 |
| registration_date | DATETIME | 가입일 |
| is_active | INTEGER | 활성 상태 |
| created_at | DATETIME | 등록일 |
| updated_at | DATETIME | 수정일 |

### loans 테이블
| 컬럼명 | 타입 | 설명 |
|--------|------|------|
| id | INTEGER PRIMARY KEY | 대출 ID |
| book_id | INTEGER | 도서 ID (외래키) |
| member_id | INTEGER | 회원 ID (외래키) |
| loan_date | DATETIME | 대출일 |
| due_date | DATETIME | 반납 예정일 |
| return_date | DATETIME | 실제 반납일 |
| is_returned | INTEGER | 반납 상태 |
| renewal_count | INTEGER | 연장 횟수 |
| created_at | DATETIME | 등록일 |

## ⚙️ 설정

### 기본 설정값
- 기본 대출 기간: 14일
- 최대 대출 권수: 5권
- 최대 연장 횟수: 2회
- 데이터베이스 경로: `database/library.db`
- 백업 디렉토리: `backups/`

### 설정 변경
프로그램 내 "시스템 설정" 메뉴에서 변경 가능하거나, `config.ini` 파일을 직접 편집할 수 있습니다.

## 🔧 개발 정보

### 개발 환경
- **언어**: C (C17 표준)
- **데이터베이스**: SQLite 3
- **테스트 프레임워크**: GoogleTest (C++)
- **빌드 시스템**: CMake, GCC
- **버전 관리**: Git

### 코딩 표준
- 함수명: snake_case
- 변수명: snake_case  
- 상수명: UPPER_CASE
- 구조체명: PascalCase
- 포인터 검증 필수
- 메모리 누수 방지
- 에러 처리 필수

### 아키텍처
- **모듈화**: 각 기능별로 독립적인 모듈 구성
- **계층화**: 데이터 액세스 → 비즈니스 로직 → 프레젠테이션
- **의존성 분리**: 헤더 파일을 통한 인터페이스 정의

## 🏗️ 프로젝트 구조

```
copilotWorkshop-CGT-start-main/
├── include/                    # 헤더 파일들
│   ├── constants.h            # 상수 정의
│   ├── types.h               # 데이터 구조체 정의
│   ├── database.h            # 데이터베이스 관련 함수
│   ├── book.h                # 도서 관리 함수
│   ├── member.h              # 회원 관리 함수
│   ├── loan.h                # 대출 관리 함수
│   ├── utils.h               # 유틸리티 함수
│   └── main.h                # 메인 애플리케이션 함수
├── src/                       # 소스 파일들
│   ├── database.c            # 데이터베이스 구현
│   ├── book.c                # 도서 관리 구현
│   ├── member.c              # 회원 관리 구현
│   ├── loan.c                # 대출 관리 구현
│   ├── utils.c               # 유틸리티 구현
│   ├── main.c                # 메인 애플리케이션
│   └── external/             # 외부 라이브러리
│       ├── sqlite/           # SQLite 데이터베이스
│       └── googletest/       # GoogleTest 테스트 프레임워크
├── tests/                     # 테스트 파일들
│   ├── unit/                 # 단위 테스트
│   ├── integration/          # 통합 테스트
│   ├── simple_test.c         # 기본 기능 테스트
│   └── run_tests.bat         # 테스트 실행 스크립트
├── database/                  # 데이터베이스 파일 저장 경로
├── backups/                   # 백업 파일 저장 경로
├── CMakeLists.txt            # CMake 빌드 설정
└── README.md                 # 프로젝트 문서
```

## 🤝 기여하기

1. 프로젝트 포크
2. 기능 브랜치 생성 (`git checkout -b feature/AmazingFeature`)
3. 변경사항 커밋 (`git commit -m 'Add some AmazingFeature'`)
4. 브랜치에 푸시 (`git push origin feature/AmazingFeature`)
5. Pull Request 생성

## 📝 라이선스

이 프로젝트는 교육 목적으로 작성되었습니다.

## 🙏 감사의 말

- **SQLite**: 임베디드 데이터베이스 제공
- **GoogleTest**: 테스트 프레임워크 제공
- **MinGW-w64**: Windows용 GCC 컴파일러 제공

## 📞 문의

프로젝트에 대한 질문이나 제안사항이 있으시면 이슈를 생성해 주세요.

---

## 📈 프로젝트 히스토리

| Stage | 설명 | 상태 |
|-------|------|------|
| 1 | 프로젝트 기본 구조 설정 | ✅ 완료 |
| 2 | 데이터베이스 스키마 설계 및 구현 | ✅ 완료 |
| 3 | 도서 관리 모듈 구현 | ✅ 완료 |
| 4 | 회원 관리 모듈 구현 | ✅ 완료 |
| 5 | 대출 관리 모듈 구현 | ✅ 완료 |
| 6 | 유틸리티 모듈 구현 | ✅ 완료 |
| 7 | 메인 애플리케이션 구현 | ✅ 완료 |
| 8 | 테스트 시스템 구현 | ✅ 완료 |

**총 개발 기간**: 8단계 완료  
**총 커밋 수**: 11개  
**코드 라인 수**: 약 5,000줄  
**테스트 커버리지**: 모든 주요 모듈 포함

---

🎉 **프로젝트 완성!** 완전한 기능을 갖춘 도서관 관리 시스템이 성공적으로 구현되었습니다!
