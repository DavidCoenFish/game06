#pragma once

// Issue. Have paranoia about having all windows includes across translation units have the same defines/settings
// Solution. Have the windows include in a common file, common.h

typedef __int8 int8;
typedef unsigned __int8 uint8;
typedef __int16 int16;
typedef unsigned __int16 uint16;
typedef __int32 int32;
typedef unsigned __int32 uint32;
typedef __int64 int64;
typedef unsigned __int64 uint64;

// Windows
#include <winsdkver.h>
#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>
#define NOMINMAX // Use the C++ standard templated min/max
#define NODRAWTEXT
#define NOGDI // DirectX apps don't need GDI
#define NOBITMAP
#define NOMCX // Include <mcx.h> if you need this
#define NOSERVICE // Include <winsvc.h> if you need this
#define NOHELP // WinHelp is deprecated
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cassert>

//std lib
#include <cmath>
#include <codecvt>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iostream>
#include <limits>
#include <locale>
#include <string>
#include <vector>
#include <map>

//Macro
#define TOKEN_PAIR(TOKEN) TOKEN,#TOKEN
#define ENUM_TOKEN_PAIR(ENUM, TOKEN) ENUM::TOKEN,#TOKEN
#define DSC_CONDITION_THROW(CONDITION, MESSAGE) if (CONDITION) { throw std::exception(MESSAGE); }
#if defined(_DEBUG)
   #define DSC_ASSERT(CONDITION, MESSAGE) assert(MESSAGE && (CONDITION))
   #define DSC_ASSERT_ALWAYS(MESSAGE) assert(MESSAGE && false)
#else
   #define DSC_ASSERT(CONDITION, MESSAGE) __noop
   #define DSC_ASSERT_ALWAYS __noop
#endif
#define DSC_TODO(MESSAGE, ...) DSC_ASSERT_ALWAYS(MESSAGE)
