/*
 * \file type_defs.h
 * \brief Common type definitions shared by the GS 5 project.
 *
 *  Created on: Feb 2, 2010
 *      Author: Randy
 */


#if !defined(TYPE_DEFS_H)
#define TYPE_DEFS_H

#include <cstring> //for size_t

#ifdef _MSC_VER
#include <cstdint>
#else
#include <stdint.h>
#endif

#include "api_defs.h"


#ifndef os_type_t
typedef unsigned int os_type_t;
#endif

const os_type_t OS_WIN= 0x0001;
const os_type_t OS_MAC = 0x0002;
const os_type_t OS_LINUX = 0x0004;

const os_type_t OS_SOLARIS = 0x0008;
const os_type_t OS_FREEBSD = 0x0010;
const os_type_t OS_ANDROID = 0x0020;

//The current OS we are running
#if defined(WIN32)||defined(_WIN32_)||defined(_WIN64_)||defined(_WIN64)
#define OS_CURRENT OS_WIN
#elif defined(_MAC_)
#define OS_CURRENT OS_MAC
#elif defined(_LINUX_)
#define OS_CURRENT OS_LINUX
#elif defined(_ANDROID_)||defined(__ANDROID__)||defined(ANDROID)
#define OS_CURRENT OS_ANDROID

#ifndef _ANDROID_
#define _ANDROID_
#endif

#else
#error Please specify your platform macro!
#endif

#ifndef cpu_arch_t
typedef unsigned int cpu_arch_t;
#endif

const cpu_arch_t CPU_ARCH_INTEL32 = 0x1;
const cpu_arch_t CPU_ARCH_INTEL64 = 0x2;
const cpu_arch_t CPU_ARCH_PPC32 = 0x4;
const cpu_arch_t CPU_ARCH_PPC64 = 0x8;

const cpu_arch_t CPU_ARCH_ALL = 0xffffffff;
const cpu_arch_t CPU_ARCH_VOID = 0x0;

/**
 * short-cut for all gs data types.
 */
typedef uint8_t xp_byte;
typedef int8_t xp_int8;
typedef uint16_t xp_word;
typedef int16_t xp_int16;
typedef uint32_t xp_dword;
typedef int32_t xp_int32;
typedef int64_t xp_int64;
typedef uint64_t xp_qword;
typedef float xp_float;
typedef double xp_double;
typedef uint8_t xp_bool;
typedef char xp_char;
typedef wchar_t xp_wchar;
typedef char* xp_string;
typedef wchar_t* xp_wstring;



#endif /* TYPE_DEFS_H_ */
