#pragma once

enum {
    BEGIN = 0x10000000,
    IF,
    THEN,
    ELSE,
    REPEAT,
    UNTIL,
    WHILE,
    DO,
    OR,
    DIV,
    MOD,
    AND,
	TRUE,
	FALSE,
    END,
};

extern int isKeyword(const char*);
