/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
/*****************************************************************************************
 ******************** This file was generated by sksllex. Do not edit. *******************
 *****************************************************************************************/
#ifndef SKSL_Lexer
#define SKSL_Lexer
#include <cstddef>
#include <cstdint>
#include "include/core/SkStringView.h"
namespace SkSL {

struct Token {
    enum class Kind {
        TK_END_OF_FILE,
        TK_FLOAT_LITERAL,
        TK_INT_LITERAL,
        TK_BAD_OCTAL,
        TK_TRUE_LITERAL,
        TK_FALSE_LITERAL,
        TK_IF,
        TK_STATIC_IF,
        TK_ELSE,
        TK_FOR,
        TK_WHILE,
        TK_DO,
        TK_SWITCH,
        TK_STATIC_SWITCH,
        TK_CASE,
        TK_DEFAULT,
        TK_BREAK,
        TK_CONTINUE,
        TK_DISCARD,
        TK_RETURN,
        TK_IN,
        TK_OUT,
        TK_INOUT,
        TK_UNIFORM,
        TK_CONST,
        TK_FLAT,
        TK_NOPERSPECTIVE,
        TK_INLINE,
        TK_NOINLINE,
        TK_HASSIDEEFFECTS,
        TK_STRUCT,
        TK_LAYOUT,
        TK_HIGHP,
        TK_MEDIUMP,
        TK_LOWP,
        TK_ES3,
        TK_RESERVED,
        TK_IDENTIFIER,
        TK_DIRECTIVE,
        TK_LPAREN,
        TK_RPAREN,
        TK_LBRACE,
        TK_RBRACE,
        TK_LBRACKET,
        TK_RBRACKET,
        TK_DOT,
        TK_COMMA,
        TK_PLUSPLUS,
        TK_MINUSMINUS,
        TK_PLUS,
        TK_MINUS,
        TK_STAR,
        TK_SLASH,
        TK_PERCENT,
        TK_SHL,
        TK_SHR,
        TK_BITWISEOR,
        TK_BITWISEXOR,
        TK_BITWISEAND,
        TK_BITWISENOT,
        TK_LOGICALOR,
        TK_LOGICALXOR,
        TK_LOGICALAND,
        TK_LOGICALNOT,
        TK_QUESTION,
        TK_COLON,
        TK_EQ,
        TK_EQEQ,
        TK_NEQ,
        TK_GT,
        TK_LT,
        TK_GTEQ,
        TK_LTEQ,
        TK_PLUSEQ,
        TK_MINUSEQ,
        TK_STAREQ,
        TK_SLASHEQ,
        TK_PERCENTEQ,
        TK_SHLEQ,
        TK_SHREQ,
        TK_BITWISEOREQ,
        TK_BITWISEXOREQ,
        TK_BITWISEANDEQ,
        TK_SEMICOLON,
        TK_ARROW,
        TK_WHITESPACE,
        TK_LINE_COMMENT,
        TK_BLOCK_COMMENT,
        TK_INVALID,
        TK_NONE,
    };

    Token() {}
    Token(Kind kind, int32_t offset, int32_t length, int32_t line)
            : fKind(kind), fOffset(offset), fLength(length), fLine(line) {}

    Kind fKind = Kind::TK_NONE;
    int32_t fOffset = -1;
    int32_t fLength = -1;
    int32_t fLine = -1;
};

class Lexer {
public:
    void start(skstd::string_view text) {
        fText = text;
        fOffset = 0;
        fLine = 1;
    }

    Token next();

    struct Checkpoint {
        int32_t fOffset;
        int32_t fLine;
    };

    Checkpoint getCheckpoint() const { return {fOffset, fLine}; }

    void rewindToCheckpoint(Checkpoint checkpoint) {
        fOffset = checkpoint.fOffset;
        fLine = checkpoint.fLine;
    }

private:
    skstd::string_view fText;
    int32_t fOffset;
    int32_t fLine;
};

}  // namespace SkSL
#endif
