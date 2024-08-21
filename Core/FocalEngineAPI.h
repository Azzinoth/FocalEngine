#pragma once

#ifdef FOCAL_ENGINE_SHARED
    #ifdef FOCAL_ENGINE_EXPORTS
        #define FOCAL_ENGINE_API __declspec(dllexport)
    #else
        #define FOCAL_ENGINE_API __declspec(dllimport)
    #endif
#else
    #define FOCAL_ENGINE_API
#endif