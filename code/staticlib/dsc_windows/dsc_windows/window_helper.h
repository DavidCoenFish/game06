#pragma once
#include <dsc_common/dsc_common.h>

namespace DscWindows
{
	class IWindowApplication;

	//The return raw pointer IApplication is because ownership is placed under operating system SetWindowLongPtr(hWnd, GWLP_USERDATA,..)
	//typedef std::function< std::shared_ptr< IApplication > (const HWND hWnd, const IApplicationParam&) > TApplicationFactory;
	typedef std::function< IWindowApplication* const (const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight) > TWindowApplicationFactory;

	const bool UpdateApplication(const HWND in_hwnd);
	//IWindowApplication* GetWindowApplication(const HWND in_hwnd);
	//void SetWindowApplication(const HWND in_hwnd, IWindowApplication* const in_pWindowApplicaiton);
	//auto application = reinterpret_cast<DscWindows::IWindowApplication*>(GetWindowLongPtr(in_hwnd, GWLP_USERDATA));


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

} //DscWindows
