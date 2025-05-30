#pragma once
#include <dsc_common/common.h>
#include <dsc_windows/i_window_application.h>

#define LOG_TOPIC_APPLICATION "APPLICATION"

namespace DscCommon
{
    class FileSystem;
}

namespace DscRender
{
    class DrawSystem;
}

namespace DscRenderResource
{
    class GeometryGeneric;
    class Shader;
    class ShaderResource;
}

class Application : public DscWindows::IWindowApplication
{
public:
    Application(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight);
    ~Application();

    Application() = delete;
    Application& operator=(const Application&) = delete;
    Application(const Application&) = delete;

private:
    virtual const bool Update() override;
    virtual void OnWindowSizeChanged(const int in_width, const int in_height) override;

private:
    typedef DscWindows::IWindowApplication BaseType;
    std::unique_ptr<DscCommon::FileSystem> _file_system;
    std::unique_ptr<DscRender::DrawSystem> _draw_system;
    std::shared_ptr<DscRenderResource::GeometryGeneric> _geometry_generic;
    std::shared_ptr<DscRenderResource::Shader> _shader;
    std::shared_ptr<DscRenderResource::ShaderResource> _texture;

};