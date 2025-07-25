#include "test_util.h"
#include <dsc_common/dsc_common.h>
#include <dsc_windows/window_helper.h>

namespace
{
	std::vector<TestUtil::TWindowApplicationFactory>& GetTestArray()
	{
		static std::vector<TestUtil::TWindowApplicationFactory> sTestArray;
		return sTestArray;
	}
};

const bool TestUtil::AlmostEqual(const bool in_ok, const float in_valueLhs, const float in_valueRhs, const std::string& in_fileName, const int in_lineNumber)
{
	DSC_UNUSED(in_fileName);
	DSC_UNUSED(in_lineNumber);
	if (std::numeric_limits<float>::epsilon() < std::abs(in_valueLhs - in_valueRhs))
	{
		DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "AlmostEqual failed, lhs:%f rhs:%f file:%s line:%d\n", in_valueLhs, in_valueRhs, in_fileName.c_str(), in_lineNumber);
		return false;
	}
	return in_ok;
}

const bool TestUtil::EqualString(const bool in_ok, const std::string& in_valueLhs, const std::string& in_valueRhs, const std::string& in_fileName, const int in_lineNumber)
{
	DSC_UNUSED(in_fileName);
	DSC_UNUSED(in_lineNumber);
	if (in_valueLhs != in_valueRhs)
	{
		DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "EqualString failed, lhs:%s rhs:%s file:%s line:%d\n", in_valueLhs.c_str(), in_valueRhs.c_str(), in_fileName.c_str(), in_lineNumber);
		return false;
	}
	return in_ok;
}

void TestUtil::AddTest(const TWindowApplicationFactory& in_test)
{
	GetTestArray().push_back(in_test);
}

const bool TestUtil::RunTests(
	_In_ HINSTANCE in_hInstance,
	int in_nCmdShow
	)
{
	int32 count_total = 0;
	int32 count_pass = 0;
	for(const auto& item : GetTestArray())
	{
		try
		{
			const HWND hwnd = DscWindows::WindowHelper(
				item,
				800,
				600,
				false,
				in_hInstance,
				"RenderUnitTest",
				in_nCmdShow
				);

			bool ok = false;
			while (true)
			{
				MSG msg = {};
				if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					if (WM_QUIT == msg.message)
					{
						ok = (1 == (int)msg.wParam);
						break;
					}
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					DscWindows::UpdateApplication(hwnd);
				}
			}

			if (true == ok)
			{
				count_pass += 1;
			}
		}
		catch(...)
		{
			DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "exception thrown\n");
		}
		count_total += 1;
	}

	DSC_LOG_INFO(LOG_TOPIC_APPLICATION, "UNIT TEST: Passed:%d of tests:%d\n", count_pass, count_total);
	// give output debug string a second to cominicate with debugger (if present, then let app shutdown)
	Sleep(1000);
	return (count_total == count_pass);
}
