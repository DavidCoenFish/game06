#include "render_text.h"
#include "application.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/file_system.h>
#include <dsc_common/i_file_overlay.h>
#include <dsc_render/draw_system.h>
#include <dsc_render_resource/frame.h>
#include <dsc_text/text_manager.h>

namespace
{
}

Application::Application(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight)
    : DscWindows::IWindowApplication(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight)
{
    _file_system = std::make_unique<DscCommon::FileSystem>();
    _draw_system = DscRender::DrawSystem::Factory(in_hwnd);

    _resources = std::make_unique<Resources>();
    if ((nullptr != _file_system) && (nullptr != _draw_system))
    {
        _resources->_text_manager = std::make_shared<DscText::TextManager>(*_draw_system, *_file_system);
    }
}

Application::~Application()
{
    if (_draw_system)
    {
        _draw_system->WaitForGpu();
    }
    _resources.reset();
    _draw_system.reset();
    _file_system.reset();
}

const bool Application::Update()
{
    BaseType::Update();
    if (_draw_system && (false == GetMinimized()))
    {
        std::unique_ptr<DscRenderResource::Frame> frame = DscRenderResource::Frame::CreateNewFrame(*_draw_system);

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

