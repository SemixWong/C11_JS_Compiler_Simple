#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

/* 位置信息结构体 */
typedef struct {
    int line;      /* 行号（从1开始） */
    int column;    /* 列号（从1开始） */
    int offset;    /* 文件偏移量 */
} Position;

/* 错误码定义 */
typedef enum {
    ERROR_NONE = 0,
    ERROR_LEXER_INVALID_CHAR,
    ERROR_LEXER_UNTERMINATED_STRING,
    ERROR_LEXER_UNTERMINATED_REGEX,
    ERROR_LEXER_INVALID_NUMBER,
    ERROR_LEXER_INVALID_UNICODE_ESCAPE,
    ERROR_PARSER_UNEXPECTED_TOKEN,
    ERROR_PARSER_EXPECTED_TOKEN,
    ERROR_PARSER_INVALID_ASSIGNMENT,
    ERROR_PARSER_MISSING_SEMICOLON,
    ERROR_PARSER_UNEXPECTED_EOF,
    ERROR_FILE_READ,
    ERROR_OUT_OF_MEMORY
} ErrorCode;

/* 错误信息结构体 */
typedef struct {
    ErrorCode code;
    Position position;
    char message[256];
} ErrorInfo;

/* Unicode字符相关工具函数声明 */
bool is_unicode_id_start(uint32_t ch);
bool is_unicode_id_continue(uint32_t ch);
bool is_line_terminator(uint32_t ch);
bool is_whitespace(uint32_t ch);

/* 错误处理函数 */
void set_error(ErrorInfo *error, ErrorCode code, Position pos, const char *message);
void print_error(const ErrorInfo *error);

#endif /* COMMON_H */
