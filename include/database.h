#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include "types.h"
#include "constants.h"

/**
 * @brief 데이터베이스 연결을 초기화합니다.
 * 
 * @param db_path 데이터베이스 파일 경로
 * @return sqlite3* 데이터베이스 연결 포인터, 실패 시 NULL
 */
sqlite3* database_init(const char *db_path);

/**
 * @brief 데이터베이스 연결을 종료합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 */
void database_close(sqlite3 *db);

/**
 * @brief 데이터베이스 테이블들을 생성합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE
 */
int database_create_tables(sqlite3 *db);

/**
 * @brief 트랜잭션을 시작합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE
 */
int database_begin_transaction(sqlite3 *db);

/**
 * @brief 트랜잭션을 커밋합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE
 */
int database_commit_transaction(sqlite3 *db);

/**
 * @brief 트랜잭션을 롤백합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE
 */
int database_rollback_transaction(sqlite3 *db);

/**
 * @brief SQL 쿼리를 실행합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param sql SQL 쿼리 문자열
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE
 */
int database_execute_query(sqlite3 *db, const char *sql);

/**
 * @brief 준비된 문을 생성합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param sql SQL 쿼리 문자열
 * @param stmt 준비된 문 포인터의 포인터
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE
 */
int database_prepare_statement(sqlite3 *db, const char *sql, sqlite3_stmt **stmt);

/**
 * @brief 데이터베이스 백업을 생성합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param backup_path 백업 파일 경로
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE
 */
int database_backup(sqlite3 *db, const char *backup_path);

/**
 * @brief 데이터베이스를 복원합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @param backup_path 백업 파일 경로
 * @return int 성공 시 SUCCESS, 실패 시 FAILURE
 */
int database_restore(sqlite3 *db, const char *backup_path);

/**
 * @brief 마지막 삽입된 행의 ID를 반환합니다.
 * 
 * @param db 데이터베이스 연결 포인터
 * @return int 마지막 삽입된 행의 ID
 */
int database_get_last_insert_id(sqlite3 *db);

#endif // DATABASE_H
