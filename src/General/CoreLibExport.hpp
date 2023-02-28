#pragma once

#ifdef CORELIB_STATIC_DEFINE
#  define CORE_LIB_API
#  define CORELIB_NO_EXPORT
#else
#  ifndef CORE_LIB_API
#    ifdef CoreLib_EXPORTS
        /* We are building this library */
#      define CORE_LIB_API __declspec(dllexport)
#    else
        /* We are using this library */
#      define CORE_LIB_API __declspec(dllimport)
#    endif
#  endif

#  ifndef CORELIB_NO_EXPORT
#    define CORELIB_NO_EXPORT 
#  endif
#endif

#ifndef CORELIB_DEPRECATED
#  define CORELIB_DEPRECATED __declspec(deprecated)
#endif

#ifndef CORELIB_DEPRECATED_EXPORT
#  define CORELIB_DEPRECATED_EXPORT CORE_LIB_API CORELIB_DEPRECATED
#endif

#ifndef CORELIB_DEPRECATED_NO_EXPORT
#  define CORELIB_DEPRECATED_NO_EXPORT CORELIB_NO_EXPORT CORELIB_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef CORELIB_NO_DEPRECATED
#    define CORELIB_NO_DEPRECATED
#  endif
#endif
