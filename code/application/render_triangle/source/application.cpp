#include <dsc_common/common.h>
#include "application.h"

#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>

Application::Application(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight)
    : DscWindows::IWindowApplication(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight)
{
    _draw_system = DscRender::DrawSystem::Factory(in_hwnd);
}

Application::~Application()
{
    if (_draw_system)
    {
        _draw_system->WaitForGpu();
    }
    _draw_system.reset();
}

void Application::Update()
{
    BaseType::Update();
    if (_draw_system)
    {
        ID3D12GraphicsCommandList* _command_list = nullptr;
        _draw_system->Prepare(_command_list);

        DscRender::IRenderTarget* render_target = _draw_system->GetRenderTargetBackBuffer();
        render_target->StartRender(_command_list, true);
        render_target->EndRender(_command_list);
        _draw_system->Present();
    }
    
    InvalidateRect(GetHwnd(), NULL, TRUE);
}
void Application::OnWindowSizeChanged(const int in_width, const int in_height)
{
    BaseType::OnWindowSizeChanged(
        in_width,
        in_height
    );
    if (_draw_system)
    {
        _draw_system->OnResize();
    }
    return;
}

