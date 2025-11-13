#include "parser.h"

#define MAX_RECURSION_DEPTH 1000

/* 创建语法分析器 */
Parser* parser_create(Lexer *lexer, ErrorInfo *error) {
    Parser *parser = (Parser*)malloc(sizeof(Parser));
    if (!parser) return NULL;
    
    parser->lexer = lexer;
    parser->current_token = NULL;
    parser->prev_token = NULL;
    parser->error = error;
    parser->asi_allowed = true;
    parser->depth = 0;
    
    /* 读取第一个token */
    parser_advance(parser);
    
    return parser;
}

/* 销毁语法分析器 */
void parser_destroy(Parser *parser) {
    if (parser) {
        if (parser->current_token) {
            token_destroy(parser->current_token);
        }
        if (parser->prev_token) {
            token_destroy(parser->prev_token);
        }
        free(parser);
    }
}

/* 前进到下一个token */
bool parser_advance(Parser *parser) {
    if (parser->current_token) {
        if (parser->prev_token) {
            token_destroy(parser->prev_token);
        }
        parser->prev_token = parser->current_token;
    }
    
    parser->current_token = lexer_next_token(parser->lexer);
    
    if (!parser->current_token) {
        return false;
    }
    
    return parser->current_token->type != TOKEN_ERROR;
}

/* 检查当前token类型 */
bool parser_check(Parser *parser, TokenType type) {
    if (!parser->current_token) return false;
    return parser->current_token->type == type;
}

/* 匹配并前进 */
bool parser_match(Parser *parser, TokenType type) {
    if (parser_check(parser, type)) {
        parser_advance(parser);
        return true;
    }
    return false;
}

/* 期望特定token */
bool parser_expect(Parser *parser, TokenType type) {
    if (parser_check(parser, type)) {
        parser_advance(parser);
        return true;
    }
    
    char msg[256];
    snprintf(msg, sizeof(msg), "Expected token type %d, got %d", 
             type, parser->current_token ? (int)parser->current_token->type : -1);
    set_error(parser->error, ERROR_PARSER_EXPECTED_TOKEN,
             parser->current_token ? parser->current_token->start : 
             parser->prev_token->end, msg);
    
    return false;
}

/* 判断是否应该插入分号（ASI规则） */
bool parser_should_insert_semicolon(Parser *parser, TokenType prev_type) {
    if (!parser->current_token) return true;
    
    /* 规则1: 遇到换行且当前token不符合语法 */
    if (parser->current_token->preceded_by_newline) {
        return true;
    }
    
    /* 规则2: 当前token是}或EOF */
    if (parser->current_token->type == TOKEN_RBRACE || 
        parser->current_token->type == TOKEN_EOF) {
        return true;
    }
    
    /* 规则3: 特定语句后换行自动插入分号 */
    switch (prev_type) {
        case TOKEN_RETURN:
        case TOKEN_BREAK:
        case TOKEN_CONTINUE:
        case TOKEN_THROW:
            if (parser->current_token->preceded_by_newline) {
                return true;
            }
            break;
        default:
            break;
    }
    
    return false;
}

/* 检查ASI */
bool parser_check_asi(Parser *parser) {
    if (!parser->current_token) return true;
    
    /* 如果是分号，直接消耗 */
    if (parser->current_token->type == TOKEN_SEMICOLON) {
        return true;
    }
    
    /* 检查是否应该自动插入分号 */
    TokenType prev_type = parser->prev_token ? parser->prev_token->type : TOKEN_EOF;
    return parser_should_insert_semicolon(parser, prev_type);
}

/* 消耗分号（或通过ASI插入） */
bool parser_consume_semicolon(Parser *parser) {
    /* 如果是显式分号，消耗它 */
    if (parser_check(parser, TOKEN_SEMICOLON)) {
        parser_advance(parser);
        return true;
    }
    
    /* 检查ASI条件 */
    if (parser_check_asi(parser)) {
        /* ASI成功，不需要消耗token */
        return true;
    }
    
    /* ASI失败，报错 */
    set_error(parser->error, ERROR_PARSER_MISSING_SEMICOLON,
             parser->current_token->start, "Missing semicolon");
    return false;
}

/* 判断是否为赋值运算符 */
bool is_assignment_operator(TokenType type) {
    switch (type) {
        case TOKEN_ASSIGN:
        case TOKEN_PLUS_ASSIGN:
        case TOKEN_MINUS_ASSIGN:
        case TOKEN_MULTIPLY_ASSIGN:
        case TOKEN_DIVIDE_ASSIGN:
        case TOKEN_MODULO_ASSIGN:
        case TOKEN_EXPONENT_ASSIGN:
        case TOKEN_LSHIFT_ASSIGN:
        case TOKEN_RSHIFT_ASSIGN:
        case TOKEN_URSHIFT_ASSIGN:
        case TOKEN_AND_ASSIGN:
        case TOKEN_OR_ASSIGN:
        case TOKEN_XOR_ASSIGN:
        case TOKEN_AND_AND_ASSIGN:
        case TOKEN_OR_OR_ASSIGN:
        case TOKEN_NULLISH_ASSIGN:
            return true;
        default:
            return false;
    }
}

/* 判断是否为一元运算符 */
bool is_unary_operator(TokenType type) {
    switch (type) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_NOT:
        case TOKEN_BITWISE_NOT:
        case TOKEN_INCREMENT:
        case TOKEN_DECREMENT:
        case TOKEN_TYPEOF:
        case TOKEN_VOID:
        case TOKEN_DELETE:
            return true;
        default:
            return false;
    }
}

/* 判断是否为语句开始 */
bool is_statement_start(TokenType type) {
    switch (type) {
        case TOKEN_VAR:
        case TOKEN_LET:
        case TOKEN_CONST:
        case TOKEN_FUNCTION:
        case TOKEN_CLASS:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_DO:
        case TOKEN_FOR:
        case TOKEN_SWITCH:
        case TOKEN_RETURN:
        case TOKEN_BREAK:
        case TOKEN_CONTINUE:
        case TOKEN_THROW:
        case TOKEN_TRY:
        case TOKEN_LBRACE:
        case TOKEN_SEMICOLON:
            return true;
        default:
            return false;
    }
}

/* 解析程序 */
bool parse_program(Parser *parser) {
    parser->depth = 0;
    return parse_statement_list(parser);
}

/* 解析语句列表 */
bool parse_statement_list(Parser *parser) {
    while (parser->current_token && 
           parser->current_token->type != TOKEN_EOF &&
           parser->current_token->type != TOKEN_RBRACE) {
        if (!parse_statement(parser)) {
            return false;
        }
    }
    return true;
}

/* 解析语句 */
bool parse_statement(Parser *parser) {
    if (++parser->depth > MAX_RECURSION_DEPTH) {
        set_error(parser->error, ERROR_PARSER_UNEXPECTED_TOKEN,
                 parser->current_token->start, "Maximum recursion depth exceeded");
        return false;
    }
    
    if (!parser->current_token) return false;
    
    bool result = false;
    
    switch (parser->current_token->type) {
        case TOKEN_VAR:
        case TOKEN_LET:
        case TOKEN_CONST:
            result = parse_variable_declaration(parser);
            break;
            
        case TOKEN_FUNCTION:
            result = parse_function_declaration(parser);
            break;
            
        case TOKEN_CLASS:
            result = parse_class_declaration(parser);
            break;
            
        case TOKEN_IF:
            result = parse_if_statement(parser);
            break;
            
        case TOKEN_WHILE:
            result = parse_while_statement(parser);
            break;
            
        case TOKEN_DO:
            result = parse_do_while_statement(parser);
            break;
            
        case TOKEN_FOR:
            result = parse_for_statement(parser);
            break;
            
        case TOKEN_SWITCH:
            result = parse_switch_statement(parser);
            break;
            
        case TOKEN_RETURN:
            result = parse_return_statement(parser);
            break;
            
        case TOKEN_BREAK:
            result = parse_break_statement(parser);
            break;
            
        case TOKEN_CONTINUE:
            result = parse_continue_statement(parser);
            break;
            
        case TOKEN_THROW:
            result = parse_throw_statement(parser);
            break;
            
        case TOKEN_TRY:
            result = parse_try_statement(parser);
            break;
            
        case TOKEN_LBRACE:
            result = parse_block_statement(parser);
            break;
            
        case TOKEN_SEMICOLON:
            /* 空语句 */
            parser_advance(parser);
            result = true;
            break;
            
        default:
            /* 表达式语句 */
            result = parse_expression_statement(parser);
            break;
    }
    
    parser->depth--;
    return result;
}

/* 解析变量声明 */
bool parse_variable_declaration(Parser *parser) {
    /* var/let/const */
    parser_advance(parser);
    
    /* 至少一个标识符 */
    if (!parser_expect(parser, TOKEN_IDENTIFIER)) {
        return false;
    }
    
    /* 可选的初始化 */
    if (parser_match(parser, TOKEN_ASSIGN)) {
        if (!parse_assignment_expression(parser)) {
            return false;
        }
    }
    
    /* 可能有多个声明 */
    while (parser_match(parser, TOKEN_COMMA)) {
        if (!parser_expect(parser, TOKEN_IDENTIFIER)) {
            return false;
        }
        
        if (parser_match(parser, TOKEN_ASSIGN)) {
            if (!parse_assignment_expression(parser)) {
                return false;
            }
        }
    }
    
    /* 分号（或ASI） */
    return parser_consume_semicolon(parser);
}

/* 解析函数声明 */
bool parse_function_declaration(Parser *parser) {
    /* function */
    parser_advance(parser);
    
    /* 函数名（可选，用于函数表达式） */
    if (parser_check(parser, TOKEN_IDENTIFIER)) {
        parser_advance(parser);
    }
    
    /* 参数列表 */
    if (!parser_expect(parser, TOKEN_LPAREN)) {
        return false;
    }
    
    /* 参数 */
    if (!parser_check(parser, TOKEN_RPAREN)) {
        if (!parser_expect(parser, TOKEN_IDENTIFIER)) {
            return false;
        }
        
        while (parser_match(parser, TOKEN_COMMA)) {
            if (!parser_expect(parser, TOKEN_IDENTIFIER)) {
                return false;
            }
        }
    }
    
    if (!parser_expect(parser, TOKEN_RPAREN)) {
        return false;
    }
    
    /* 函数体 */
    return parse_block_statement(parser);
}

/* 解析类声明 */
bool parse_class_declaration(Parser *parser) {
    /* class */
    parser_advance(parser);
    
    /* 类名 */
    if (!parser_expect(parser, TOKEN_IDENTIFIER)) {
        return false;
    }
    
    /* extends */
    if (parser_match(parser, TOKEN_EXTENDS)) {
        if (!parser_expect(parser, TOKEN_IDENTIFIER)) {
            return false;
        }
    }
    
    /* 类体 */
    if (!parser_expect(parser, TOKEN_LBRACE)) {
        return false;
    }
    
    /* 成员（简化处理） */
    while (!parser_check(parser, TOKEN_RBRACE) && 
           !parser_check(parser, TOKEN_EOF)) {
        /* 方法或属性 */
        if (parser_check(parser, TOKEN_IDENTIFIER) ||
            parser_check(parser, TOKEN_STATIC) ||
            parser_check(parser, TOKEN_GET) ||
            parser_check(parser, TOKEN_SET)) {
            parser_advance(parser);
            
            /* 如果是函数 */
            if (parser_match(parser, TOKEN_LPAREN)) {
                /* 参数 */
                while (!parser_check(parser, TOKEN_RPAREN) && 
                       !parser_check(parser, TOKEN_EOF)) {
                    parser_advance(parser);
                }
                if (!parser_expect(parser, TOKEN_RPAREN)) {
                    return false;
                }
                
                /* 函数体 */
                if (!parse_block_statement(parser)) {
                    return false;
                }
            }
        } else {
            parser_advance(parser);
        }
    }
    
    return parser_expect(parser, TOKEN_RBRACE);
}

/* 解析表达式语句 */
bool parse_expression_statement(Parser *parser) {
    if (!parse_expression(parser)) {
        return false;
    }
    
    return parser_consume_semicolon(parser);
}

/* 解析if语句 */
bool parse_if_statement(Parser *parser) {
    /* if */
    parser_advance(parser);
    
    if (!parser_expect(parser, TOKEN_LPAREN)) {
        return false;
    }
    
    if (!parse_expression(parser)) {
        return false;
    }
    
    if (!parser_expect(parser, TOKEN_RPAREN)) {
        return false;
    }
    
    if (!parse_statement(parser)) {
        return false;
    }
    
    /* else */
    if (parser_match(parser, TOKEN_ELSE)) {
        if (!parse_statement(parser)) {
            return false;
        }
    }
    
    return true;
}

/* 解析while语句 */
bool parse_while_statement(Parser *parser) {
    /* while */
    parser_advance(parser);
    
    if (!parser_expect(parser, TOKEN_LPAREN)) {
        return false;
    }
    
    if (!parse_expression(parser)) {
        return false;
    }
    
    if (!parser_expect(parser, TOKEN_RPAREN)) {
        return false;
    }
    
    return parse_statement(parser);
}

/* 解析do-while语句 */
bool parse_do_while_statement(Parser *parser) {
    /* do */
    parser_advance(parser);
    
    if (!parse_statement(parser)) {
        return false;
    }
    
    if (!parser_expect(parser, TOKEN_WHILE)) {
        return false;
    }
    
    if (!parser_expect(parser, TOKEN_LPAREN)) {
        return false;
    }
    
    if (!parse_expression(parser)) {
        return false;
    }
    
    if (!parser_expect(parser, TOKEN_RPAREN)) {
        return false;
    }
    
    return parser_consume_semicolon(parser);
}

/* 解析for语句 */
bool parse_for_statement(Parser *parser) {
    /* for */
    parser_advance(parser);
    
    if (!parser_expect(parser, TOKEN_LPAREN)) {
        return false;
    }
    
    /* 初始化部分 */
    if (!parser_check(parser, TOKEN_SEMICOLON)) {
        if (parser_check(parser, TOKEN_VAR) || 
            parser_check(parser, TOKEN_LET) ||
            parser_check(parser, TOKEN_CONST)) {
            parser_advance(parser);
            
            if (!parser_expect(parser, TOKEN_IDENTIFIER)) {
                return false;
            }
            
            /* for-in or for-of */
            if (parser_check(parser, TOKEN_IN) || 
                parser_check(parser, TOKEN_OF)) {
                parser_advance(parser);
                if (!parse_expression(parser)) {
                    return false;
                }
                if (!parser_expect(parser, TOKEN_RPAREN)) {
                    return false;
                }
                return parse_statement(parser);
            }
            
            /* 普通for循环的声明 */
            if (parser_match(parser, TOKEN_ASSIGN)) {
                if (!parse_assignment_expression(parser)) {
                    return false;
                }
            }
        } else {
            if (!parse_expression(parser)) {
                return false;
            }
        }
    }
    
    if (!parser_expect(parser, TOKEN_SEMICOLON)) {
        return false;
    }
    
    /* 条件部分 */
    if (!parser_check(parser, TOKEN_SEMICOLON)) {
        if (!parse_expression(parser)) {
            return false;
        }
    }
    
    if (!parser_expect(parser, TOKEN_SEMICOLON)) {
        return false;
    }
    
    /* 更新部分 */
    if (!parser_check(parser, TOKEN_RPAREN)) {
        if (!parse_expression(parser)) {
            return false;
        }
    }
    
    if (!parser_expect(parser, TOKEN_RPAREN)) {
        return false;
    }
    
    return parse_statement(parser);
}

/* 解析switch语句 */
bool parse_switch_statement(Parser *parser) {
    /* switch */
    parser_advance(parser);
    
    if (!parser_expect(parser, TOKEN_LPAREN)) {
        return false;
    }
    
    if (!parse_expression(parser)) {
        return false;
    }
    
    if (!parser_expect(parser, TOKEN_RPAREN)) {
        return false;
    }
    
    if (!parser_expect(parser, TOKEN_LBRACE)) {
        return false;
    }
    
    /* case子句 */
    while (parser_check(parser, TOKEN_CASE) || 
           parser_check(parser, TOKEN_DEFAULT)) {
        parser_advance(parser);
        
        if (parser->prev_token->type == TOKEN_CASE) {
            if (!parse_expression(parser)) {
                return false;
            }
        }
        
        if (!parser_expect(parser, TOKEN_COLON)) {
            return false;
        }
        
        /* 语句列表 */
        while (!parser_check(parser, TOKEN_CASE) &&
               !parser_check(parser, TOKEN_DEFAULT) &&
               !parser_check(parser, TOKEN_RBRACE) &&
               !parser_check(parser, TOKEN_EOF)) {
            if (!parse_statement(parser)) {
                return false;
            }
        }
    }
    
    return parser_expect(parser, TOKEN_RBRACE);
}

/* 解析return语句 */
bool parse_return_statement(Parser *parser) {
    /* return */
    parser_advance(parser);
    
    /* ASI规则：return后换行则自动插入分号 */
    if (parser->current_token->preceded_by_newline) {
        return true;
    }
    
    /* 表达式 */
    if (!parser_check(parser, TOKEN_SEMICOLON) &&
        !parser_check(parser, TOKEN_RBRACE) &&
        !parser_check(parser, TOKEN_EOF)) {
        if (!parse_expression(parser)) {
            return false;
        }
    }
    
    return parser_consume_semicolon(parser);
}

/* 解析break语句 */
bool parse_break_statement(Parser *parser) {
    /* break */
    parser_advance(parser);
    
    /* ASI规则：break后换行则自动插入分号 */
    if (parser->current_token->preceded_by_newline) {
        return true;
    }
    
    /* 可选标签 */
    if (parser_check(parser, TOKEN_IDENTIFIER)) {
        parser_advance(parser);
    }
    
    return parser_consume_semicolon(parser);
}

/* 解析continue语句 */
bool parse_continue_statement(Parser *parser) {
    /* continue */
    parser_advance(parser);
    
    /* ASI规则：continue后换行则自动插入分号 */
    if (parser->current_token->preceded_by_newline) {
        return true;
    }
    
    /* 可选标签 */
    if (parser_check(parser, TOKEN_IDENTIFIER)) {
        parser_advance(parser);
    }
    
    return parser_consume_semicolon(parser);
}

/* 解析throw语句 */
bool parse_throw_statement(Parser *parser) {
    /* throw */
    parser_advance(parser);
    
    /* ASI规则：throw后不允许换行 */
    if (parser->current_token->preceded_by_newline) {
        set_error(parser->error, ERROR_PARSER_UNEXPECTED_TOKEN,
                 parser->current_token->start, 
                 "Line break is not allowed between 'throw' and its expression");
        return false;
    }
    
    if (!parse_expression(parser)) {
        return false;
    }
    
    return parser_consume_semicolon(parser);
}

/* 解析try语句 */
bool parse_try_statement(Parser *parser) {
    /* try */
    parser_advance(parser);
    
    if (!parse_block_statement(parser)) {
        return false;
    }
    
    /* catch */
    if (parser_match(parser, TOKEN_CATCH)) {
        /* 可选的参数 */
        if (parser_match(parser, TOKEN_LPAREN)) {
            if (!parser_expect(parser, TOKEN_IDENTIFIER)) {
                return false;
            }
            if (!parser_expect(parser, TOKEN_RPAREN)) {
                return false;
            }
        }
        
        if (!parse_block_statement(parser)) {
            return false;
        }
    }
    
    /* finally */
    if (parser_match(parser, TOKEN_FINALLY)) {
        if (!parse_block_statement(parser)) {
            return false;
        }
    }
    
    return true;
}

/* 解析块语句 */
bool parse_block_statement(Parser *parser) {
    if (!parser_expect(parser, TOKEN_LBRACE)) {
        return false;
    }
    
    if (!parse_statement_list(parser)) {
        return false;
    }
    
    return parser_expect(parser, TOKEN_RBRACE);
}

/* 解析表达式 */
bool parse_expression(Parser *parser) {
    if (!parse_assignment_expression(parser)) {
        return false;
    }
    
    /* 逗号表达式 */
    while (parser_match(parser, TOKEN_COMMA)) {
        if (!parse_assignment_expression(parser)) {
            return false;
        }
    }
    
    return true;
}

/* 解析赋值表达式 */
bool parse_assignment_expression(Parser *parser) {
    /* 条件表达式 */
    if (!parse_conditional_expression(parser)) {
        return false;
    }
    
    /* 赋值运算符 */
    if (is_assignment_operator(parser->current_token->type)) {
        parser_advance(parser);
        if (!parse_assignment_expression(parser)) {
            return false;
        }
    }
    
    return true;
}

/* 解析条件表达式 */
bool parse_conditional_expression(Parser *parser) {
    if (!parse_logical_or_expression(parser)) {
        return false;
    }
    
    if (parser_match(parser, TOKEN_QUESTION)) {
        if (!parse_assignment_expression(parser)) {
            return false;
        }
        
        if (!parser_expect(parser, TOKEN_COLON)) {
            return false;
        }
        
        if (!parse_assignment_expression(parser)) {
            return false;
        }
    }
    
    return true;
}

/* 解析逻辑或表达式 */
bool parse_logical_or_expression(Parser *parser) {
    if (!parse_logical_and_expression(parser)) {
        return false;
    }
    
    while (parser_match(parser, TOKEN_OR) || 
           parser_match(parser, TOKEN_NULLISH)) {
        if (!parse_logical_and_expression(parser)) {
            return false;
        }
    }
    
    return true;
}

/* 解析逻辑与表达式 */
bool parse_logical_and_expression(Parser *parser) {
    if (!parse_bitwise_or_expression(parser)) {
        return false;
    }
    
    while (parser_match(parser, TOKEN_AND)) {
        if (!parse_bitwise_or_expression(parser)) {
            return false;
        }
    }
    
    return true;
}

/* 解析位或表达式 */
bool parse_bitwise_or_expression(Parser *parser) {
    if (!parse_bitwise_xor_expression(parser)) {
        return false;
    }
    
    while (parser_match(parser, TOKEN_BITWISE_OR)) {
        if (!parse_bitwise_xor_expression(parser)) {
            return false;
        }
    }
    
    return true;
}

/* 解析位异或表达式 */
bool parse_bitwise_xor_expression(Parser *parser) {
    if (!parse_bitwise_and_expression(parser)) {
        return false;
    }
    
    while (parser_match(parser, TOKEN_BITWISE_XOR)) {
        if (!parse_bitwise_and_expression(parser)) {
            return false;
        }
    }
    
    return true;
}

/* 解析位与表达式 */
bool parse_bitwise_and_expression(Parser *parser) {
    if (!parse_equality_expression(parser)) {
        return false;
    }
    
    while (parser_match(parser, TOKEN_BITWISE_AND)) {
        if (!parse_equality_expression(parser)) {
            return false;
        }
    }
    
    return true;
}

/* 解析相等表达式 */
bool parse_equality_expression(Parser *parser) {
    if (!parse_relational_expression(parser)) {
        return false;
    }
    
    while (parser_check(parser, TOKEN_EQ) ||
           parser_check(parser, TOKEN_NE) ||
           parser_check(parser, TOKEN_EQ_STRICT) ||
           parser_check(parser, TOKEN_NE_STRICT)) {
        parser_advance(parser);
        if (!parse_relational_expression(parser)) {
            return false;
        }
    }
    
    return true;
}

/* 解析关系表达式 */
bool parse_relational_expression(Parser *parser) {
    if (!parse_shift_expression(parser)) {
        return false;
    }
    
    while (parser_check(parser, TOKEN_LT) ||
           parser_check(parser, TOKEN_LE) ||
           parser_check(parser, TOKEN_GT) ||
           parser_check(parser, TOKEN_GE) ||
           parser_check(parser, TOKEN_INSTANCEOF) ||
           parser_check(parser, TOKEN_IN)) {
        parser_advance(parser);
        if (!parse_shift_expression(parser)) {
            return false;
        }
    }
    
    return true;
}

/* 解析移位表达式 */
bool parse_shift_expression(Parser *parser) {
    if (!parse_additive_expression(parser)) {
        return false;
    }
    
    while (parser_check(parser, TOKEN_LSHIFT) ||
           parser_check(parser, TOKEN_RSHIFT) ||
           parser_check(parser, TOKEN_URSHIFT)) {
        parser_advance(parser);
        if (!parse_additive_expression(parser)) {
            return false;
        }
    }
    
    return true;
}

/* 解析加法表达式 */
bool parse_additive_expression(Parser *parser) {
    if (!parse_multiplicative_expression(parser)) {
        return false;
    }
    
    while (parser_check(parser, TOKEN_PLUS) ||
           parser_check(parser, TOKEN_MINUS)) {
        parser_advance(parser);
        if (!parse_multiplicative_expression(parser)) {
            return false;
        }
    }
    
    return true;
}

/* 解析乘法表达式 */
bool parse_multiplicative_expression(Parser *parser) {
    if (!parse_exponentiation_expression(parser)) {
        return false;
    }
    
    while (parser_check(parser, TOKEN_MULTIPLY) ||
           parser_check(parser, TOKEN_DIVIDE) ||
           parser_check(parser, TOKEN_MODULO)) {
        parser_advance(parser);
        if (!parse_exponentiation_expression(parser)) {
            return false;
        }
    }
    
    return true;
}

/* 解析指数表达式 */
bool parse_exponentiation_expression(Parser *parser) {
    if (!parse_unary_expression(parser)) {
        return false;
    }
    
    if (parser_match(parser, TOKEN_EXPONENT)) {
        if (!parse_exponentiation_expression(parser)) {
            return false;
        }
    }
    
    return true;
}

/* 解析一元表达式 */
bool parse_unary_expression(Parser *parser) {
    if (is_unary_operator(parser->current_token->type)) {
        parser_advance(parser);
        return parse_unary_expression(parser);
    }
    
    return parse_postfix_expression(parser);
}

/* 解析后缀表达式 */
bool parse_postfix_expression(Parser *parser) {
    if (!parse_left_hand_side_expression(parser)) {
        return false;
    }
    
    /* ++ 或 -- */
    if (!parser->current_token->preceded_by_newline) {
        if (parser_check(parser, TOKEN_INCREMENT) ||
            parser_check(parser, TOKEN_DECREMENT)) {
            parser_advance(parser);
        }
    }
    
    return true;
}

/* 解析左侧表达式 */
bool parse_left_hand_side_expression(Parser *parser) {
    /* new表达式 */
    if (parser_match(parser, TOKEN_NEW)) {
        if (!parse_member_expression(parser)) {
            return false;
        }
        
        /* new后面可以有参数列表 */
        if (parser_check(parser, TOKEN_LPAREN)) {
            parser_advance(parser);
            
            /* 参数列表 */
            if (!parser_check(parser, TOKEN_RPAREN)) {
                if (!parse_assignment_expression(parser)) {
                    return false;
                }
                
                while (parser_match(parser, TOKEN_COMMA)) {
                    if (!parse_assignment_expression(parser)) {
                        return false;
                    }
                }
            }
            
            if (!parser_expect(parser, TOKEN_RPAREN)) {
                return false;
            }
        }
        
        return true;
    }
    
    return parse_call_expression(parser);
}

/* 解析调用表达式 */
bool parse_call_expression(Parser *parser) {
    if (!parse_member_expression(parser)) {
        return false;
    }
    
    /* 函数调用 */
    while (parser_check(parser, TOKEN_LPAREN)) {
        parser_advance(parser);
        
        /* 参数列表 */
        if (!parser_check(parser, TOKEN_RPAREN)) {
            if (!parse_assignment_expression(parser)) {
                return false;
            }
            
            while (parser_match(parser, TOKEN_COMMA)) {
                if (!parse_assignment_expression(parser)) {
                    return false;
                }
            }
        }
        
        if (!parser_expect(parser, TOKEN_RPAREN)) {
            return false;
        }
        
        /* 调用后可以继续访问成员 */
        while (parser_check(parser, TOKEN_DOT) ||
               parser_check(parser, TOKEN_LBRACKET) ||
               parser_check(parser, TOKEN_OPTIONAL_CHAIN)) {
            if (parser_match(parser, TOKEN_DOT) ||
                parser_match(parser, TOKEN_OPTIONAL_CHAIN)) {
                if (!parser_expect(parser, TOKEN_IDENTIFIER)) {
                    return false;
                }
            } else if (parser_match(parser, TOKEN_LBRACKET)) {
                if (!parse_expression(parser)) {
                    return false;
                }
                if (!parser_expect(parser, TOKEN_RBRACKET)) {
                    return false;
                }
            }
        }
    }
    
    return true;
}

/* 解析成员表达式 */
bool parse_member_expression(Parser *parser) {
    if (!parse_primary_expression(parser)) {
        return false;
    }
    
    /* 成员访问 */
    while (parser_check(parser, TOKEN_DOT) ||
           parser_check(parser, TOKEN_LBRACKET) ||
           parser_check(parser, TOKEN_OPTIONAL_CHAIN)) {
        if (parser_match(parser, TOKEN_DOT) ||
            parser_match(parser, TOKEN_OPTIONAL_CHAIN)) {
            if (!parser_expect(parser, TOKEN_IDENTIFIER)) {
                return false;
            }
        } else if (parser_match(parser, TOKEN_LBRACKET)) {
            if (!parse_expression(parser)) {
                return false;
            }
            if (!parser_expect(parser, TOKEN_RBRACKET)) {
                return false;
            }
        }
    }
    
    return true;
}

/* 觨析主表达式 */
bool parse_primary_expression(Parser *parser) {
    if (!parser->current_token) {
        set_error(parser->error, ERROR_PARSER_UNEXPECTED_EOF,
                 parser->prev_token ? parser->prev_token->end : (Position){1, 1, 0},
                 "Unexpected end of file");
        return false;
    }
    
    switch (parser->current_token->type) {
        case TOKEN_IDENTIFIER:
        case TOKEN_THIS:
        case TOKEN_SUPER:
        case TOKEN_NUMBER:
        case TOKEN_STRING:
        case TOKEN_TRUE:
        case TOKEN_FALSE:
        case TOKEN_NULL:
        case TOKEN_UNDEFINED:
        case TOKEN_REGEX:
        case TOKEN_TEMPLATE:
            parser_advance(parser);
            return true;
            
        case TOKEN_LPAREN:
            /* 分组表达式或箭头函数参数 */
            parser_advance(parser);
            if (!parse_expression(parser)) {
                return false;
            }
            return parser_expect(parser, TOKEN_RPAREN);
            
        case TOKEN_LBRACKET:
            return parse_array_literal(parser);
            
        case TOKEN_LBRACE:
            return parse_object_literal(parser);
            
        case TOKEN_FUNCTION:
            return parse_function_declaration(parser);
            
        case TOKEN_EOF:
            set_error(parser->error, ERROR_PARSER_UNEXPECTED_EOF,
                     parser->current_token->start,
                     "Unexpected end of file in expression");
            return false;
            
        default:
            {
                char msg[256];
                snprintf(msg, sizeof(msg), "Unexpected token type %d in expression",
                         parser->current_token->type);
                set_error(parser->error, ERROR_PARSER_UNEXPECTED_TOKEN,
                         parser->current_token->start, msg);
                return false;
            }
    }
}

/* 解析数组字面量 */
bool parse_array_literal(Parser *parser) {
    if (!parser_expect(parser, TOKEN_LBRACKET)) {
        return false;
    }
    
    /* 元素 */
    if (!parser_check(parser, TOKEN_RBRACKET)) {
        /* 允许省略元素 */
        if (!parser_check(parser, TOKEN_COMMA)) {
            if (!parse_assignment_expression(parser)) {
                return false;
            }
        }
        
        while (parser_match(parser, TOKEN_COMMA)) {
            if (parser_check(parser, TOKEN_RBRACKET)) break;
            
            if (!parser_check(parser, TOKEN_COMMA)) {
                if (!parse_assignment_expression(parser)) {
                    return false;
                }
            }
        }
    }
    
    return parser_expect(parser, TOKEN_RBRACKET);
}

/* 解析对象字面量 */
bool parse_object_literal(Parser *parser) {
    if (!parser_expect(parser, TOKEN_LBRACE)) {
        return false;
    }
    
    /* 属性 */
    while (!parser_check(parser, TOKEN_RBRACE) && 
           !parser_check(parser, TOKEN_EOF)) {
        /* 属性名 */
        if (parser_check(parser, TOKEN_IDENTIFIER) ||
            parser_check(parser, TOKEN_STRING) ||
            parser_check(parser, TOKEN_NUMBER)) {
            parser_advance(parser);
        } else if (parser_match(parser, TOKEN_LBRACKET)) {
            /* 计算属性名 */
            if (!parse_expression(parser)) {
                return false;
            }
            if (!parser_expect(parser, TOKEN_RBRACKET)) {
                return false;
            }
        } else if (parser_check(parser, TOKEN_SPREAD)) {
            /* 展开运算符 */
            parser_advance(parser);
            if (!parse_assignment_expression(parser)) {
                return false;
            }
            if (parser_match(parser, TOKEN_COMMA)) {
                continue;
            }
            break;
        } else {
            break;
        }
        
        /* 方法或属性值 */
        if (parser_match(parser, TOKEN_COLON)) {
            if (!parse_assignment_expression(parser)) {
                return false;
            }
        } else if (parser_match(parser, TOKEN_LPAREN)) {
            /* 方法 */
            while (!parser_check(parser, TOKEN_RPAREN) && 
                   !parser_check(parser, TOKEN_EOF)) {
                parser_advance(parser);
            }
            if (!parser_expect(parser, TOKEN_RPAREN)) {
                return false;
            }
            if (!parse_block_statement(parser)) {
                return false;
            }
        }
        
        if (!parser_match(parser, TOKEN_COMMA)) {
            break;
        }
    }
    
    return parser_expect(parser, TOKEN_RBRACE);
}

/* 主解析函数 */
bool parser_parse(Parser *parser) {
    if (!parser || !parser->current_token) {
        return false;
    }
    
    return parse_program(parser);
}
