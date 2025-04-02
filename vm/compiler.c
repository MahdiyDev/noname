#include "compiler.h"
#include "scanner.h"
#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

static void binary(Compiler* compiler);
static ParseRule* get_rule(TokenType type);

static void error_at(Parser* parser, Token* token, const char* message)
{
    if (parser->panic_mode) return;
    parser->panic_mode = true;

    fprintf(stderr, "[line %d] Error", token->line);
    
    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing.
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }
    fprintf(stderr, ": %s\n", message);
    parser->had_error = true;
}

static void error(Parser* parser, const char* message)
{
    error_at(parser, &parser->previous, message);
}

static void error_at_current(Parser* parser, const char* message)
{
    error_at(parser, &parser->current, message);
}

static void advance(Compiler* compiler)
{
    compiler->parser.previous = compiler->parser.current;

    for (;;) {
        compiler->parser.current = scan_token(&compiler->scanner);
        if (compiler->parser.current.type != TOKEN_ERROR) break;

        error_at_current(&compiler->parser, compiler->parser.current.start);
    }
}

static void consume(Compiler* compiler, TokenType type, const char* message)
{
    if (compiler->parser.current.type == type) {
        advance(compiler);
        return;
    }

    error_at_current(&compiler->parser, message);
}

static Chunk* current_chunk(Compiler* compiler)
{
    return compiler->compiling_chunk;
}

static void emit_byte(Compiler* compiler, uint8_t byte)
{
    write_chunk(compiler->compiling_chunk, byte, compiler->parser.previous.line);
}

static void emit_bytes(Compiler* compiler, uint8_t byte1, uint8_t byte2)
{
    emit_byte(compiler, byte1);
    emit_byte(compiler, byte2);
}

static uint8_t make_constant(Compiler* compiler, Value value)
{
    int constant = add_constant(current_chunk(compiler), value);
    if (constant > UINT8_MAX) {
        error(&compiler->parser, "Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

static void emit_constant(Compiler* compiler, Value value)
{
    emit_bytes(compiler, OP_CONSTANT, make_constant(compiler, value));
}

static void emit_return(Compiler* compiler)
{
    emit_byte(compiler, OP_RETURN);
}

static void end_compiler(Compiler* compiler)
{
    emit_return(compiler);
#ifdef DEBUG_PRINT_CODE
    if (!compiler->parser.had_error) {
        disassemble_chunk(current_chunk(compiler), "code");
    }
#endif
}

static void number(Compiler* compiler)
{
    double value = strtod(compiler->parser.previous.start, NULL);
    emit_constant(compiler, NUMBER_VAL(value));
}

static void parse_precedence(Compiler* compiler, Precedence precedence)
{
    advance(compiler);

    ParseFn prefix_rule = get_rule(compiler->parser.previous.type)->prefix;
    if (prefix_rule == NULL) {
        error(&compiler->parser, "Expect expression.");
        return;
    }

    prefix_rule(compiler);

    while (precedence <= get_rule(compiler->parser.current.type)->precedence) {
        advance(compiler);
        ParseFn infix_rule = get_rule(compiler->parser.previous.type)->infix;
        infix_rule(compiler);
    }
}

static void expression(Compiler* compiler)
{
    parse_precedence(compiler, PREC_ASSIGNMENT);
}

static void grouping(Compiler* compiler)
{
    expression(compiler);
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void unary(Compiler* compiler)
{
    TokenType operator_type = compiler->parser.previous.type;

    // Compile the operand.
    parse_precedence(compiler, PREC_UNARY);

    // Emit the operator instruction.
    switch (operator_type) {
        case TOKEN_BANG: emit_byte(compiler, OP_NOT); break;
        case TOKEN_MINUS: emit_byte(compiler, OP_NEGATE); break;
        default: return; // Unreachable.
    }
}

static void literal(Compiler* compiler) {
    switch (compiler->parser.previous.type) {
      case TOKEN_FALSE: emit_byte(compiler, OP_FALSE); break;
      case TOKEN_NIL: emit_byte(compiler, OP_NIL); break;
      case TOKEN_TRUE: emit_byte(compiler, OP_TRUE); break;
      default: return; // Unreachable.
    }
}

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN]    = { grouping, NULL,   PREC_NONE },
    [TOKEN_RIGHT_PAREN]   = { NULL,     NULL,   PREC_NONE },
    [TOKEN_LEFT_BRACE]    = { NULL,     NULL,   PREC_NONE} , 
    [TOKEN_RIGHT_BRACE]   = { NULL,     NULL,   PREC_NONE },
    [TOKEN_COMMA]         = { NULL,     NULL,   PREC_NONE },
    [TOKEN_DOT]           = { NULL,     NULL,   PREC_NONE },
    [TOKEN_MINUS]         = { unary,    binary, PREC_TERM },
    [TOKEN_PLUS]          = { NULL,     binary, PREC_TERM },
    [TOKEN_SEMICOLON]     = { NULL,     NULL,   PREC_NONE },
    [TOKEN_SLASH]         = { NULL,     binary, PREC_FACTOR },
    [TOKEN_STAR]          = { NULL,     binary, PREC_FACTOR },
    [TOKEN_BANG]          = { unary,    NULL,   PREC_NONE },
    [TOKEN_BANG_EQUAL]    = { NULL,     binary, PREC_EQUALITY },
    [TOKEN_EQUAL]         = { NULL,     NULL,   PREC_NONE },
    [TOKEN_EQUAL_EQUAL]   = { NULL,     binary, PREC_EQUALITY},
    [TOKEN_GREATER]       = { NULL,     binary, PREC_COMPARISON },
    [TOKEN_GREATER_EQUAL] = { NULL,     binary, PREC_COMPARISON },
    [TOKEN_LESS]          = { NULL,     binary, PREC_COMPARISON },
    [TOKEN_LESS_EQUAL]    = { NULL,     binary, PREC_COMPARISON },
    [TOKEN_IDENTIFIER]    = { NULL,     NULL,   PREC_NONE },
    [TOKEN_STRING]        = { NULL,     NULL,   PREC_NONE },
    [TOKEN_NUMBER]        = { number,   NULL,   PREC_NONE },
    [TOKEN_AND]           = { NULL,     NULL,   PREC_NONE },
    [TOKEN_CLASS]         = { NULL,     NULL,   PREC_NONE },
    [TOKEN_ELSE]          = { NULL,     NULL,   PREC_NONE },
    [TOKEN_FALSE]         = { literal,  NULL,   PREC_NONE },
    [TOKEN_FOR]           = { NULL,     NULL,   PREC_NONE },
    [TOKEN_FUN]           = { NULL,     NULL,   PREC_NONE },
    [TOKEN_IF]            = { NULL,     NULL,   PREC_NONE },
    [TOKEN_NIL]           = { literal,  NULL,   PREC_NONE },
    [TOKEN_OR]            = { NULL,     NULL,   PREC_NONE },
    [TOKEN_PRINT]         = { NULL,     NULL,   PREC_NONE },
    [TOKEN_RETURN]        = { NULL,     NULL,   PREC_NONE },
    [TOKEN_SUPER]         = { NULL,     NULL,   PREC_NONE },
    [TOKEN_THIS]          = { NULL,     NULL,   PREC_NONE },
    [TOKEN_TRUE]          = { literal,  NULL,   PREC_NONE },
    [TOKEN_VAR]           = { NULL,     NULL,   PREC_NONE },
    [TOKEN_WHILE]         = { NULL,     NULL,   PREC_NONE },
    [TOKEN_ERROR]         = { NULL,     NULL,   PREC_NONE },
    [TOKEN_EOF]           = { NULL,     NULL,   PREC_NONE },
};

static ParseRule* get_rule(TokenType type)
{
    return &rules[type];
}

static void binary(Compiler* compiler)
{
    // Remember the operator.
    TokenType operator_type = compiler->parser.previous.type;

    // Compile the right operand.
    ParseRule* rule = get_rule(operator_type);
    parse_precedence(compiler, (Precedence)(rule->precedence + 1));

    // Emit the operator instruction.
    switch (operator_type) {
        case TOKEN_BANG_EQUAL:    emit_bytes(compiler, OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:   emit_byte(compiler, OP_EQUAL); break;
        case TOKEN_GREATER:       emit_byte(compiler, OP_GREATER); break;
        case TOKEN_GREATER_EQUAL: emit_bytes(compiler, OP_LESS, OP_NOT); break;
        case TOKEN_LESS:          emit_byte(compiler, OP_LESS); break;
        case TOKEN_LESS_EQUAL:    emit_bytes(compiler, OP_GREATER, OP_NOT); break;
        case TOKEN_PLUS: emit_byte(compiler, OP_ADD); break;
        case TOKEN_MINUS: emit_byte(compiler, OP_SUBTRACT); break;
        case TOKEN_STAR: emit_byte(compiler, OP_MULTIPLY); break;
        case TOKEN_SLASH: emit_byte(compiler, OP_DIVIDE); break;
        default: return; // Unreachable.
    }
}

bool compile(const char* source, Chunk* chunk)
{ 
    Compiler compiler;
    compiler.parser = (Parser){0};
    compiler.scanner = init_scanner(source);
    compiler.compiling_chunk = chunk;

    advance(&compiler);
    expression(&compiler);
    consume(&compiler, TOKEN_EOF, "Expect end of expression.");

    end_compiler(&compiler);

    return !compiler.parser.had_error;
}
