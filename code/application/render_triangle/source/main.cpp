#include <dsc_common/common.h>
#include <dsc_common/log_system.h>
#include <dsc_windows/i_window_application.h>
#include <dsc_windows/window_helper.h>
#include "main.h"

namespace
{
    class Application : public DscWindows::IWindowApplication
    {
    public:
        Application(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight) 
            : DscWindows::IWindowApplication(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight)
        {
            //nop
        }
        Application() = delete;
        Application& operator=(const Application&) = delete;
        Application(const Application&) = delete;

    };
} //namespace

int APIENTRY wWinMain(_In_ HINSTANCE in_hInstance,
    _In_opt_ HINSTANCE,// in_hPrevInstance,
    _In_ LPWSTR,//    in_lpCmdLine,
    _In_ int       in_nCmdShow)
{
    DscCommon::LogSystem logSystem(DscCommon::LogLevel::Diagnostic);

    auto createApplication = [](const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight)->DscWindows::IWindowApplication*
    {
        return new Application(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight);
    };

    DscWindows::WindowHelper(
        createApplication,
        800,
        600,
        false,
        in_hInstance,
        "RenderTriangle",
        in_nCmdShow
    );

    // Main message loop:
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, 0, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}