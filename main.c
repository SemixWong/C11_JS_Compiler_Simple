#include "parser.h"
#include "lexer.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 读取文件内容 */
char* read_file(const char *filename, size_t *length) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return NULL;
    }
    
    /* 获取文件大小 */
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size < 0) {
        fprintf(stderr, "Error: Cannot determine file size\n");
        fclose(file);
        return NULL;
    }
    
    /* 分配内存 */
    char *content = (char*)malloc(file_size + 1);
    if (!content) {
        fprintf(stderr, "Error: Out of memory\n");
        fclose(file);
        return NULL;
    }
    
    /* 读取内容 */
    size_t read_size = fread(content, 1, file_size, file);
    content[read_size] = '\0';
    
    fclose(file);
    
    if (length) {
        *length = read_size;
    }
    
    return content;
}

/* 解析JavaScript文件 */
bool parse_javascript_file(const char *filename) {
    size_t length;
    char *source = read_file(filename, &length);
    
    if (!source) {
        return false;
    }
    
    /* 创建错误信息 */
    ErrorInfo error = {0};
    error.code = ERROR_NONE;
    
    /* 创建词法分析器 */
    Lexer *lexer = lexer_create(source, length, &error);
    if (!lexer) {
        fprintf(stderr, "Error: Cannot create lexer\n");
        free(source);
        return false;
    }
    
    /* 创建语法分析器 */
    Parser *parser = parser_create(lexer, &error);
    if (!parser) {
        fprintf(stderr, "Error: Cannot create parser\n");
        lexer_destroy(lexer);
        free(source);
        return false;
    }
    
    /* 执行解析 */
    bool success = parser_parse(parser);
    
    /* 输出结果 */
    if (success && error.code == ERROR_NONE) {
        printf("✓ Syntax is valid\n");
        printf("File: %s\n", filename);
        printf("Status: PASSED\n");
    } else {
        printf("✗ Syntax error detected\n");
        printf("File: %s\n", filename);
        printf("Status: FAILED\n");
        print_error(&error);
    }
    
    /* 清理资源 */
    parser_destroy(parser);
    lexer_destroy(lexer);
    free(source);
    
    return success;
}

/* 解析字符串 */
bool parse_javascript_string(const char *source) {
    if (!source) return false;
    
    size_t length = strlen(source);
    
    /* 创建错误信息 */
    ErrorInfo error = {0};
    error.code = ERROR_NONE;
    
    /* 创建词法分析器 */
    Lexer *lexer = lexer_create(source, length, &error);
    if (!lexer) {
        fprintf(stderr, "Error: Cannot create lexer\n");
        return false;
    }
    
    /* 创建语法分析器 */
    Parser *parser = parser_create(lexer, &error);
    if (!parser) {
        fprintf(stderr, "Error: Cannot create parser\n");
        lexer_destroy(lexer);
        return false;
    }
    
    /* 执行解析 */
    bool success = parser_parse(parser);
    
    /* 输出结果 */
    if (success && error.code == ERROR_NONE) {
        printf("✓ Syntax is valid\n");
    } else {
        printf("✗ Syntax error detected\n");
        print_error(&error);
    }
    
    /* 清理资源 */
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return success;
}

/* 打印使用说明 */
void print_usage(const char *program_name) {
    printf("JavaScript Syntax Parser (Hand-written in C)\n");
    printf("============================================\n\n");
    printf("Usage:\n");
    printf("  %s <javascript-file>\n", program_name);
    printf("  %s -s \"<javascript-code>\"\n\n", program_name);
    printf("Options:\n");
    printf("  -s    Parse JavaScript code from string\n");
    printf("  -h    Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s script.js\n", program_name);
    printf("  %s -s \"let x = 10; console.log(x);\"\n", program_name);
    printf("\nFeatures:\n");
    printf("  - Full Unicode support\n");
    printf("  - Automatic Semicolon Insertion (ASI) according to ECMA262\n");
    printf("  - Detailed error reporting with line/column information\n");
    printf("  - No dependencies on lexer/parser generator tools\n");
}

/* 主函数 */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    /* 处理命令行参数 */
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    } else if (strcmp(argv[1], "-s") == 0) {
        /* 解析字符串 */
        if (argc < 3) {
            fprintf(stderr, "Error: Missing JavaScript code string\n");
            return 1;
        }
        
        bool success = parse_javascript_string(argv[2]);
        return success ? 0 : 1;
    } else {
        /* 解析文件 */
        bool success = parse_javascript_file(argv[1]);
        return success ? 0 : 1;
    }
    
    return 0;
}
