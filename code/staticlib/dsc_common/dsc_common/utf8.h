#pragma once

#include <dsc_common\dsc_common.h>

namespace DscCommon
{
	namespace Utf8
	{
		const std::wstring Utf8ToUtf16(const std::string& in_utf8);
		const std::string Utf16ToUtf8(const std::wstring& in_utf16);

	} //Utf8
} //DscCommon
