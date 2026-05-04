// debug.hpp - debug logging macros
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>
#include <cstdio>

#ifndef _DEBUG_H
#define _DEBUG_H

#define LOG_I(x, ...) do{fputs("LOG: ", stderr);fprintf(stderr, x, ##__VA_ARGS__);fputs("\n", stderr);}while(false);

#ifdef TRACE
    #define DEBUG
    #define LOG_T(x, ...) do{fputs("TRACE: ", stderr);fprintf(stderr, x, ##__VA_ARGS__);fputs("\n", stderr);}while(false);
#else
    #define LOG_T(x, ...) do{} while(false);
#endif
#ifdef DEBUG
    #define LOG_D(x, ...) do{fputs("DEBUG: ", stderr);fprintf(stderr, x, ##__VA_ARGS__);fputs("\n", stderr);}while(false);
#else
    #define LOG_D(x, ...) do{} while(false);
#endif
#endif