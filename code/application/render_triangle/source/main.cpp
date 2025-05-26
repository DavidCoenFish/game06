#include <dsc_common/common.h>
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

    DscWindows::WindowHelper(
        [](const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight)->DscWindows::IWindowApplication*
        {
            return new Application(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight);
        },
        800,
        600,
        false,
        in_hInstance,
        "RenderTriangle",
        in_nCmdShow
    );
    // what if we ran the update via a reference to the application while(pApplication->Run(error_code)){;}
#if 1
    // Main message loop:
    MSG msg = {};
    // GetMessage returns non zero on messages other than WM_QUIT
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, 0, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
#else
    int exitCode = 0;
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
            for (auto iter : _application_list)
            {
                iter->Update();
            }
        }
    }

#endif
}