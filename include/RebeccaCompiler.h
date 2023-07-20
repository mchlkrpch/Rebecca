/**
 * @file RebeccaCompiler.h
 * Main compiler logic of Rebecca-language.
 * 
 * Provides
 *   -main constants,
 *   -token identifiers,
 *   -token text representation,
 *   -all functions of each part of compilation (from frontend to binary translation)
 */
#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Global constants --------------------------------------------------------------------------

/// Maximal token length in characters.
#define kTokenMaxLen 256

/// Maximal local variables in scope restriction.
static const uint64_t kMaxLocals     = 256;
/// Maximal scope layers in program to restrict memory consuming.
static const uint64_t kMaxScopeDepth = 256;
/// Maximal scope layers in program.
static const uint64_t kInitSequenceSize = 256;

/**
 * @brief Logically different token types.
 * 
 * @note For example '|' and '||' is one logical
 * token and they have different meanings.
 */
typedef enum
{
  TOKEN_UNKNOWN,
  TOKEN_LEFT_PARENTHESIS,   // '('
  TOKEN_RIGHT_PARENTHESIS,  // ')'
  TOKEN_LEFT_BRACKET,       // '['
  TOKEN_RIGHT_BRACKET,      // ']'
  TOKEN_LEFT_BRACE,         // '{'
  TOKEN_RIGHT_BRACE,        // '}'
  TOKEN_COLON,              // ':'
  TOKEN_DOT,                // '.'
  TOKEN_COMMA,              // ','
  TOKEN_STAR,               // '*'
  TOKEN_SLASH,              // '/'
  TOKEN_BACK_SLASH,         // '\'
  TOKEN_PERCENT,            // '%'
  TOKEN_HASHTAG,            // '#'
  TOKEN_PLUS,               // '+'
  TOKEN_PLUSPLUS,           // '++'
  TOKEN_MINUS,              // '-'
  TOKEN_LL,                 // '<<'
  TOKEN_GG,                 // '>>'
  TOKEN_PIPE,               // '|'
  TOKEN_PIPEPIPE,           // '||'
  TOKEN_CARET,              // '^'
  TOKEN_TILDE,              // '~'
  TOKEN_QUESTION,           // '?'
  TOKEN_EXCLAMATION,        // '!'
  TOKEN_EQ,                 // '='
  TOKEN_L,                  // '<'
  TOKEN_G,                  // '>'
  TOKEN_LEQ,                // '<='
  TOKEN_GEQ,                // '>='
  TOKEN_EQEQ,               // '=='
  TOKEN_EXCLAMATION_EQ,     // '!='
  TOKEN_BREAK,              // 'break'
  TOKEN_CONTINUE,           // 'continue'
  TOKEN_CLASS,              // 'class'
  TOKEN_STRUCT,             // 'struct'
  TOKEN_ELSE,               // 'else'
  TOKEN_FALSE,              // 'false'
  TOKEN_CYCLE,              // 'for'
  TOKEN_IF,                 // 'if'
  TOKEN_LOAD,               // 'load'
  TOKEN_NULL,               // 'nullptr'
  TOKEN_RETURN,             // 'ret'
  TOKEN_STATIC,             // 'static'
  TOKEN_THIS,               // 'this'
  TOKEN_TRUE,               // 'true'
  TOKEN_PRIVATE,            // 'private'
  TOKEN_PUBLIC,             // 'public'
  TOKEN_NAME,               // any name of variavle/function/class/...
  TOKEN_NUMBER,             // any number for example: 123
  TOKEN_COMP,               // '<=>'

  TOKEN_UNDERLINE,          // '_'
  
  TOKEN_EOF                 // end of file
} TokenType;

const char *TranslateTokenType(TokenType type);

// Lexer part --------------------------------------------------------------------------------
/**
 * @addtogroup Lexer
 * Simple Lexer realization.
 * @{
 */

typedef struct
{
  const char *txt;
  size_t      len;
  TokenType   type;
} StableWord;

static const char  *kEofTokenTxt = "EOF";
static const size_t kEofTokenLength = 3;

static StableWord stable_words[] =
{
  {"return",    6, TOKEN_RETURN},
  {"break",     5, TOKEN_BREAK},
  {"continue",  8, TOKEN_CONTINUE},
  {"class",     5, TOKEN_CLASS},
  {"_else",     5, TOKEN_ELSE},
  {"false",     5, TOKEN_FALSE},
  {"cycle",     5, TOKEN_CYCLE},
  {"if",        2, TOKEN_IF},
  {"load",      4, TOKEN_LOAD},
  {"null",      4, TOKEN_NULL},
  {"public",    6, TOKEN_PUBLIC},
  {"private",   7, TOKEN_PRIVATE},
  {"ret",       3, TOKEN_RETURN},
  {"static",    6, TOKEN_STATIC},
  {"this",      4, TOKEN_THIS},
  {"true",      4, TOKEN_TRUE},
  {"<<",        2, TOKEN_LL},
  {"<=>",       3, TOKEN_COMP},
  {">>",        2, TOKEN_GG},
  {"<=",        2, TOKEN_LEQ},
  {">=",        2, TOKEN_GEQ},
  {"=",         1, TOKEN_EQ},
  {"[",         1, TOKEN_LEFT_BRACKET},
  {"]",         1, TOKEN_RIGHT_BRACKET},
  {"(",         1, TOKEN_LEFT_PARENTHESIS},
  {")",         1, TOKEN_RIGHT_PARENTHESIS},
  {"{",         1, TOKEN_LEFT_BRACE},
  {"}",         1, TOKEN_RIGHT_BRACE},
  {"*",         1, TOKEN_STAR},
  {"+",         1, TOKEN_PLUS},
  {"++",        1, TOKEN_PLUSPLUS},
  {"-",         1, TOKEN_MINUS},
  {"_",         1, TOKEN_UNDERLINE},
  {",",         1, TOKEN_COMMA},
  {".",         1, TOKEN_DOT},
  {"%",         1, TOKEN_PERCENT},
  {"#",         1, TOKEN_HASHTAG},
  {"/",         1, TOKEN_SLASH},
  {"EOF",       3, TOKEN_EOF}
};

// Split symbols to split words in source code.
static const char *kSplitSymbols = "()[]{}:;.,*/\\%#+-<>|^~?!=!";
// Whitespace symbols in source code.
static const char *kWhiteSpace   = " \n\t";
// Digit symbols in source code.
static const char *kDigitSymbols = "01234567890";

/**
 * @addtogroup Variables
 * @{
 */
typedef struct
{
  int val;
} Value;

typedef enum
{
  // Local scope.
  SCOPE_LOCAL,
  // Upper scope.
  SCOPE_UPPER,
  // Global scope.
  SCOPE_GLOBAL
} Scope;

/**
 * @}
 */

typedef struct
{
  // Type of token.
  TokenType type;
  char *txt;
  // Static value (if exists).
  Value     value;
} Token;

typedef struct
{
  const char *name;
  uint64_t *depth;
} Context;

// Check src/Tokenizer.c
Token *Tokenizer(const char *name, uint64_t *n_tokens);

/**
 * @}
 */

// AST part ----------------------------------------------------------------------------------
/**
 * @brief parser machine.
 * Uses Token 'sequence' and build the AST.
 */
typedef struct
{
  // Tokens from tokenizer.
  Token *sequence;
  // Token size.
  uint64_t sequence_size;
  // Current context on each token.
  Context ctx;
} Parser;

// Check src/Parser.c
Parser *BuildAst(Token *sequence, uint64_t n_tokens);

#pragma GCC diagnostic pop
