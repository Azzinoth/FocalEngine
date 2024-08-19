#pragma once

#ifdef FOCALENGINE_SHARED
    #ifdef FOCALENGINE_EXPORTS
        #define FOCALENGINE_API __declspec(dllexport)
    #else
        #define FOCALENGINE_API __declspec(dllimport)
    #endif
#else
    #define FOCALENGINE_API
#endif