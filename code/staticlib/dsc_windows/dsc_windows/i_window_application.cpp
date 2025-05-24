#include <dsc_common/common.h>
#include <dsc_common/log_system.h>
#include "dsc_windows.h"
#include "i_window_application.h"

IWindowApplication::IWindowApplication(const HWND in_hwnd, const bool in_full_screen, const int in_defaultWidth, const int in_defaultHeight)
   : _defaultWidth(in_defaultWidth)
   , _defaultHeight(in_defaultHeight)
   , _hwnd(in_hwnd)
   , _sizemove(false)
   , _suspend(false)
   , _minimized(false)
   , _full_screen(in_full_screen)
{
	DSC_LOG_MESSAGE(LOG_TOPIC_DSC_WINDOWS, DscCommon::LogLevel::Diagnostic, "IWindowApplication ctor %p", this);
}

IWindowApplication::~IWindowApplication()
{
	DSC_LOG_MESSAGE(LOG_TOPIC_DSC_WINDOWS, DscCommon::LogLevel::Diagnostic, "IWindowApplication dtor %p", this);
}

void IWindowApplication::Update()
{
   return;
}

void IWindowApplication::OnWindowMoved()
{
   return;
}

void IWindowApplication::OnWindowSizeChanged(const int, const int)
{
   return;
}

void IWindowApplication::OnActivated()
{
   return;
}

void IWindowApplication::OnDeactivated()
{
   return;
}

void IWindowApplication::OnSuspending()
{
   return;
}

void IWindowApplication::OnResuming()
{
   return;
}

void IWindowApplication::OnKey(const int, const int, const bool, const int, bool)
{
   return;
}

void IWindowApplication::OnScroll(const int, const int)
{
	return;
}

void IWindowApplication::OnDestroy(const int)
{
	return;
}

const bool IWindowApplication::GetMouseState(
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


