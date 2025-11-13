#ifndef LEXER_H
#define LEXER_H

#include "common.h"

/* Token类型枚举 */
typedef enum {
    /* 特殊标记 */
    TOKEN_EOF,              /* 文件结束 */
    TOKEN_ERROR,            /* 词法错误 */
    
    /* 字面量 */
    TOKEN_IDENTIFIER,       /* 标识符 */
    TOKEN_NUMBER,           /* 数字字面量 */
    TOKEN_STRING,           /* 字符串字面量 */
    TOKEN_TEMPLATE,         /* 模板字符串 */
    TOKEN_REGEX,            /* 正则表达式 */
    TOKEN_TRUE,             /* true */
    TOKEN_FALSE,            /* false */
    TOKEN_NULL,             /* null */
    TOKEN_UNDEFINED,        /* undefined */
    
    /* 关键字 */
    TOKEN_BREAK,
    TOKEN_CASE,
    TOKEN_CATCH,
    TOKEN_CLASS,
    TOKEN_CONST,
    TOKEN_CONTINUE,
    TOKEN_DEBUGGER,
    TOKEN_DEFAULT,
    TOKEN_DELETE,
    TOKEN_DO,
    TOKEN_ELSE,
    TOKEN_EXPORT,
    TOKEN_EXTENDS,
    TOKEN_FINALLY,
    TOKEN_FOR,
    TOKEN_FUNCTION,
    TOKEN_IF,
    TOKEN_IMPORT,
    TOKEN_IN,
    TOKEN_INSTANCEOF,
    TOKEN_LET,
    TOKEN_NEW,
    TOKEN_RETURN,
    TOKEN_SUPER,
    TOKEN_SWITCH,
    TOKEN_THIS,
    TOKEN_THROW,
    TOKEN_TRY,
    TOKEN_TYPEOF,
    TOKEN_VAR,
    TOKEN_VOID,
    TOKEN_WHILE,
    TOKEN_WITH,
    TOKEN_YIELD,
    TOKEN_ASYNC,
    TOKEN_AWAIT,
    TOKEN_OF,
    TOKEN_STATIC,
    TOKEN_GET,
    TOKEN_SET,
    
    /* 运算符和分隔符 */
    TOKEN_LPAREN,           /* ( */
    TOKEN_RPAREN,           /* ) */
    TOKEN_LBRACE,           /* { */
    TOKEN_RBRACE,           /* } */
    TOKEN_LBRACKET,         /* [ */
    TOKEN_RBRACKET,         /* ] */
    TOKEN_SEMICOLON,        /* ; */
    TOKEN_COMMA,            /* , */
    TOKEN_DOT,              /* . */
    TOKEN_COLON,            /* : */
    TOKEN_QUESTION,         /* ? */
    
    /* 算术运算符 */
    TOKEN_PLUS,             /* + */
    TOKEN_MINUS,            /* - */
    TOKEN_MULTIPLY,         /* * */
    TOKEN_DIVIDE,           /* / */
    TOKEN_MODULO,           /* % */
    TOKEN_EXPONENT,         /* ** */
    TOKEN_INCREMENT,        /* ++ */
    TOKEN_DECREMENT,        /* -- */
    
    /* 赋值运算符 */
    TOKEN_ASSIGN,           /* = */
    TOKEN_PLUS_ASSIGN,      /* += */
    TOKEN_MINUS_ASSIGN,     /* -= */
    TOKEN_MULTIPLY_ASSIGN,  /* *= */
    TOKEN_DIVIDE_ASSIGN,    /* /= */
    TOKEN_MODULO_ASSIGN,    /* %= */
    TOKEN_EXPONENT_ASSIGN,  /* **= */
    TOKEN_LSHIFT_ASSIGN,    /* <<= */
    TOKEN_RSHIFT_ASSIGN,    /* >>= */
    TOKEN_URSHIFT_ASSIGN,   /* >>>= */
    TOKEN_AND_ASSIGN,       /* &= */
    TOKEN_OR_ASSIGN,        /* |= */
    TOKEN_XOR_ASSIGN,       /* ^= */
    TOKEN_AND_AND_ASSIGN,   /* &&= */
    TOKEN_OR_OR_ASSIGN,     /* ||= */
    TOKEN_NULLISH_ASSIGN,   /* ??= */
    
    /* 比较运算符 */
    TOKEN_EQ,               /* == */
    TOKEN_NE,               /* != */
    TOKEN_EQ_STRICT,        /* === */
    TOKEN_NE_STRICT,        /* !== */
    TOKEN_LT,               /* < */
    TOKEN_LE,               /* <= */
    TOKEN_GT,               /* > */
    TOKEN_GE,               /* >= */
    
    /* 逻辑运算符 */
    TOKEN_AND,              /* && */
    TOKEN_OR,               /* || */
    TOKEN_NOT,              /* ! */
    TOKEN_NULLISH,          /* ?? */
    
    /* 位运算符 */
    TOKEN_BITWISE_AND,      /* & */
    TOKEN_BITWISE_OR,       /* | */
    TOKEN_BITWISE_XOR,      /* ^ */
    TOKEN_BITWISE_NOT,      /* ~ */
    TOKEN_LSHIFT,           /* << */
    TOKEN_RSHIFT,           /* >> */
    TOKEN_URSHIFT,          /* >>> */
    
    /* 箭头和展开 */
    TOKEN_ARROW,            /* => */
    TOKEN_SPREAD,           /* ... */
    
    /* 可选链 */
    TOKEN_OPTIONAL_CHAIN,   /* ?. */
    
    /* 自动插入的分号 */
    TOKEN_AUTO_SEMICOLON    /* ASI插入的分号 */
} TokenType;

/* Token结构体 */
typedef struct {
    TokenType type;
    char *value;            /* token的字符串值 */
    size_t length;          /* 值的长度 */
    Position start;         /* 起始位置 */
    Position end;           /* 结束位置 */
    bool preceded_by_newline; /* 是否前面有换行（用于ASI判断） */
} Token;

/* 词法分析器状态 */
typedef struct {
    const char *source;     /* 源代码 */
    size_t source_length;   /* 源代码长度 */
    size_t current;         /* 当前读取位置 */
    Position position;      /* 当前位置信息 */
    ErrorInfo *error;       /* 错误信息 */
    bool last_was_newline;  /* 上一个字符是否为换行 */
    Token *prev_token;      /* 上一个token（用于上下文判断） */
} Lexer;

/* 词法分析器函数声明 */
Lexer* lexer_create(const char *source, size_t length, ErrorInfo *error);
void lexer_destroy(Lexer *lexer);
Token* lexer_next_token(Lexer *lexer);
void token_destroy(Token *token);
const char* token_type_to_string(TokenType type);

/* 辅助函数 */
bool is_keyword(const char *str, size_t len, TokenType *type);
bool can_precede_regex(TokenType type);

#endif /* LEXER_H */
