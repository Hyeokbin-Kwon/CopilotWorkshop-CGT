#include <stdio.h>
#include <locale.h>
#ifdef _WIN32
#include <windows.h>
#endif

int main() {
    // Windows 콘솔 UTF-8 설정
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    
    // 로케일 설정
    setlocale(LC_ALL, "ko_KR.UTF-8");
    
    printf("=================================================\n");
    printf("        도서관 관리 시스템\n");
    printf("        Library Management System\n");
    printf("=================================================\n");
    printf("한글이 제대로 표시됩니까?\n");
    
    return 0;
}
