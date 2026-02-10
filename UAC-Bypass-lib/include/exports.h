#pragma once
#include <windows.h>

#ifdef _WIN32
    #ifdef MY_PROJECT_EXPORTS
        #define MY_PROJECT_API extern "C" __declspec(dllexport)
    #else
        #define MY_PROJECT_API extern "C" __declspec(dllimport)
    #endif
#else
    #define MY_PROJECT_API
#endif

MY_PROJECT_API void Example();
