/*scanner.c*/

//
// Scanner for nuPython programming language. The scanner reads the input
// stream and turns the characters into language Tokens, such as identifiers,
// keywords, and punctuation.
//
// <<Your Name>>
// Northwestern University
// CS 211
//
// Starter code: Prof. Joe Hummel
//

#include <assert.h>  // assert
#include <ctype.h>   // isspace, isdigit, isalpha
#include <stdbool.h> // true, false
#include <stdio.h>
#include <string.h> // strcmp

#include "scanner.h"
#include "util.h"

//
// collect_identifier
//
// Given the start of an identifier, collects the rest into value
// while advancing the column number.
//
static void collect_identifier(FILE *input, int c, int *colNumber,
                               char *value) {
  assert(isalpha(c) || c == '_'); // c should be start of identifier

  int i = 0;

  while (isalnum(c) || c == '_') // letter, digit, or underscore
  {
    value[i] = (char)c; // store char
    i++;

    (*colNumber)++; // advance col # past char

    c = fgetc(input); // get next char
  }

  // at this point we found the end of the identifer, so put
  // that last char back for processing later:
  ungetc(c, input);

  // turn the value into a string, and let's see if we have a keyword:
  value[i] = '\0'; // build C-style string:

  return;
}

//
// scanner_init
//
// Initializes line number, column number, and value before
// the start of the processing the next input stream.
//
void scanner_init(int *lineNumber, int *colNumber, char *value) {
  if (lineNumber == NULL || colNumber == NULL || value == NULL)
    panic("one or more parameters are NULL (scanner_init)");

  *lineNumber = 1;
  *colNumber = 1;
  value[0] = '\0'; // empty string
}

static int id_or_keyword(char *value) {
  assert(strlen(value) > 0);

  char *keywords[] = {"and",   "break", "continue", "def",    "elif", "else",
                      "False", "for",   "if",       "in",     "is",   "None",
                      "not",   "or",    "pass",     "return", "True", "while"};

  int index = -1; // assume we don't find value

  for (int i = 0; i < 18; i++) {
    if (strcmp(value, keywords[i]) == 0) { // compare each keyword to value
      index = i;
      break;
    }
  }

  if (index < 0) {
    return nuPy_IDENTIFIER;
  } else {
    return nuPy_KEYW_AND + index;
  }
}

static int int_or_real(char *value, bool pm) {
  int i = 0;
  if (pm) {
    i++;
  }
  while (isdigit(value[i]) || value[i] == '.') {
    if (value[i] == '.') {
      return nuPy_REAL_LITERAL;
    }
    i++;
  }
  return nuPy_INT_LITERAL;
}

static void collect_int_or_real_literal(FILE *input, int c, int *colNumber,
                                        char *value, bool pm) {
  int i = 0;
  bool hasDec = false;

  if (pm) {
    i++;
  }

  while (isdigit(c) || c == '.') {
    if (!hasDec && c == '.') {
      hasDec = true;
    } else if (hasDec && c == '.') {
      break;
    }
    value[i] = (char)c;
    i++;
    (*colNumber)++;

    c = fgetc(input);
  }

  ungetc(c, input);
  value[i] = '\0';
  return;
}

//
// scanner_nextToken
//
// Returns the next token in the given input stream, advancing the line
// number and column number as appropriate. The token's string-based
// value is returned via the "value" parameter. For example, if the
// token returned is an integer literal, then the value returned is
// the actual literal in string form, e.g. "123". For an identifer,
// the value is the identifer itself, e.g. "print" or "x". For a
// string literal such as 'hi there', the value is the contents of the
// string literal without the quotes.
//
struct Token scanner_nextToken(FILE *input, int *lineNumber, int *colNumber,
                               char *value) {
  if (input == NULL)
    panic("input stream is NULL (scanner_nextToken)");
  if (lineNumber == NULL || colNumber == NULL || value == NULL)
    panic("one or more parameters are NULL (scanner_nextToken)");

  struct Token T;

  //
  // repeatedly input characters one by one until a token is found:
  //
  while (true) {
    //
    // Get the next input character:
    //
    int c = fgetc(input);

    //
    // Let's see what we have...
    //

    if (c == EOF) // no more input, return EOS:
    {
      T.id = nuPy_EOS;
      T.line = *lineNumber;
      T.col = *colNumber;

      value[0] = '$';
      value[1] = '\0';

      return T;
    } else if (c == '$') // this is also EOS
    {
      T.id = nuPy_EOS;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; // advance col # past char

      value[0] = '$';
      value[1] = '\0';

      return T;
    } else if (c == '\n') // end of line, keep going:
    {
      (*lineNumber)++; // next line, restart column:
      *colNumber = 1;
      continue;
    } else if (isspace(c)) // other form of whitespace, skip:
    {
      (*colNumber)++; // advance col # past char
      continue;
    } else if (c == '(') {
      T.id = nuPy_LEFT_PAREN;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; // advance col # past char

      value[0] = (char)c;
      value[1] = '\0';

      return T;
    } else if (c == ')') {
      T.id = nuPy_RIGHT_PAREN;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; // advance col # past char

      value[0] = (char)c;
      value[1] = '\0';

      return T;
    } else if (isalpha(c) || c == '_') {
      //
      // start of identifier or keyword, let's assume identifier for now:
      //
      T.id = nuPy_IDENTIFIER;
      T.line = *lineNumber;
      T.col = *colNumber;

      collect_identifier(input, c, colNumber, value);

      T.id = id_or_keyword(value);

      return T;
    } else if (c == '"') {
      T.id = nuPy_STR_LITERAL;
      T.line = *lineNumber;
      T.col = *colNumber;
      int tempCol = *colNumber;

      int i = 0;
      c = fgetc(input);

      while (c != '"' && c != EOF) {
        if (c == '\n') {
          printf(
              "**WARNING: string literal @ (%d, %d) not terminated properly\n",
              *lineNumber, tempCol);
          (*lineNumber)++;
          *colNumber = -1;
          break;
        }

        value[i] = c;
        (*colNumber)++;
        c = fgetc(input);
        i++;
      }

      if (c == '"') {
        value[i] = '\0';
      }

      (*colNumber) += 2;

      return T;
    } else if (c == '\'') {
      T.id = nuPy_STR_LITERAL;
      T.line = *lineNumber;
      T.col = *colNumber;
      int tempCol = *colNumber;

      int i = 0;
      c = fgetc(input);

      while (c != '\'' && c != EOF) {
        if (c == '\n') {
          printf(
              "**WARNING: string literal @ (%d, %d) not terminated properly\n",
              *lineNumber, tempCol);
          (*lineNumber)++;
          *colNumber = 1;
          break;
        }
        value[i] = c;
        (*colNumber)++;
        c = fgetc(input);
        i++;
      }

      if (c == '\'') {
        value[i] = '\0';
      }
      (*colNumber) += 2;

      return T;
    } else if (isdigit(c)) {
      T.id = nuPy_INT_LITERAL;
      T.line = *lineNumber;
      T.col = *colNumber;

      collect_int_or_real_literal(input, c, colNumber, value, false);

      T.id = int_or_real(value, false);

      return T;

    } else if (c == '*') {
      //
      // could be * or **, let's assume * for now:
      //
      T.id = nuPy_ASTERISK;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; // advance col # past char

      value[0] = '*';
      value[1] = '\0';

      //
      // now let's read the next char and see what we have:
      //
      c = fgetc(input);

      if (c == '*') // it's **
      {
        T.id = nuPy_POWER;

        (*colNumber)++; // advance col # past char

        value[1] = '*';
        value[2] = '\0';

        return T;
      }

      ungetc(c, input); // not double **

      return T;
    } else if (c == '+') {
      T.id = nuPy_PLUS;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = '+';

      c = fgetc(input);

      if (isdigit(c) || c == '.') {

        collect_int_or_real_literal(input, c, colNumber, value, true);

        T.id = int_or_real(value, true);

        return T;
      }

      ungetc(c, input);
      value[1] = '\0';

      return T;

    } else if (c == '-') {
      T.id = nuPy_MINUS;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = '-';

      c = fgetc(input);

      if (isdigit(c) || c == '.') {

        collect_int_or_real_literal(input, c, colNumber, value, true);

        T.id = int_or_real(value, true);

        return T;
      }

      ungetc(c, input);
      value[1] = '\0';

      return T;

    } else if (c == '/') {
      T.id = nuPy_SLASH;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = '/';
      value[1] = '\0';

      return T;

    } else if (c == '=') {
      T.id = nuPy_EQUAL;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = '=';
      value[1] = '\0';

      c = fgetc(input);
      if (c == '=') {
        T.id = nuPy_EQUALEQUAL;

        (*colNumber)++;

        value[1] = '=';
        value[2] = '\0';

        return T;
      }
      ungetc(c, input);
      return T;

    } else if (c == '!') {
      T.id = nuPy_UNKNOWN;
      T.line = *lineNumber;
      T.col = *colNumber;
      value[0] = (char)c;
      value[1] = '\0';

      c = fgetc(input);
      if (c == '=') {
        T.id = nuPy_NOTEQUAL;
        (*colNumber) += 2;
        value[1] = '=';
        value[2] = '\0';

        return T;
      } else {
        ungetc(c, input);
      }
      (*colNumber)++;
      return T;

    } else if (c == '<') {
      T.id = nuPy_LT;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = '<';
      value[1] = '\0';

      c = fgetc(input);
      if (c == '=') {
        T.id = nuPy_LTE;

        (*colNumber)++;

        value[1] = '=';
        value[2] = '\0';

        return T;
      }

      ungetc(c, input);
      return T;

    } else if (c == '>') {
      T.id = nuPy_GT;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = '>';
      value[1] = '\0';

      c = fgetc(input);
      if (c == '=') {
        T.id = nuPy_GTE;

        (*colNumber)++;

        value[1] = '=';
        value[2] = '\0';

        return T;
      }
      ungetc(c, input);
      return T;

    } else if (c == '&') {
      T.id = nuPy_AMPERSAND;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = '&';
      value[1] = '\0';

      return T;
    } else if (c == ':') {
      T.id = nuPy_COLON;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = ':';
      value[1] = '\0';

      return T;
    } else if (c == '[') {
      T.id = nuPy_LEFT_BRACKET;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = '[';
      value[1] = '\0';

      return T;

    } else if (c == ']') {
      T.id = nuPy_RIGHT_BRACKET;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = ']';
      value[1] = '\0';

      return T;

    } else if (c == '{') {
      T.id = nuPy_LEFT_BRACE;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = '{';
      value[1] = '\0';

      return T;

    } else if (c == '}') {
      T.id = nuPy_RIGHT_BRACE;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = '}';
      value[1] = '\0';

      return T;

    } else if (c == '%') {
      T.id = nuPy_PERCENT;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = '%';
      value[1] = '\0';

      return T;

    } else if (c == '#') {
      while (c != '\n' && c != EOF) {
        c = fgetc(input);
        (*colNumber)++;
      }
      (*lineNumber)++;
      *colNumber = 1;
      continue;
    } else {
      //
      // if we get here, then char denotes an UNKNOWN token:
      //
      T.id = nuPy_UNKNOWN;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; // advance past char

      value[0] = (char)c;
      value[1] = '\0';

      return T;
    }

  } // while

  //
  // execution should never get here, return occurs
  // from within loop
  //
}
