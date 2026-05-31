#include "yflc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

static char* read_line(void) {
    char buf[4096];
    if (!fgets(buf, sizeof(buf), stdin)) return NULL;
    buf[strcspn(buf, "\r\n")] = '\0';
    return _strdup(buf);
}

int main(void) {
    char choice;

#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    for (;;) {
        printf("\n===== 与佛论禅 =====\n");
        printf("1. 加密\n");
        printf("2. 解密\n");
        printf("0. 退出\n");
        printf("请选择: ");

        choice = (char)getchar();
        while (getchar() != '\n');

        if (choice == '1') {
            char* input;
            char* result;
            printf("请输入明文: ");
            input = read_line();
            if (!input) {
                printf("读取失败\n");
                continue;
            }
            result = yflc_encrypt(input);
            free(input);
            if (result) {
                printf("密文: %s\n", result);
                free(result);
            } else {
                printf("加密失败\n");
            }

        } else if (choice == '2') {
            char* input;
            char* result;
            printf("请输入密文: ");
            input = read_line();
            if (!input) {
                printf("读取失败\n");
                continue;
            }
            result = yflc_decrypt(input);
            free(input);
            if (result) {
                printf("解密: %s\n", result);
                free(result);
            } else {
                printf("解密失败，请检查密文是否正确\n");
            }

        } else if (choice == '0') {
            break;
        } else {
            printf("无效选项，请重新输入\n");
        }
    }

    return 0;
}
