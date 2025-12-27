#pragma once
#include "render_celtic_knot.h"

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
    class ShaderResource;
    class ShaderConstantBuffer;
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
    std::shared_ptr<DscRenderResource::GeometryGeneric> _geometry_generic;

    std::shared_ptr<DscRenderResource::Shader> _knot_shader;
    std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _knot_shader_constant_buffer;
    std::shared_ptr<DscRenderResource::RenderTargetTexture> _knot_render_target;

    //std::shared_ptr<DscRenderResource::ShaderResource> _texture;

    std::shared_ptr<DscRenderResource::Shader> _data_shader;
    std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _data_shader_constant_buffer;
    std::shared_ptr<DscRenderResource::RenderTargetTexture> _data_render_target;

    std::shared_ptr<DscRenderResource::Shader> _fill_knot_shader;
    std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _fill_knot_shader_constant_buffer;


    static constexpr int32 _k_knot_texture_size = 32;

};