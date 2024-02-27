#define DECLSPEC_DEPRECATED_DDK

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NON_CONFORMING_SWPRINTFS
#define _NO_CRT_STDIO_INLINE
#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 0
#define _ALLOW_COMPILER_AND_STL_VERSION_MISMATCH
#define __EDG__
#define USE_ATL_THUNK2
#pragma warning(disable : 4073 4074 4075 4097 4514 4005 4200 4201 4238 4307 4324 4480 4530 4706 5040)
#include <stdlib.h>
//#include <wchar.h>
#include <stdio.h>
#include <string.h>

#include <Windows.h>
#pragma warning(disable : 4392)
#include <intrin.h>
#pragma warning(default : 4392)
#include <CommCtrl.h>
#include <windowsx.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;