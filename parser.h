#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "common.h"

/* 语法分析器状态 */
typedef struct {
    Lexer *lexer;           /* 词法分析器 */
    Token *current_token;   /* 当前token */
    Token *prev_token;      /* 上一个token */
    ErrorInfo *error;       /* 错误信息 */
    bool asi_allowed;       /* 是否允许ASI插入 */
    int depth;              /* 递归深度（防止栈溢出） */
} Parser;

/* 语法分析器函数声明 */
Parser* parser_create(Lexer *lexer, ErrorInfo *error);
void parser_destroy(Parser *parser);
bool parser_parse(Parser *parser);

/* ASI相关函数 */
bool parser_check_asi(Parser *parser);
bool parser_consume_semicolon(Parser *parser);
bool parser_should_insert_semicolon(Parser *parser, TokenType prev_type);

/* token操作函数 */
bool parser_advance(Parser *parser);
bool parser_expect(Parser *parser, TokenType type);
bool parser_match(Parser *parser, TokenType type);
bool parser_check(Parser *parser, TokenType type);

/* 语法解析函数（递归下降） */
bool parse_program(Parser *parser);
bool parse_statement_list(Parser *parser);
bool parse_statement(Parser *parser);
bool parse_declaration(Parser *parser);
bool parse_variable_declaration(Parser *parser);
bool parse_function_declaration(Parser *parser);
bool parse_class_declaration(Parser *parser);
bool parse_expression_statement(Parser *parser);
bool parse_if_statement(Parser *parser);
bool parse_while_statement(Parser *parser);
bool parse_do_while_statement(Parser *parser);
bool parse_for_statement(Parser *parser);
bool parse_switch_statement(Parser *parser);
bool parse_return_statement(Parser *parser);
bool parse_break_statement(Parser *parser);
bool parse_continue_statement(Parser *parser);
bool parse_throw_statement(Parser *parser);
bool parse_try_statement(Parser *parser);
bool parse_block_statement(Parser *parser);

/* 表达式解析函数 */
bool parse_expression(Parser *parser);
bool parse_assignment_expression(Parser *parser);
bool parse_conditional_expression(Parser *parser);
bool parse_logical_or_expression(Parser *parser);
bool parse_logical_and_expression(Parser *parser);
bool parse_bitwise_or_expression(Parser *parser);
bool parse_bitwise_xor_expression(Parser *parser);
bool parse_bitwise_and_expression(Parser *parser);
bool parse_equality_expression(Parser *parser);
bool parse_relational_expression(Parser *parser);
bool parse_shift_expression(Parser *parser);
bool parse_additive_expression(Parser *parser);
bool parse_multiplicative_expression(Parser *parser);
bool parse_exponentiation_expression(Parser *parser);
bool parse_unary_expression(Parser *parser);
bool parse_postfix_expression(Parser *parser);
bool parse_left_hand_side_expression(Parser *parser);
bool parse_call_expression(Parser *parser);
bool parse_member_expression(Parser *parser);
bool parse_primary_expression(Parser *parser);
bool parse_array_literal(Parser *parser);
bool parse_object_literal(Parser *parser);
bool parse_arrow_function(Parser *parser);

/* 辅助函数 */
bool is_assignment_operator(TokenType type);
bool is_unary_operator(TokenType type);
bool is_statement_start(TokenType type);

#endif /* PARSER_H */
