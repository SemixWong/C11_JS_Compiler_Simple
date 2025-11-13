#include "parser.h"
#include <stdio.h>

int main(int argc, char **argv) {
    const char *source = argc > 1 ? argv[1] : "x.y";
    ErrorInfo error = {0};
    
    Lexer *lexer = lexer_create(source, strlen(source), &error);
    Parser *parser = parser_create(lexer, &error);
    
    printf("Initial token: type=%d, value='%s'\n", 
           parser->current_token->type,
           parser->current_token->value ? parser->current_token->value : "(null)");
    
    // 手动模拟parse_member_expression
    // 先是primary_expression: 消耗 'x'
    printf("Primary: consume 'x'\n");
    parser_advance(parser);
    printf("After primary: type=%d, value='%s'\n",
           parser->current_token->type,
           parser->current_token->value ? parser->current_token->value : "(null)");
    
    // 检查是否是DOT
    if (parser_check(parser, TOKEN_DOT)) {
        printf("Found DOT\n");
        parser_advance(parser);  // match DOT
        printf("After DOT: type=%d, value='%s'\n",
               parser->current_token->type,
               parser->current_token->value ? parser->current_token->value : "(null)");
        
        // 期待IDENTIFIER
        if (parser_check(parser, TOKEN_IDENTIFIER)) {
            printf("Found IDENTIFIER 'y'\n");
        } else {
            printf("ERROR: Expected IDENTIFIER but got type=%d\n",
                   parser->current_token->type);
        }
    }
    
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return 0;
}
