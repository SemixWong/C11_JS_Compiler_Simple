#include "lexer.h"

/* 关键字映射表 */
typedef struct {
    const char *keyword;
    TokenType type;
} KeywordEntry;

static const KeywordEntry keywords[] = {
    {"break", TOKEN_BREAK},
    {"case", TOKEN_CASE},
    {"catch", TOKEN_CATCH},
    {"class", TOKEN_CLASS},
    {"const", TOKEN_CONST},
    {"continue", TOKEN_CONTINUE},
    {"debugger", TOKEN_DEBUGGER},
    {"default", TOKEN_DEFAULT},
    {"delete", TOKEN_DELETE},
    {"do", TOKEN_DO},
    {"else", TOKEN_ELSE},
    {"export", TOKEN_EXPORT},
    {"extends", TOKEN_EXTENDS},
    {"finally", TOKEN_FINALLY},
    {"for", TOKEN_FOR},
    {"function", TOKEN_FUNCTION},
    {"if", TOKEN_IF},
    {"import", TOKEN_IMPORT},
    {"in", TOKEN_IN},
    {"instanceof", TOKEN_INSTANCEOF},
    {"let", TOKEN_LET},
    {"new", TOKEN_NEW},
    {"return", TOKEN_RETURN},
    {"super", TOKEN_SUPER},
    {"switch", TOKEN_SWITCH},
    {"this", TOKEN_THIS},
    {"throw", TOKEN_THROW},
    {"try", TOKEN_TRY},
    {"typeof", TOKEN_TYPEOF},
    {"var", TOKEN_VAR},
    {"void", TOKEN_VOID},
    {"while", TOKEN_WHILE},
    {"with", TOKEN_WITH},
    {"yield", TOKEN_YIELD},
    {"async", TOKEN_ASYNC},
    {"await", TOKEN_AWAIT},
    {"of", TOKEN_OF},
    {"static", TOKEN_STATIC},
    {"get", TOKEN_GET},
    {"set", TOKEN_SET},
    {"true", TOKEN_TRUE},
    {"false", TOKEN_FALSE},
    {"null", TOKEN_NULL},
    {"undefined", TOKEN_UNDEFINED},
    {NULL, TOKEN_EOF}
};

/* 判断是否为关键字 */
bool is_keyword(const char *str, size_t len, TokenType *type) {
    for (int i = 0; keywords[i].keyword != NULL; i++) {
        if (strlen(keywords[i].keyword) == len && 
            strncmp(keywords[i].keyword, str, len) == 0) {
            if (type) *type = keywords[i].type;
            return true;
        }
    }
    return false;
}

/* 判断token类型是否可以在正则表达式之前出现 */
bool can_precede_regex(TokenType type) {
    switch (type) {
        case TOKEN_ASSIGN:
        case TOKEN_LPAREN:
        case TOKEN_LBRACKET:
        case TOKEN_NOT:
        case TOKEN_BITWISE_NOT:
        case TOKEN_AND:
        case TOKEN_OR:
        case TOKEN_QUESTION:
        case TOKEN_COMMA:
        case TOKEN_SEMICOLON:
        case TOKEN_COLON:
        case TOKEN_RETURN:
        case TOKEN_THROW:
        case TOKEN_LBRACE:
        case TOKEN_EQ:
        case TOKEN_NE:
        case TOKEN_EQ_STRICT:
        case TOKEN_NE_STRICT:
        case TOKEN_LT:
        case TOKEN_LE:
        case TOKEN_GT:
        case TOKEN_GE:
            return true;
        default:
            return false;
    }
}

/* 创建Token */
static Token* token_create(TokenType type, const char *value, size_t length,
                          Position start, Position end, bool preceded_by_newline) {
    Token *token = (Token*)malloc(sizeof(Token));
    if (!token) return NULL;
    
    token->type = type;
    token->length = length;
    token->start = start;
    token->end = end;
    token->preceded_by_newline = preceded_by_newline;
    
    if (value && length > 0) {
        token->value = (char*)malloc(length + 1);
        if (token->value) {
            memcpy(token->value, value, length);
            token->value[length] = '\0';
        }
    } else {
        token->value = NULL;
    }
    
    return token;
}

/* 销毁Token */
void token_destroy(Token *token) {
    if (token) {
        if (token->value) free(token->value);
        free(token);
    }
}

/* 创建词法分析器 */
Lexer* lexer_create(const char *source, size_t length, ErrorInfo *error) {
    Lexer *lexer = (Lexer*)malloc(sizeof(Lexer));
    if (!lexer) return NULL;
    
    lexer->source = source;
    lexer->source_length = length;
    lexer->current = 0;
    lexer->position.line = 1;
    lexer->position.column = 1;
    lexer->position.offset = 0;
    lexer->error = error;
    lexer->last_was_newline = false;
    lexer->prev_token = NULL;
    
    return lexer;
}

/* 销毁词法分析器 */
void lexer_destroy(Lexer *lexer) {
    if (lexer) {
        if (lexer->prev_token) {
            token_destroy(lexer->prev_token);
        }
        free(lexer);
    }
}

/* 查看当前字符但不前进 */
static char peek(Lexer *lexer, size_t offset) {
    size_t pos = lexer->current + offset;
    if (pos >= lexer->source_length) return '\0';
    return lexer->source[pos];
}

/* 前进一个字符 */
static char advance(Lexer *lexer) {
    if (lexer->current >= lexer->source_length) return '\0';
    
    char ch = lexer->source[lexer->current++];
    lexer->position.offset++;
    
    if (ch == '\n') {
        lexer->position.line++;
        lexer->position.column = 1;
        lexer->last_was_newline = true;
    } else if (ch == '\r') {
        /* 处理\r\n */
        if (peek(lexer, 0) == '\n') {
            lexer->current++;
            lexer->position.offset++;
        }
        lexer->position.line++;
        lexer->position.column = 1;
        lexer->last_was_newline = true;
    } else {
        lexer->position.column++;
    }
    
    return ch;
}

/* 跳过空白字符 */
static void skip_whitespace(Lexer *lexer) {
    while (lexer->current < lexer->source_length) {
        char ch = peek(lexer, 0);
        
        if (is_whitespace(ch)) {
            advance(lexer);
        } else if (ch == '\n' || ch == '\r') {
            advance(lexer);
        } else {
            break;
        }
    }
}

/* 跳过单行注释 */
static void skip_line_comment(Lexer *lexer) {
    /* 跳过 // */
    advance(lexer);
    advance(lexer);
    
    while (lexer->current < lexer->source_length) {
        char ch = peek(lexer, 0);
        if (is_line_terminator(ch)) {
            break;
        }
        advance(lexer);
    }
}

/* 跳过块注释 */
static bool skip_block_comment(Lexer *lexer) {
    /* 跳过开始的斜杠和星号 */
    advance(lexer);
    advance(lexer);
    
    while (lexer->current < lexer->source_length) {
        char ch = peek(lexer, 0);
        if (ch == '*' && peek(lexer, 1) == '/') {
            advance(lexer);
            advance(lexer);
            return true;
        }
        advance(lexer);
    }
    
    /* 未闭合的注释 */
    set_error(lexer->error, ERROR_LEXER_UNTERMINATED_STRING,
              lexer->position, "Unterminated block comment");
    return false;
}

/* 解析Unicode转义序列 \uXXXX */
static bool parse_unicode_escape(Lexer *lexer, uint32_t *result) {
    /* 跳过 \u */
    advance(lexer);
    advance(lexer);
    
    uint32_t value = 0;
    for (int i = 0; i < 4; i++) {
        char ch = peek(lexer, 0);
        if (!isxdigit(ch)) {
            set_error(lexer->error, ERROR_LEXER_INVALID_UNICODE_ESCAPE,
                     lexer->position, "Invalid Unicode escape sequence");
            return false;
        }
        
        value = value * 16 + (isdigit(ch) ? (ch - '0') : 
                             (tolower(ch) - 'a' + 10));
        advance(lexer);
    }
    
    *result = value;
    return true;
}

/* 读取标识符 */
static Token* read_identifier(Lexer *lexer, Position start) {
    size_t start_pos = lexer->current - 1;
    
    while (lexer->current < lexer->source_length) {
        char ch = peek(lexer, 0);
        
        if (ch == '\\' && peek(lexer, 1) == 'u') {
            uint32_t unicode_char;
            if (!parse_unicode_escape(lexer, &unicode_char)) {
                return NULL;
            }
            continue;
        }
        
        if (is_unicode_id_continue((uint8_t)ch)) {
            advance(lexer);
        } else {
            break;
        }
    }
    
    size_t length = lexer->current - start_pos;
    const char *value = lexer->source + start_pos;
    
    /* 检查是否为关键字 */
    TokenType type = TOKEN_IDENTIFIER;
    is_keyword(value, length, &type);
    
    return token_create(type, value, length, start, lexer->position, 
                       lexer->last_was_newline);
}

/* 读取数字 */
static Token* read_number(Lexer *lexer, Position start) {
    size_t start_pos = lexer->current - 1;
    
    /* 处理十六进制、八进制、二进制 */
    char first = lexer->source[start_pos];
    if (first == '0' && lexer->current < lexer->source_length) {
        char next = peek(lexer, 0);
        if (next == 'x' || next == 'X') {
            /* 十六进制 */
            advance(lexer);
            while (lexer->current < lexer->source_length && 
                   isxdigit(peek(lexer, 0))) {
                advance(lexer);
            }
        } else if (next == 'b' || next == 'B') {
            /* 二进制 */
            advance(lexer);
            while (lexer->current < lexer->source_length) {
                char ch = peek(lexer, 0);
                if (ch == '0' || ch == '1') advance(lexer);
                else break;
            }
        } else if (next == 'o' || next == 'O') {
            /* 八进制 */
            advance(lexer);
            while (lexer->current < lexer->source_length) {
                char ch = peek(lexer, 0);
                if (ch >= '0' && ch <= '7') advance(lexer);
                else break;
            }
        }
    }
    
    /* 读取整数部分 */
    while (lexer->current < lexer->source_length && 
           isdigit(peek(lexer, 0))) {
        advance(lexer);
    }
    
    /* 读取小数部分 */
    if (peek(lexer, 0) == '.' && isdigit(peek(lexer, 1))) {
        advance(lexer); /* . */
        while (lexer->current < lexer->source_length && 
               isdigit(peek(lexer, 0))) {
            advance(lexer);
        }
    }
    
    /* 读取指数部分 */
    if (peek(lexer, 0) == 'e' || peek(lexer, 0) == 'E') {
        advance(lexer);
        if (peek(lexer, 0) == '+' || peek(lexer, 0) == '-') {
            advance(lexer);
        }
        while (lexer->current < lexer->source_length && 
               isdigit(peek(lexer, 0))) {
            advance(lexer);
        }
    }
    
    size_t length = lexer->current - start_pos;
    return token_create(TOKEN_NUMBER, lexer->source + start_pos, length,
                       start, lexer->position, lexer->last_was_newline);
}

/* 读取字符串 */
static Token* read_string(Lexer *lexer, Position start, char quote) {
    size_t start_pos = lexer->current - 1;
    
    while (lexer->current < lexer->source_length) {
        char ch = peek(lexer, 0);
        
        if (ch == quote) {
            advance(lexer);
            break;
        } else if (ch == '\\') {
            advance(lexer);
            if (lexer->current < lexer->source_length) {
                advance(lexer);
            }
        } else if (is_line_terminator(ch)) {
            set_error(lexer->error, ERROR_LEXER_UNTERMINATED_STRING,
                     lexer->position, "Unterminated string literal");
            return NULL;
        } else {
            advance(lexer);
        }
    }
    
    size_t length = lexer->current - start_pos;
    return token_create(TOKEN_STRING, lexer->source + start_pos, length,
                       start, lexer->position, lexer->last_was_newline);
}

/* 读取模板字符串 */
static Token* read_template(Lexer *lexer, Position start) {
    size_t start_pos = lexer->current - 1;
    
    while (lexer->current < lexer->source_length) {
        char ch = peek(lexer, 0);
        
        if (ch == '`') {
            advance(lexer);
            break;
        } else if (ch == '\\') {
            advance(lexer);
            if (lexer->current < lexer->source_length) {
                advance(lexer);
            }
        } else if (ch == '$' && peek(lexer, 1) == '{') {
            /* 处理模板表达式 ${...} - 简化处理 */
            advance(lexer);
            advance(lexer);
        } else {
            advance(lexer);
        }
    }
    
    size_t length = lexer->current - start_pos;
    return token_create(TOKEN_TEMPLATE, lexer->source + start_pos, length,
                       start, lexer->position, lexer->last_was_newline);
}

/* 读取正则表达式 */
static Token* read_regex(Lexer *lexer, Position start) {
    size_t start_pos = lexer->current - 1;
    
    /* 跳过开始的 / */
    while (lexer->current < lexer->source_length) {
        char ch = peek(lexer, 0);
        
        if (ch == '/') {
            advance(lexer);
            /* 读取标志 */
            while (lexer->current < lexer->source_length) {
                char flag = peek(lexer, 0);
                if (isalpha(flag)) {
                    advance(lexer);
                } else {
                    break;
                }
            }
            break;
        } else if (ch == '\\') {
            advance(lexer);
            if (lexer->current < lexer->source_length) {
                advance(lexer);
            }
        } else if (is_line_terminator(ch)) {
            set_error(lexer->error, ERROR_LEXER_UNTERMINATED_REGEX,
                     lexer->position, "Unterminated regular expression");
            return NULL;
        } else if (ch == '[') {
            /* 字符类 */
            advance(lexer);
            while (lexer->current < lexer->source_length) {
                char cls_ch = peek(lexer, 0);
                if (cls_ch == ']') {
                    advance(lexer);
                    break;
                } else if (cls_ch == '\\') {
                    advance(lexer);
                    if (lexer->current < lexer->source_length) {
                        advance(lexer);
                    }
                } else {
                    advance(lexer);
                }
            }
        } else {
            advance(lexer);
        }
    }
    
    size_t length = lexer->current - start_pos;
    return token_create(TOKEN_REGEX, lexer->source + start_pos, length,
                       start, lexer->position, lexer->last_was_newline);
}

/* 获取下一个token */
Token* lexer_next_token(Lexer *lexer) {
    /* 跳过空白和注释，但保留换行信息 */
    while (lexer->current < lexer->source_length) {
        skip_whitespace(lexer);
        
        if (lexer->current >= lexer->source_length) break;
        
        char ch = peek(lexer, 0);
        char next = peek(lexer, 1);
        
        if (ch == '/' && next == '/') {
            skip_line_comment(lexer);
        } else if (ch == '/' && next == '*') {
            if (!skip_block_comment(lexer)) {
                return NULL;
            }
        } else {
            break;
        }
    }
    
    /* 现在检查是否有换行 */
    bool had_newline = lexer->last_was_newline;
    lexer->last_was_newline = false;
    
    if (lexer->current >= lexer->source_length) {
        return token_create(TOKEN_EOF, NULL, 0, lexer->position, 
                          lexer->position, had_newline);
    }
    
    Position start = lexer->position;
    char ch = advance(lexer);
    
    /* 标识符和关键字 */
    if (is_unicode_id_start((uint8_t)ch) || ch == '$') {
        Token *token = read_identifier(lexer, start);
        if (token) {
            token->preceded_by_newline = had_newline;
            /* 保存当前token以供上下文判断 */
            if (lexer->prev_token) token_destroy(lexer->prev_token);
            lexer->prev_token = token_create(token->type, token->value, 
                                            token->length, token->start, 
                                            token->end, token->preceded_by_newline);
        }
        return token;
    }
    
    /* 数字 */
    if (isdigit(ch)) {
        Token *token = read_number(lexer, start);
        if (token) {
            token->preceded_by_newline = had_newline;
            /* 保存当前token以供上下文判断 */
            if (lexer->prev_token) token_destroy(lexer->prev_token);
            lexer->prev_token = token_create(token->type, token->value, 
                                            token->length, token->start, 
                                            token->end, token->preceded_by_newline);
        }
        return token;
    }
    
    /* 字符串 */
    if (ch == '"' || ch == '\'') {
        Token *token = read_string(lexer, start, ch);
        if (token) {
            token->preceded_by_newline = had_newline;
            /* 保存当前token以供上下文判断 */
            if (lexer->prev_token) token_destroy(lexer->prev_token);
            lexer->prev_token = token_create(token->type, token->value, 
                                            token->length, token->start, 
                                            token->end, token->preceded_by_newline);
        }
        return token;
    }
    
    /* 模板字符串 */
    if (ch == '`') {
        Token *token = read_template(lexer, start);
        if (token) {
            token->preceded_by_newline = had_newline;
            /* 保存当前token以供上下文判断 */
            if (lexer->prev_token) token_destroy(lexer->prev_token);
            lexer->prev_token = token_create(token->type, token->value, 
                                            token->length, token->start, 
                                            token->end, token->preceded_by_newline);
        }
        return token;
    }
    
    /* 运算符和分隔符 */
    char next = peek(lexer, 0);
    char next2 = peek(lexer, 1);
    
    /* 三字符运算符 */
    if (ch == '>' && next == '>' && next2 == '>') {
        advance(lexer); advance(lexer);
        if (peek(lexer, 0) == '=') {
            advance(lexer);
            return token_create(TOKEN_URSHIFT_ASSIGN, ">>>=", 4, start, 
                              lexer->position, had_newline);
        }
        return token_create(TOKEN_URSHIFT, ">>>", 3, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '=' && next == '=' && next2 == '=') {
        advance(lexer); advance(lexer);
        return token_create(TOKEN_EQ_STRICT, "===", 3, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '!' && next == '=' && next2 == '=') {
        advance(lexer); advance(lexer);
        return token_create(TOKEN_NE_STRICT, "!==", 3, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '.' && next == '.' && next2 == '.') {
        advance(lexer); advance(lexer);
        return token_create(TOKEN_SPREAD, "...", 3, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '*' && next == '*' && next2 == '=') {
        advance(lexer); advance(lexer);
        return token_create(TOKEN_EXPONENT_ASSIGN, "**=", 3, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '&' && next == '&' && next2 == '=') {
        advance(lexer); advance(lexer);
        return token_create(TOKEN_AND_AND_ASSIGN, "&&=", 3, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '|' && next == '|' && next2 == '=') {
        advance(lexer); advance(lexer);
        return token_create(TOKEN_OR_OR_ASSIGN, "||=", 3, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '?' && next == '?' && next2 == '=') {
        advance(lexer); advance(lexer);
        return token_create(TOKEN_NULLISH_ASSIGN, "?" "?" "=", 3, start, 
                          lexer->position, had_newline);
    }
    
    /* 双字符运算符 */
    if (ch == '=' && next == '=') {
        advance(lexer);
        return token_create(TOKEN_EQ, "==", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '!' && next == '=') {
        advance(lexer);
        return token_create(TOKEN_NE, "!=", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '<' && next == '=') {
        advance(lexer);
        return token_create(TOKEN_LE, "<=", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '>' && next == '=') {
        advance(lexer);
        return token_create(TOKEN_GE, ">=", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '<' && next == '<') {
        advance(lexer);
        if (peek(lexer, 0) == '=') {
            advance(lexer);
            return token_create(TOKEN_LSHIFT_ASSIGN, "<<=", 3, start, 
                              lexer->position, had_newline);
        }
        return token_create(TOKEN_LSHIFT, "<<", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '>' && next == '>') {
        advance(lexer);
        if (peek(lexer, 0) == '=') {
            advance(lexer);
            return token_create(TOKEN_RSHIFT_ASSIGN, ">>=", 3, start, 
                              lexer->position, had_newline);
        }
        return token_create(TOKEN_RSHIFT, ">>", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '&' && next == '&') {
        advance(lexer);
        return token_create(TOKEN_AND, "&&", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '|' && next == '|') {
        advance(lexer);
        return token_create(TOKEN_OR, "||", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '?' && next == '?') {
        advance(lexer);
        return token_create(TOKEN_NULLISH, "??", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '+' && next == '+') {
        advance(lexer);
        return token_create(TOKEN_INCREMENT, "++", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '-' && next == '-') {
        advance(lexer);
        return token_create(TOKEN_DECREMENT, "--", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '*' && next == '*') {
        advance(lexer);
        return token_create(TOKEN_EXPONENT, "**", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '=' && next == '>') {
        advance(lexer);
        return token_create(TOKEN_ARROW, "=>", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '?' && next == '.') {
        advance(lexer);
        return token_create(TOKEN_OPTIONAL_CHAIN, "?.", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '+' && next == '=') {
        advance(lexer);
        return token_create(TOKEN_PLUS_ASSIGN, "+=", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '-' && next == '=') {
        advance(lexer);
        return token_create(TOKEN_MINUS_ASSIGN, "-=", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '*' && next == '=') {
        advance(lexer);
        return token_create(TOKEN_MULTIPLY_ASSIGN, "*=", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '/' && next == '=') {
        advance(lexer);
        return token_create(TOKEN_DIVIDE_ASSIGN, "/=", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '%' && next == '=') {
        advance(lexer);
        return token_create(TOKEN_MODULO_ASSIGN, "%=", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '&' && next == '=') {
        advance(lexer);
        return token_create(TOKEN_AND_ASSIGN, "&=", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '|' && next == '=') {
        advance(lexer);
        return token_create(TOKEN_OR_ASSIGN, "|=", 2, start, 
                          lexer->position, had_newline);
    }
    
    if (ch == '^' && next == '=') {
        advance(lexer);
        return token_create(TOKEN_XOR_ASSIGN, "^=", 2, start, 
                          lexer->position, had_newline);
    }
    
    /* 单字符运算符 */
    TokenType type;
    switch (ch) {
        case '(': type = TOKEN_LPAREN; break;
        case ')': type = TOKEN_RPAREN; break;
        case '{': type = TOKEN_LBRACE; break;
        case '}': type = TOKEN_RBRACE; break;
        case '[': type = TOKEN_LBRACKET; break;
        case ']': type = TOKEN_RBRACKET; break;
        case ';': type = TOKEN_SEMICOLON; break;
        case ',': type = TOKEN_COMMA; break;
        case '.': type = TOKEN_DOT; break;
        case ':': type = TOKEN_COLON; break;
        case '?': type = TOKEN_QUESTION; break;
        case '+': type = TOKEN_PLUS; break;
        case '-': type = TOKEN_MINUS; break;
        case '*': type = TOKEN_MULTIPLY; break;
        case '%': type = TOKEN_MODULO; break;
        case '=': type = TOKEN_ASSIGN; break;
        case '<': type = TOKEN_LT; break;
        case '>': type = TOKEN_GT; break;
        case '!': type = TOKEN_NOT; break;
        case '&': type = TOKEN_BITWISE_AND; break;
        case '|': type = TOKEN_BITWISE_OR; break;
        case '^': type = TOKEN_BITWISE_XOR; break;
        case '~': type = TOKEN_BITWISE_NOT; break;
        case '/':
            /* 判断是否为正则表达式 */
            if (lexer->prev_token && 
                can_precede_regex(lexer->prev_token->type)) {
                Token *token = read_regex(lexer, start);
                if (token) {
                    token->preceded_by_newline = had_newline;
                    /* 保存当前token以供上下文判断 */
                    if (lexer->prev_token) token_destroy(lexer->prev_token);
                    lexer->prev_token = token_create(token->type, token->value, 
                                                    token->length, token->start, 
                                                    token->end, token->preceded_by_newline);
                }
                return token;
            }
            type = TOKEN_DIVIDE;
            break;
        default:
            {
                char msg[128];
                snprintf(msg, sizeof(msg), "Unexpected character: '%c'", ch);
                set_error(lexer->error, ERROR_LEXER_INVALID_CHAR, 
                         start, msg);
                return NULL;
            }
    }
    
    char str[2] = {ch, '\0'};
    Token *token = token_create(type, str, 1, start, lexer->position, had_newline);
    
    /* 保存当前token */
    if (lexer->prev_token) token_destroy(lexer->prev_token);
    lexer->prev_token = token_create(token->type, token->value, token->length,
                                     token->start, token->end, 
                                     token->preceded_by_newline);
    
    return token;
}

/* Token类型转字符串 */
const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        default: return "UNKNOWN";
    }
}
