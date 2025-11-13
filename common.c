#include "common.h"

/* Unicode字符分类函数 - 简化实现，支持基本的ASCII和部分Unicode范围 */
bool is_unicode_id_start(uint32_t ch) {
    /* 支持 a-z, A-Z, $, _, 和部分Unicode字符 */
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '$' || ch == '_') {
        return true;
    }
    /* 支持部分Unicode标识符字符（简化版） */
    if (ch >= 0x80) {
        /* Unicode字母范围（简化判断） */
        return (ch >= 0x00AA && ch <= 0xFFFF);
    }
    return false;
}

bool is_unicode_id_continue(uint32_t ch) {
    /* 标识符继续字符包括 ID_Start + 数字 + 其他特殊字符 */
    if (is_unicode_id_start(ch) || (ch >= '0' && ch <= '9')) {
        return true;
    }
    /* Unicode组合字符等（简化） */
    if (ch == 0x200C || ch == 0x200D) { /* ZWNJ, ZWJ */
        return true;
    }
    return false;
}

bool is_line_terminator(uint32_t ch) {
    /* ECMAScript定义的行终止符 */
    return ch == '\n' || ch == '\r' || ch == 0x2028 || ch == 0x2029;
}

bool is_whitespace(uint32_t ch) {
    /* ECMAScript定义的空白字符 */
    return ch == ' ' || ch == '\t' || ch == '\v' || ch == '\f' || 
           ch == 0x00A0 || ch == 0xFEFF || /* NBSP, BOM */
           (ch >= 0x2000 && ch <= 0x200A); /* Unicode空格 */
}

/* 设置错误信息 */
void set_error(ErrorInfo *error, ErrorCode code, Position pos, const char *message) {
    if (!error) return;
    error->code = code;
    error->position = pos;
    strncpy(error->message, message, sizeof(error->message) - 1);
    error->message[sizeof(error->message) - 1] = '\0';
}

/* 打印错误信息 */
void print_error(const ErrorInfo *error) {
    if (!error || error->code == ERROR_NONE) return;
    
    fprintf(stderr, "Error at line %d, column %d: %s\n",
            error->position.line,
            error->position.column,
            error->message);
}
