#include <dsc_common/dsc_common.h>
#include <dsc_common/log_system.h>
#include <dsc_windows/window_helper.h>
#include "main.h"
#include "application.h"

int APIENTRY wWinMain(_In_ HINSTANCE in_hInstance,
    _In_opt_ HINSTANCE,// in_hPrevInstance,
    _In_ LPWSTR,//    in_lpCmdLine,
    _In_ int       in_nCmdShow)
{
    DscCommon::LogSystem logSystem(DscCommon::LogLevel::Diagnostic);

    const HWND hwnd = DscWindows::WindowHelper(
        [](const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight)->DscWindows::IWindowApplication*
        {
            return new Application(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight);
        },
        800,
        600,
        false,
        in_hInstance,
        "RenderGlyph",
        in_nCmdShow
    );

    int32 exitCode = 0;
    bool _continue = true;
    while (true == _continue)
    {
        MSG msg = {};
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (WM_QUIT == msg.message)
            {
                exitCode = (int)msg.wParam;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            _continue = DscWindows::UpdateApplication(hwnd);
        }
    }

    return exitCode;
}