#include <dsc_common/common.h>
#include "application.h"

#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render_resource/frame.h>

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

const bool Application::Update()
{
    BaseType::Update();
    if (_draw_system && (false == GetMinimized()))
    {
        std::unique_ptr<DscRenderResource::Frame> frame = DscRenderResource::Frame::CreateNewFrame(*_draw_system);

        frame->SetRenderTarget(_draw_system->GetRenderTargetBackBuffer());
    }
    
    return true;
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

