#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <cstdint>

// TOKENS 
enum TokenType {
    TOK_LET, TOK_PRINT,
    TOK_IF, TOK_ELSE,
    TOK_FOR, TOK_DO, 

    TOK_IDENT, TOK_INT,

    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH,
    TOK_ASSIGN, TOK_EQUAL, TOK_SEMI,

    TOK_LPAREN, TOK_RPAREN,
    TOK_LBRACE, TOK_RBRACE,
    
    TOK_LESS, TOK_GREATER,

    TOK_WHILE,
    TOK_BREAK,
    TOK_CONTINUE,

    TOK_EOF
};

struct Token {
    TokenType type;
    std::string_view lexeme; // zero allocated string value
};

// OPCODES 
enum Opcode : uint8_t {
    OP_PUSH,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV,
    OP_LESS, OP_GREATER, OP_EQUAL,

    OP_STORE, OP_LOAD,
    OP_PRINT,

    OP_JUMP,
    OP_JUMP_IF_FALSE,

    OP_HALT
};

//  utility function to translate numerical opcodes to readable commands
inline const char* opcodeToString(uint8_t op) {
    switch (op) {
        case OP_PUSH:          return "OP_PUSH";
        case OP_ADD:           return "OP_ADD";
        case OP_SUB:           return "OP_SUB";
        case OP_MUL:           return "OP_MUL";
        case OP_DIV:           return "OP_DIV";
        case OP_LESS:          return "OP_LESS";
        case OP_GREATER:       return "OP_GREATER";
        case OP_EQUAL:         return "OP_EQUAL";
        case OP_STORE:         return "OP_STORE";
        case OP_LOAD:          return "OP_LOAD";
        case OP_PRINT:         return "OP_PRINT";
        case OP_JUMP:          return "OP_JUMP";
        case OP_JUMP_IF_FALSE: return "OP_JUMP_IF_FALSE";
        case OP_HALT:          return "OP_HALT";
        default:               return "OP_UNKNOWN";
    }
}
