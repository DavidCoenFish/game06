#pragma once
#include "render_unit_test.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/log_system.h>

namespace DscWindows
{
	class IWindowApplication;
}

namespace TestUtil
{
	template<typename TYPE>
	const bool Equal(const bool in_ok, const TYPE in_valueLhs, const TYPE in_valueRhs, const std::string& in_fileName, const int in_lineNumber)
	{
		DSC_UNUSED(in_fileName);
		DSC_UNUSED(in_lineNumber);
		if (in_valueLhs != in_valueRhs)
		{
			DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "Equal failed file:%s line:%d\n", in_fileName.c_str(), in_lineNumber);
			return false;
		}
		return in_ok;
	}

	template<typename TYPE>
	const bool EqualVector(const bool in_ok, const std::vector<TYPE>& in_valueLhs, const std::vector<TYPE>& in_valueRhs, const std::string& in_fileName, const int in_lineNumber)
	{
		DSC_UNUSED(in_fileName);
		DSC_UNUSED(in_lineNumber);
		bool match = false;
		if (in_valueLhs.size() == in_valueRhs.size())
		{
			match = true;
			for (size_t index = 0; index < in_valueRhs.size(); ++index)
			{
				if (in_valueLhs[index] != in_valueRhs[index])
				{
					match = false;
					DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "Equal Vector failed file:%s line:%d\n", in_fileName.c_str(), in_lineNumber);
					break;
				}
			}
		}
		else
		{
			DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "Equal Vector failed lhs size:%d rhs size:%d file:%s line:%d\n", in_valueLhs.size(), in_valueRhs.size(), in_fileName.c_str(), in_lineNumber);
		}

		if (false == match)
		{
			return false;
		}
		return in_ok;
	}

	template<typename TYPE>
	const bool NotEqual(const bool in_ok, const TYPE in_valueLhs, const TYPE in_valueRhs, const std::string& in_fileName, const int in_lineNumber)
	{
		DSC_UNUSED(in_fileName);
		DSC_UNUSED(in_lineNumber);
		if (in_valueLhs == in_valueRhs)
		{
			DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "Not Equal failed file:%s line:%d\n", in_fileName.c_str(), in_lineNumber);
			return false;
		}
		return in_ok;
	}

	const bool AlmostEqual(const bool in_ok, const float in_valueLhs, const float in_valueRhs, const std::string& in_fileName, const int in_lineNumber);
	const bool EqualString(const bool in_ok, const std::string& in_valueLhs, const std::string& in_valueRhs, const std::string& in_fileName, const int in_lineNumber);

	typedef std::function< DscWindows::IWindowApplication* const (const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight) > TWindowApplicationFactory;
	void AddTest(const TWindowApplicationFactory& in_test);

	const bool RunTests(
		HINSTANCE in_hInstance,
		int in_nCmdShow
	);
}

#define TEST_UTIL_EQUAL(OK, LHS, RHS) TestUtil::Equal(OK, LHS, RHS, __FILE__, __LINE__);
#define TEST_UTIL_NOT_EQUAL(OK, LHS, RHS) TestUtil::NotEqual(OK, LHS, RHS, __FILE__, __LINE__);
#define TEST_UTIL_ALMOST_EQUAL(OK, LHS, RHS) TestUtil::AlmostEqual(OK, LHS, RHS, __FILE__, __LINE__);
#define TEST_UTIL_EQUAL_VECTOR(OK, LHS, RHS) TestUtil::EqualVector(OK, LHS, RHS, __FILE__, __LINE__);
#define TEST_UTIL_EQUAL_STRING(OK, LHS, RHS) TestUtil::EqualString(OK, LHS, RHS, __FILE__, __LINE__);

