#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_int2.h>
#include <dsc_common/log_system.h>
#include <dsc_common/utf8.h>
#include "window_helper.h"
#include "i_window_application.h"

namespace
{
	typedef std::function< DscWindows::IWindowApplication* (const HWND in_hwnd)> TCreateFunction;

	void SetWindowApplication(const HWND in_hwnd, DscWindows::IWindowApplication* const in_pWindowApplicaiton)
	{
		SetWindowLongPtr(in_hwnd, GWLP_USERDATA, (LONG_PTR)in_pWindowApplicaiton);
	}

	DscWindows::IWindowApplication* const GetWindowApplication(const HWND in_hwnd)
	{
		auto application = reinterpret_cast<DscWindows::IWindowApplication* const>(GetWindowLongPtr(in_hwnd, GWLP_USERDATA));
		return application;
	}

	const float GetMonitorScale(HWND in_hwnd)
	{
		HMONITOR mon = MonitorFromWindow(
			in_hwnd,
			MONITOR_DEFAULTTONEAREST
		);

		UINT dpiX = 0, dpiY = 0;
		HRESULT result = GetDpiForMonitor(mon, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
		if (S_OK != result)
		{
			DSC_LOG_WARNING(LOG_TOPIC_DSC_WINDOWS, "GetDpiForMonitor FAILED %d\n", result);
		}
		//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_WINDOWS, "GetDpiForMonitor %d %d\n", dpiX, dpiY);
		const float scale = static_cast<float>(dpiX + dpiY) / (96.0f + 96.0f);
		return scale;
	}

	// Windows procedure
	LRESULT CALLBACK WndProc(HWND in_hwnd, UINT in_message, WPARAM in_wparam, LPARAM in_lparam)
	{
		// can be null before WM_CREATE
		auto application = GetWindowApplication(in_hwnd);

		switch (in_message)
		{
		default:
			return DefWindowProc(in_hwnd, in_message, in_wparam, in_lparam);

		case WM_CREATE:
			{
				auto data = (LPCREATESTRUCTA)(in_lparam);
				TCreateFunction* create_function = data ? (TCreateFunction*)(data->lpCreateParams) : nullptr;
				if (create_function)
				{
					auto application_new = (*create_function)(in_hwnd);
					SetWindowApplication(in_hwnd, application_new);
				}
			}
			break;

		case WM_DESTROY:
			{
				int32 exit_code = 0;
				if (nullptr != application)
				{
					exit_code = application->GetExitCode();
					delete application;
				}
				SetWindowApplication(in_hwnd, nullptr);
				//WM_QUIT is never sent to window, but you can pull it out of the GetMessage/PeekMessage
				PostQuitMessage(exit_code);
			}
			break;

		// custom show window to avoid a frame of white, possibly a time slice between window shown and erase/paint being called
		//https://stackoverflow.com/questions/69715610/how-to-initialize-the-background-color-of-win32-app-to-something-other-than-whit
		case WM_SHOWWINDOW:
			{
				if (!GetLayeredWindowAttributes(in_hwnd, NULL, NULL, NULL))
				{
					SetLayeredWindowAttributes(in_hwnd, 0, 0, LWA_ALPHA);
					DefWindowProc(in_hwnd, WM_ERASEBKGND, (WPARAM)GetDC(in_hwnd), in_lparam);
					SetLayeredWindowAttributes(in_hwnd, 0, 255, LWA_ALPHA);
					AnimateWindow(in_hwnd, 1, AW_ACTIVATE | AW_BLEND);
					return 0;
				}
				return DefWindowProc(in_hwnd, in_message, in_wparam, in_lparam);
			}
			break;

		case WM_PAINT:
			if (nullptr != application)
			{
				//application->Update();
				PAINTSTRUCT ps;
				(void)BeginPaint(in_hwnd, &ps);
				EndPaint(in_hwnd, &ps);
			}
			break;

		case WM_ERASEBKGND:
			return TRUE;

		case WM_MOVE:
			if (nullptr != application)
			{
				application->OnWindowMoved();
			}
			break;

		case WM_SIZE:
			if (nullptr != application)
			{
				if (in_wparam == SIZE_MINIMIZED)
				{
					if (false == application->GetMinimized())
					{
						application->SetMinimized(true);
						application->OnSuspending();
					}
				}
				else if (true == application->GetMinimized())
				{
					application->SetMinimized(false);
					application->OnResuming();
				}
				else if (false == application->GetInSizemove())
				{
					const DscCommon::VectorInt2 size(LOWORD(in_lparam), HIWORD(in_lparam));
					const float monitor_scale = GetMonitorScale(in_hwnd);

					application->OnWindowSizeChanged(size, monitor_scale);
				}
			}
			break;

		case WM_ENTERSIZEMOVE:
			if (nullptr != application)
			{
				application->SetInSizemove(true);
			}
			break;

		case WM_EXITSIZEMOVE:
			if (nullptr != application)
			{
				application->SetInSizemove(false);
				RECT rc;
				GetClientRect(in_hwnd, &rc);
				const DscCommon::VectorInt2 size(rc.right - rc.left, rc.bottom - rc.top);
				const float monitor_scale = GetMonitorScale(in_hwnd);

				application->OnWindowSizeChanged(size, monitor_scale);
			}
			break;

		case WM_GETMINMAXINFO:
			if (in_lparam)
			{
				auto info = reinterpret_cast<MINMAXINFO*>(in_lparam);
				info->ptMinTrackSize.x = 320;
				info->ptMinTrackSize.y = 200;
			}
			break;

			// equivalent to WM_ACTIVATE
		case WM_ACTIVATEAPP:
			if (nullptr != application)
			{
				if (in_wparam)
				{
					application->OnActivated();
				}
				else
				{
					application->OnDeactivated();
				}
			}
			break;

		case WM_POWERBROADCAST:
			switch (in_wparam)
			{
			default:
				break;
			case PBT_APMQUERYSUSPEND:
				if (nullptr != application)
				{
					application->OnSuspending();
				}
				return TRUE;
			case PBT_APMRESUMESUSPEND:
				if ((nullptr != application) && (false == application->GetMinimized()))
				{
					application->OnResuming();
				}
				return TRUE;
			}
			break;

		case WM_MOUSEWHEEL:
			if (nullptr != application)
			{
				const int key_state = GET_KEYSTATE_WPARAM(in_wparam);
				const int z_delta = GET_WHEEL_DELTA_WPARAM(in_wparam);
				application->OnScroll(key_state, z_delta);
			}
			break;

		//https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input#keystroke-message-flags
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			if (nullptr != application)
			{
				if ((in_wparam == VK_RETURN && (in_lparam & 0x60000000) == 0x20000000))
				{
					// Implements the classic ALT+ENTER fullscreen toggle
					if (application->GetFullScreen())
					{
						application->SetFullScreen(false);
						SetWindowLongPtr(in_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
						SetWindowLongPtr(in_hwnd, GWL_EXSTYLE, 0);

						const int width = application ? application->GetDefaultWidth() : 800;
						const int height = application ? application->GetDefaultHeight() : 600;

						ShowWindow(in_hwnd, SW_SHOWNORMAL);
						SetWindowPos(in_hwnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
					}
					else
					{
						application->SetFullScreen(true);
						SetWindowLongPtr(in_hwnd, GWL_STYLE, WS_POPUP);
						SetWindowLongPtr(in_hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);

						SetWindowPos(in_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
						ShowWindow(in_hwnd, SW_SHOWMAXIMIZED);
					}
				}
				else
				{
					WORD vk_code = LOWORD(in_wparam);								 // virtual-key code
					WORD key_flags = HIWORD(in_lparam);

					WORD scan_code = LOBYTE(key_flags);							 // scan code
					BOOL is_extended_key = (key_flags & KF_EXTENDED) == KF_EXTENDED; // extended-key flag, 1 if scancode has 0xE0 prefix

					if (is_extended_key)
					{
						scan_code = MAKEWORD(scan_code, 0xE0);
					}

					BOOL repeat_flag = (key_flags & KF_REPEAT) == KF_REPEAT;		// previous key-state flag, 1 on autorepeat
					WORD repeat_count = LOWORD(in_lparam);							// repeat count, > 0 if several keydown messages was combined into one message

					BOOL up_flag = (key_flags & KF_UP) == KF_UP;					// transition-state flag, 1 on keyup

					// if we want to distinguish these keys:
					switch (vk_code)
					{
					default:
						break;
					case VK_SHIFT:   // converts to VK_LSHIFT or VK_RSHIFT
					case VK_CONTROL: // converts to VK_LCONTROL or VK_RCONTROL
					case VK_MENU:	// converts to VK_LMENU or VK_RMENU
						vk_code = LOWORD(MapVirtualKeyW(scan_code, MAPVK_VSC_TO_VK_EX));
						break;
					}

					application->OnKey(vk_code, scan_code, repeat_flag, repeat_count, up_flag);
				}
			}
			break;

		case WM_MENUCHAR:
			// A menu is active and the user presses a key that does not correspond
			// to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
			return MAKELRESULT(0, MNC_CLOSE);
		}

		return 0; 
	}

} // namespace

void DscWindows::UpdateApplication(const HWND in_hwnd)
{
	DscWindows::IWindowApplication* const pApplication = GetWindowApplication(in_hwnd);
	if (nullptr != pApplication)
	{
		if (false == pApplication->Update())
		{
			DestroyWindow(in_hwnd);
		}
	}

	return;
}

const HWND DscWindows::WindowHelper(
	const TWindowApplicationFactory& in_application_factory,
	const int32 in_defaultWidth, 
	const int32 in_defaultHeight,
	const bool in_fullScreen,
	HINSTANCE in_instance,
	const std::string& in_application_name,
	const int in_cmd_show
)
{
	BOOL result = SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
	if (TRUE != result)
	{
		DSC_LOG_WARNING(LOG_TOPIC_DSC_WINDOWS, "SetProcessDpiAwarenessContext FAILED %d\n", result);
	}

	const std::wstring className(DscCommon::Utf8::Utf8ToUtf16(in_application_name + std::string("Class")));
	const std::wstring name(DscCommon::Utf8::Utf8ToUtf16(in_application_name));

	//static HBRUSH s_background_brush = NULL;
	//if (NULL == s_background_brush)
	//{
	//	LOGBRUSH lb = {};
	//	lb.lbStyle = BS_HOLLOW; // Or BS_HATCHED, BS_PATTERN, etc.
	//	lb.lbColor = RGB(0, 0, 0); // Red color
	//	lb.lbHatch = 0; // Not used for solid brush
	//	s_background_brush = CreateBrushIndirect(&lb);
	//}

	// Register class
	{
		WNDCLASSEXW wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEXW);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.hInstance = in_instance;
		wcex.hIcon = LoadIconW(in_instance, L"IDI_ICON");
		wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		//wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		//wcex.hbrBackground = s_background_brush;
		wcex.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(HOLLOW_BRUSH));

		wcex.lpszClassName = className.c_str();
		wcex.hIconSm = LoadIconW(wcex.hInstance, L"IDI_ICON");
		if (!RegisterClassExW(&wcex))
		{
			return NULL;
		}
	}

	// Create window
	HWND hwnd = NULL;
	{
		RECT rc = { 0, 0, static_cast<LONG>(in_defaultWidth), static_cast<LONG>(in_defaultHeight) };

		TCreateFunction createFunction = [=](const HWND in_wnd)
		{
			return in_application_factory(in_wnd, in_fullScreen, in_defaultWidth, in_defaultHeight);
		};
		DWORD dwStyle = in_fullScreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;
		AdjustWindowRect(&rc, dwStyle, FALSE);
		hwnd = CreateWindowExW(in_fullScreen ? WS_EX_TOPMOST : 0, className.c_str(), name.c_str(), dwStyle,
			CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, 
			nullptr, in_instance, &createFunction);

		if (!hwnd)
		{
			return NULL;
		}

#if 1
		// frame of white
		ShowWindow(hwnd, in_fullScreen ? SW_SHOWMAXIMIZED : in_cmd_show);
#else
		DSC_UNUSED(in_cmd_show);
		//https://stackoverflow.com/questions/69715610/how-to-initialize-the-background-color-of-win32-app-to-something-other-than-whit
		ShowWindow(hwnd, SW_SHOWMINIMIZED);
		ShowWindow(hwnd, in_fullScreen ? SW_SHOWMAXIMIZED : SW_RESTORE);
#endif
		UpdateWindow(hwnd);
	}

	return hwnd;
}

const bool DscWindows::GetMouseState(
	const HWND in_hwnd,
	DscCommon::VectorInt2& out_pos,
	bool& out_left_button,
	bool& out_right_button
	)
{
	bool valid = false;
	POINT p{ 0,0 };
	if (GetCursorPos(&p))
	{
		if (ScreenToClient(in_hwnd, &p))
		{
			valid = true;
		}
	}

	out_pos.Set(p.x, p.y);

	out_left_button = (0 != GetAsyncKeyState(VK_LBUTTON));
	out_right_button = (0 != GetAsyncKeyState(VK_RBUTTON));

	return valid;
}
