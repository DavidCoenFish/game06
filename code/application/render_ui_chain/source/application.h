#pragma once
#include "render_ui.h"
#include <dsc_common/dsc_common.h>
#include <dsc_windows/i_window_application.h>
#include <dsc_dag/dag_node_group.h>
#include <dsc_ui/ui_enum.h>
#include <dsc_render_resource/render_target_pool.h>

#define LOG_TOPIC_APPLICATION "APPLICATION"

namespace DscCommon
{
    class FileSystem;
    class Timer;
}

namespace DscDag
{
    class DagCollection;
    class IDagNode;
    typedef IDagNode* NodeToken;
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

namespace DscDagRender
{
    class DagResource;
}

namespace DscUi
{
    class UiManager;
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

        std::unique_ptr<DscDag::DagCollection> _dag_collection = {};
        std::unique_ptr<DscUi::UiManager> _ui_manager = {};

        std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture> _gradient_render_target_texture = {};
        std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _gradient_constant_buffer = {};

        std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture> _button_render_target_texture = {};
        std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _button_constant_buffer = {};

        std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture> _effect_render_target_texture = {};
        std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _effect_constant_buffer = {};

        std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _root_constant_buffer = {};
    };
    std::unique_ptr<Resources> _resources;

    bool _keep_running = false;
};