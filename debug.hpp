
#ifdef TRACE
    #define DEBUG
    #define LOG_T(x) do{std::cout << "TRACE: " << x << std::endl;}while(false);
#else
    #define LOG_T(x) do{} while(false);
#endif
#ifdef DEBUG
    #include <iostream>
    #define LOG_D(x) do{std::cout << "DEBUG: " << x << std::endl;}while(false);
#else
    #define LOG_D(x) do{} while(false);
#endif