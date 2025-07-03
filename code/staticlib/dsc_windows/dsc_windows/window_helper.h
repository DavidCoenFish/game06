#pragma once
#include "dsc_windows.h"
#include <dsc_common/dsc_common.h>

namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
}

namespace DscWindows
{
	class IWindowApplication;

	//The return raw pointer IApplication is because ownership is placed under operating system SetWindowLongPtr(hWnd, GWLP_USERDATA,..)
	typedef std::function< IWindowApplication* const (const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight) > TWindowApplicationFactory;

	const bool UpdateApplication(const HWND in_hwnd);

	// return the handel of the created window, or NULL on error
	const HWND WindowHelper(
		const TWindowApplicationFactory& in_application_factory,
		const int32 in_defaultWidth,
		const int32 in_defaultHeight,
		const bool in_fullScreen,
		HINSTANCE in_instance,
		const std::string& in_application_name,
		const int in_cmd_show
	);

	// top left is 0,0 and +x is across the screen, +y is down the screen
	const bool GetMouseState(
		const HWND in_hwnd,
		DscCommon::VectorInt2& out_pos,
		bool& out_left_button,
		bool& out_right_button
	);

} //DscWindows
