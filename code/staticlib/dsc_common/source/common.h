// Issue. Have paranoia about having all windows includes across translation units have the same defines/settings
// Solution. Have the windows include in a common file, common.h

#pragma once




//#include <winsdkver.h>
//#define _WIN32_WINNT 0x0A00
//#include <sdkddkver.h>
//
//// Use the C++ standard templated min/max
//#define NOMINMAX
//
//// DirectX apps don't need GDI
//#define NODRAWTEXT
//#define NOGDI
//#define NOBITMAP
//
//// Include <mcx.h> if you need this
//#define NOMCX
//
//// Include <winsvc.h> if you need this
//#define NOSERVICE
//
//// WinHelp is deprecated
//#define NOHELP
//
//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>
//
//#include <wrl/client.h>
//#include <wrl/event.h>
//
//#include <d3d12.h>
//#include <dxgi1_3.h>
//#include <dxgi1_6.h>
//
//#include <DirectXMath.h>
//#include <DirectXColors.h>
//
//// Std library includes
//#include <algorithm>
//#include <array>
//#include <bitset>
//#include <chrono>
////#include <cstdint>
//#include <exception>
//#include <filesystem>
//#include <fstream>
//#include <functional>
//#include <future>
//#include <map>
//#include <memory>
//#include <mutex>
//#include <set>
////#include <stdexcept>
//#include <shared_mutex>
//#include <sstream>
//#include <string>
//#include <variant>
//#include <vector>
//
//#ifdef _DEBUG
//#include <dxgidebug.h>
//#endif
//
//#include <stdio.h>
//#include <assert.h>
//
//
//// To use graphics and CPU markup events with the latest version of PIX, change this to include <pix3.h>
//// then add the NuGet package WinPixEventRuntime to the project.
//#include <pix.h>

