#include <dsc_common/dsc_common.h>
#include <dsc_common/log_system.h>
#include "dsc_windows.h"
#include "i_window_application.h"

DscWindows::IWindowApplication::IWindowApplication(const HWND in_hwnd, const bool in_full_screen, const int in_defaultWidth, const int in_defaultHeight)
   : _hwnd(in_hwnd)
   , _defaultWidth(in_defaultWidth)
   , _defaultHeight(in_defaultHeight)
   , _sizemove(false)
   , _suspend(false)
   , _minimized(false)
   , _full_screen(in_full_screen)
{
	DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_WINDOWS, "IWindowApplication ctor %p\n", this);
}

DscWindows::IWindowApplication::~IWindowApplication()
{
	DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_WINDOWS, "IWindowApplication dtor %p\n", this);
}

const bool DscWindows::IWindowApplication::Update()
{
   return false;
}

void DscWindows::IWindowApplication::OnWindowMoved()
{
   return;
}

void DscWindows::IWindowApplication::OnWindowSizeChanged(const int, const int)
{
   return;
}

void DscWindows::IWindowApplication::OnActivated()
{
   return;
}

void DscWindows::IWindowApplication::OnDeactivated()
{
   return;
}

void DscWindows::IWindowApplication::OnSuspending()
{
   return;
}

void DscWindows::IWindowApplication::OnResuming()
{
   return;
}

void DscWindows::IWindowApplication::OnKey(const int, const int, const bool, const int, bool)
{
   return;
}

void DscWindows::IWindowApplication::OnScroll(const int, const int)
{
	return;
}

//void DscWindows::IWindowApplication::OnDestroy(const int)
//{
//	return;
//}

const bool DscWindows::IWindowApplication::GetMouseState(
	int& out_x,
	int& out_y,
	bool& out_left_button,
	bool& out_right_button
	)
{
	bool valid = false;
	POINT p{0,0};
	if (GetCursorPos(&p))
	{
		if (ScreenToClient(_hwnd, &p))
		{
			valid = true;
		}
	}

	out_x = p.x;
	out_y = p.y;

	out_left_button = (0 != GetAsyncKeyState(VK_LBUTTON));
	out_right_button = (0 != GetAsyncKeyState(VK_RBUTTON));

	return valid;
}


