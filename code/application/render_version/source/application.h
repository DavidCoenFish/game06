#pragma once
#include "render_version.h"
#include <dsc_common/dsc_common.h>
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
    class RenderTargetTexture;
    class Shader;
    class ShaderConstantBuffer;
}

namespace DscText
{
    class TextManager;
}

namespace DscOnscreenVersion
{
    class OnscreenVersion;
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
    virtual void OnWindowSizeChanged(const DscCommon::VectorInt2& in_size, const float in_monitor_scale) override;

private:
    typedef DscWindows::IWindowApplication BaseType;
    std::unique_ptr<DscCommon::FileSystem> _file_system;
    std::unique_ptr<DscRender::DrawSystem> _draw_system;

    struct Resources
    {
        Resources();
        Resources& operator=(const Resources&) = delete;
        Resources(const Resources&) = delete;

        std::unique_ptr<DscText::TextManager> _text_manager;
        std::unique_ptr<DscOnscreenVersion::OnscreenVersion> _onscreen_version;
    };
    std::unique_ptr<Resources> _resources;

};