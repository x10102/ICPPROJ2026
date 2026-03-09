
#ifdef TRACE
    #define DEBUG
    #define LOG_T(x, ...) do{fputs("TRACE: ", stderr);fprintf(stderr, x, ##__VA_ARGS__);fputs("\n", stderr);}while(false);
#else
    #define LOG_T(x) do{} while(false);
#endif
#ifdef DEBUG
    #include <cstdio>
    #define LOG_D(x, ...) do{fputs("DEBUG: ", stderr);fprintf(stderr, x, ##__VA_ARGS__);fputs("\n", stderr);}while(false);
#else
    #define LOG_D(x) do{} while(false);
#endif