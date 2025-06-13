#pragma once
#include "dsc_windows.h"
#include <dsc_common/dsc_common.h>

class WindowApplicationParam;
class TaskWindow;
namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
}

namespace DscWindows
{
	class IWindowApplication
	{
	public:
		explicit IWindowApplication(const HWND in_hwnd = 0, const bool in_full_screen = 0, const int in_defaultWidth = 0, const int in_defaultHeight = 0);
		IWindowApplication() = delete;
		IWindowApplication& operator=(const IWindowApplication&) = delete;
		IWindowApplication(const IWindowApplication&) = delete;

		virtual ~IWindowApplication();

		virtual const bool Update();
		virtual void OnWindowMoved();
		virtual void OnWindowSizeChanged(const DscCommon::VectorInt2& in_size);
		virtual void OnActivated();
		virtual void OnDeactivated();
		virtual void OnSuspending();
		virtual void OnResuming();
		virtual void OnKey(const int in_vk_code, const int in_scan_code, const bool in_repeat_flag, const int in_repeat_count, bool in_up_flag);
		virtual void OnScroll(
			const int in_key_state,
			const int in_z_delta
		);
		//virtual void OnDestroy(const int in_exit_code);

		const int GetDefaultWidth() const { return _defaultWidth; }
		const int GetDefaultHeight() const { return _defaultHeight; }

		const bool GetInSizemove() const { return _sizemove; }
		void SetInSizemove(const bool in_sizemove) { _sizemove = in_sizemove; }

		const bool GetMinimized() const { return _minimized; }
		void SetMinimized(const bool in_minimized) { _minimized = in_minimized; }

		const bool GetFullScreen() const { return _full_screen; }
		void SetFullScreen(const bool in_full_screen) { _full_screen = in_full_screen; }

		const bool GetMouseState(
			int& out_x,
			int& out_y,
			bool& out_left_button,
			bool& out_right_button
		);

		HWND GetHwnd() const { return _hwnd; }

	private:
		HWND _hwnd;
		const int _defaultWidth;
		const int _defaultHeight;

		bool _sizemove;
		bool _suspend;
		bool _minimized;
		bool _full_screen;

	};

} //DscWindows
