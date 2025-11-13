#include "lexer.h"
#include "common.h"
#include <stdio.h>

int main(int argc, char **argv) {
    const char *source = argc > 1 ? argv[1] : "x.y";
    ErrorInfo error = {0};
    
    Lexer *lexer = lexer_create(source, strlen(source), &error);
    
    Token *token;
    while ((token = lexer_next_token(lexer)) && token->type != TOKEN_EOF) {
        printf("Token: type=%d, value='%s', line=%d, col=%d\n",
               token->type, token->value ? token->value : "(null)",
               token->start.line, token->start.column);
        token_destroy(token);
    }
    
    if (token) token_destroy(token);
    lexer_destroy(lexer);
    
    return 0;
}
