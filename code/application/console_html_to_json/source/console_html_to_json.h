#pragma once

//#include <Windows.h> // for WideCharToMultiByte

#include <set>
#include <string>
#include <map>
#include <vector>

#pragma warning(push)

#pragma warning(disable : 4514)  // unreferenced inline function has been removed
#pragma warning(disable : 4623)  // default constructor was implicitly defined as deleted
#pragma warning(disable : 4625)  // copy constructor was implicitly defined as deleted
#pragma warning(disable : 4626)  // assignment operator was implicitly defined as deleted
#pragma warning(disable : 5027)  // move assignment operator was implicitly defined as deleted

#include <nlohmann\json.hpp>

#pragma warning(pop)

// trying to get visual studio to see settings
#ifndef DSC_BFF_BUILD

//G:\development\game06\code\application\console_html_to_json\source
//G:\development\game06\code\sdk\nlohmann-json_x64-windows\include

#include "..\..\..\sdk\nlohmann-json_x64-windows\include\nlohmann\json.hpp"

#endif //#ifndef DSC_BFF_BUILD


