#pragma once
#include "a00_spring_sphere.h"

#include <dsc_common/dsc_common.h>
#include <dsc_windows/i_window_application.h>

#define LOG_TOPIC_APPLICATION "APPLICATION"

namespace DscCommon
{
    class FileSystem;
}

namespace DscOnscreenVersion
{
    class OnscreenVersion;
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
    class ShaderResource;
    class ShaderConstantBuffer;
}

namespace DscText
{
    class TextManager;
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
        std::unique_ptr<DscOnscreenVersion::OnscreenVersion> _onscreen_version = {};

        std::shared_ptr<DscRenderResource::Shader> _advance_spring_shader;
        struct TSpringData
        {
            int32 SelfIndex;
            int32 LinkIndex[12];
            float LinkWeight[12]; //or use a special index for 
        };
        // 3 pos textures, [prev pos 2, pre pos, texture to write new pos to]

        // draw the given pos texture as point cloud the provided colour
        std::shared_ptr<DscRenderResource::Shader> _present_spring_shader;
        // geometry data is the pos index
        std::shared_ptr<DscRenderResource::GeometryGeneric> _present_spring_geometry;
    };
    std::unique_ptr<Resources> _resources;

};