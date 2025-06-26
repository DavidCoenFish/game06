#pragma once

// Issue. Have paranoia about having all windows includes across translation units have the same defines/settings
// Solution. Have the windows include in a common file, dsc_common.h

typedef __int8 int8;
typedef unsigned __int8 uint8;
typedef __int16 int16;
typedef unsigned __int16 uint16;
typedef __int32 int32;
typedef unsigned __int32 uint32;
typedef __int64 int64;
typedef unsigned __int64 uint64;

#define LOG_TOPIC_DSC_COMMON "DSC_COMMON"

//FastBuild nmake workaround
// allow the IDE to get definitions, otherwise the Fastbuild definitions are defined in scripts and used as cmd line param to compiler and linker...
#ifndef DSC_BFF_BUILD
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef UNICODE
#define UNICODE
#endif

// reminder, this is just used in the IDE, otherwise defines are in the BFF script files for each platform configuration, see code\fbuild\dsc.bff
#define _DEBUG

#ifdef _DEBUG
	#define DSC_LOG
#endif

#endif //#ifndef DSC_BFF_BUILD

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
#include <shellscalingapi.h>
#include <shtypes.h>
#include <cassert>

//std lib
#include <any>
#include <array>
#include <chrono>
#include <cmath>
#include <codecvt>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

//D3DX12
#pragma warning( push )
#pragma warning( disable: 4265 ) // class has virtual functions, but its non-trivial destructor is not virtual
#pragma warning( disable: 4625 ) // copy constructor was implicitly defined as deleted
#pragma warning( disable: 4626 ) // assignment operator was implicitly defined as deleted
#pragma warning( disable: 4986 ) // exception specification does not match previous declaration
#pragma warning( disable: 5220 ) // a non-static data member with a volatile qualified
#pragma warning( disable: 5204 ) // class has virtual functions, but its trivial destructor is not virtual
#include <d3d12.h>
#include <d3dcommon.h>
#include <dxgi1_3.h>
#include <dxgi1_6.h>
#include <unknwn.h>
#include <wrl/client.h>
#include <wrl/event.h>
#ifdef _DEBUG
#include <dxgidebug.h>
#endif
// To use graphics and CPU markup events with the latest version of PIX, change this to include <pix3.h>
// then add the NuGet package WinPixEventRuntime to the project.
//#include <pix.h>
#pragma warning(pop)

//Macro
#define TOKEN_PAIR(TOKEN) TOKEN,#TOKEN
#define ENUM_TOKEN_PAIR(ENUM, TOKEN) ENUM::TOKEN,#TOKEN
#define DSC_CONDITION_THROW(CONDITION, MESSAGE) if (CONDITION) { throw std::exception(MESSAGE); }
#if defined(_DEBUG)
#define DSC_ASSERT(CONDITION, MESSAGE) if (false == (CONDITION)){ __debugbreak(); } assert(MESSAGE && (CONDITION))
#define DSC_ASSERT_ALWAYS(MESSAGE) __debugbreak(); assert(MESSAGE && false)
#define DSC_DEBUG_ONLY(PAYLOAD) PAYLOAD
#else
#define DSC_ASSERT(CONDITION, MESSAGE) __noop
#define DSC_ASSERT_ALWAYS(MESSAGE) __noop
#define DSC_DEBUG_ONLY(PAYLOAD)
#endif
#define DSC_TODO(MESSAGE, ...) DSC_ASSERT_ALWAYS(MESSAGE)

#define ARRAY_LITERAL_SIZE(DATA) (sizeof(DATA)/sizeof(DATA[0]))
#define DSC_COMMA ,

//FastBuild nmake workaround
// allow the IDE to get definitions, otherwise the include paths are defined in the BFF script outside awarness of the IDE
// added DSC_BFF_BUILD to fastbuild defines
#ifndef DSC_BFF_BUILD

#include "data_helper.h"
#include "enum_soft_bind.h"
#include "file_overlay_local.h"
#include "file_system.h"
#include "i_file_overlay.h"
#include "i_log_consumer.h"
#include "log_consumer_output_debug_string.h"
#include "log_consumer_std_out.h"
#include "log_system.h"
#include "math.h"
#include "math_angle.h"
#include "timer.h"
#include "utf8.h"
#include "vector_2.h"
#include "vector_4.h"
#include "vector_float2.h"
#include "vector_float4.h"
#include "vector_int2.h"
#include "vector_int4.h"

#endif //#ifndef DSC_BFF_BUILD

