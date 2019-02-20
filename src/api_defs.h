/*
 * api_defs.h
 *
 *  Created on: 2011-02-02
 *      Author: randy
 */

#ifndef API_DEFS_H_
#define API_DEFS_H_


/**
 * \def DYNLIB_EXPORT
 * \brief A macro to export c-style api from shared library.
 */
#ifndef EXTERN_C
#define EXTERN_C extern "C"
#endif
/**
 * \def NOTHROW
 * \brief A macro defines that the procedure does not leak any exceptions outside.
 *
 * By encapsulating internal exceptions, all errors are either handled or returned as error code.
 */
#ifdef __cplusplus
#define NOTHROW throw()
#else
#define NOTHROW
#endif

//API is already obsoleted, will be deprecated in next major release
#ifndef OBSOLETE
#define OBSOLETE(ver)
#endif

#ifndef DYNLIB_EXPORT
#if defined(_MAC_)||defined(_LINUX_)||defined(_ANDROID_)
#define DYNLIB_EXPORT  __attribute__((visibility("default")))
#else
#ifdef _MSC_VER
#define DYNLIB_EXPORT  __declspec(dllexport)
#else
#error Plaform not defined!
//#define DYNLIB_EXPORT  __attribute__ ((dllexport))
#endif

#endif
#endif
/**
 * \def DYNLIB_IMPORT
 * \brief A macro to import c-style api from shared library.
 */
#ifndef DYNLIB_IMPORT
#if defined(_MAC_)||defined(_LINUX_)||defined(_ANDROID_)
#define DYNLIB_IMPORT
#else
#ifdef _MSC_VER
#define DYNLIB_IMPORT  __declspec(dllimport)
#else
#error Plaform not defined!
//#define DYNLIB_IMPORT  __attribute__((dllimport))
#endif
#endif
#endif
///**
// * \def STDCALL
// * \brief The windows-style API calling.
// */
//#ifndef STDCALL
//#define STDCALL __attribute__ ((stdcall))
//#endif

/**
 * The function never returns
 */
#ifndef NORETURN
#ifdef _MSC_VER
#define NORETURN __declspec(noreturn)
#else
#define NORETURN __attribute__((noreturn))
#endif

#endif

#ifndef NULL
#define NULL 0
#endif

/**
 * A internal tag that a method should not be subclassed.
 */
#ifndef FINAL
#define FINAL
#endif

#ifndef UNUSED
#define UNUSED(expr) do { (void)(expr); } while (0)
#endif

#endif /* API_DEFS_H_ */
