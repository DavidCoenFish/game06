#include "application.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_common/math.h>
#include <dsc_common/i_file_overlay.h>
#include <dsc_common/timer.h>
#include <dsc_common/vector_int2.h>
#include <dsc_dag/dag_collection.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render_resource/frame.h>
#include <dsc_render_resource/shader.h>
#include <dsc_render_resource/shader_constant_buffer.h>
#include <dsc_render_resource/shader_resource.h>
#include <dsc_render_resource/render_target_texture.h>
#include <dsc_text/text_manager.h>
#include <dsc_onscreen_version/onscreen_version.h>
#include <dsc_ui/ui_enum.h>
#include <dsc_ui/component_construction_helper.h>
#include <dsc_ui/ui_manager.h>
#include <dsc_ui/ui_render_target.h>
#include <dsc_ui/ui_input_param.h>
#include <dsc_locale/dsc_locale.h>
#include <dsc_png/dsc_png.h>
#include <dsc_text/text_manager.h>
#include <dsc_text/text_run.h>
#include <dsc_text/text_run_text.h>
#include <dsc_text/glyph_collection_text.h>
#include <dsc_windows/window_helper.h>

namespace
{
}

Application::Resources::Resources()
{
    //nop
}

Application::Application(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight)
    : DscWindows::IWindowApplication(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight)
{
    _keep_running = true;

    _file_system = std::make_unique<DscCommon::FileSystem>();
    _draw_system = DscRender::DrawSystem::FactoryClearColour(in_hwnd, DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 0.0f));

    _resources = std::make_unique<Resources>();


    if ((nullptr != _file_system) && (nullptr != _draw_system))
    {
        _resources->_dag_collection = std::make_unique<DscDag::DagCollection>();
        _resources->_ui_manager = std::make_unique<DscUi::UiManager>(*_draw_system, *_file_system, *(_resources->_dag_collection));

        std::vector < DscRender::RenderTargetFormatData > array_render_target_format_data = {};
        array_render_target_format_data.push_back(
            DscRender::RenderTargetFormatData(
                DXGI_FORMAT_B8G8R8A8_UNORM,
                true,
                DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 1.0f)
            )
        );

        // gradient
        _resources->_gradient_render_target_texture = _resources->_ui_manager->GetRenderTargetPool().MakeOrReuseRenderTarget(
            _draw_system.get(),
            array_render_target_format_data,
            DscRender::RenderTargetDepthData(),
            DscCommon::VectorInt2(256, 64)
            );
        _resources->_gradient_constant_buffer = _resources->_ui_manager->GetGradientFillShader()->MakeShaderConstantBuffer(_draw_system.get());

        // button
        _resources->_button_render_target_texture = _resources->_ui_manager->GetRenderTargetPool().MakeOrReuseRenderTarget(
            _draw_system.get(),
            array_render_target_format_data,
            DscRender::RenderTargetDepthData(),
            DscCommon::VectorInt2(256 + 64, 64 + 64)
        );
        _resources->_button_constant_buffer = _resources->_ui_manager->GetUiPanelShader()->MakeShaderConstantBuffer(_draw_system.get());

        //effect
        _resources->_effect_render_target_texture = _resources->_ui_manager->GetRenderTargetPool().MakeOrReuseRenderTarget(
            _draw_system.get(),
            array_render_target_format_data,
            DscRender::RenderTargetDepthData(),
            DscCommon::VectorInt2(256 + 64 + 64, 64 + 64 + 64)
        );
        _resources->_effect_constant_buffer = _resources->_ui_manager->GetEffectDropShadowShader()->MakeShaderConstantBuffer(_draw_system.get());

        //root
        _resources->_root_constant_buffer = _resources->_ui_manager->GetUiPanelShader()->MakeShaderConstantBuffer(_draw_system.get());
    }

    return;
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
    if (_draw_system && _resources && (false == GetMinimized()))
    {
        std::unique_ptr<DscRenderResource::Frame> frame = DscRenderResource::Frame::CreateNewFrame(*_draw_system);

        static int32 s_trace = 0;
        s_trace += 1;

        // gradient
        {
            frame->SetRenderTargetTexture(_resources->_gradient_render_target_texture->_render_target_texture);
            auto& gradient_const_buffer = _resources->_gradient_constant_buffer->GetConstant<DscUi::TGradientFillConstantBuffer>(0);
            if (0 == (s_trace & 0x1))
            {
                gradient_const_buffer = { {0.0f, 1.0f, 1.1f, 1.2f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} };
            }
            else
            {
                gradient_const_buffer = { {0.0f, 1.0f, 1.1f, 1.2f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} };
            }

            frame->SetShader(
                _resources->_ui_manager->GetGradientFillShader(),
                _resources->_gradient_constant_buffer
                );
            frame->Draw(_resources->_ui_manager->GetGeometryFullQuadPosUv());
            frame->SetRenderTarget(nullptr);
        }

        //button
        {
            frame->SetRenderTargetTexture(_resources->_button_render_target_texture->_render_target_texture);
            auto& button_const_buffer = _resources->_button_constant_buffer->GetConstant<DscUi::TUiPanelShaderConstantBuffer>(0);
            button_const_buffer._pos_size[0] = -0.959999979f;
            button_const_buffer._pos_size[1] = 1.0f;
            button_const_buffer._pos_size[2] = 1.91f;
            button_const_buffer._pos_size[3] = 1.91f;
            button_const_buffer._uv_size[0] = 0.00000000f;
            button_const_buffer._uv_size[1] = 0.00000000f;
            button_const_buffer._uv_size[2] = 1.00000000f;
            button_const_buffer._uv_size[3] = 0.912500024f;

            auto shader = _resources->_ui_manager->GetUiPanelShader();

            shader->SetShaderResourceViewHandle(
                0,
                _resources->_gradient_render_target_texture->_render_target_texture->GetShaderResourceHeapWrapperItem()
                );
            frame->SetShader(
                shader,
                _resources->_gradient_constant_buffer
            );
            frame->Draw(_resources->_ui_manager->GetGeometryUiPanel());
            frame->SetRenderTarget(nullptr);
        }

        // effect
        {
            frame->SetRenderTargetTexture(_resources->_effect_render_target_texture->_render_target_texture);
            const auto render_size = _resources->_effect_render_target_texture->_render_target_texture->GetViewportSize();
            auto& effect_const_buffer = _resources->_effect_constant_buffer->GetConstant<DscUi::TEffectConstantBuffer>(0);
            effect_const_buffer._width_height[0] = static_cast<float>(render_size.GetX());
            effect_const_buffer._width_height[1] = static_cast<float>(render_size.GetY());
            effect_const_buffer._width_height[2] = 0.0f;
            effect_const_buffer._width_height[3] = 0.0f;
            effect_const_buffer._effect_param[0] = 2.0f;
            effect_const_buffer._effect_param[1] = 4.0f;
            effect_const_buffer._effect_param[2] = 6.0f;
            effect_const_buffer._effect_param[3] = 0.0f;
            effect_const_buffer._effect_tint[0] = 0.0f;
            effect_const_buffer._effect_tint[1] = 0.0f;
            effect_const_buffer._effect_tint[2] = 1.0f;
            effect_const_buffer._effect_tint[3] = 1.0f;
            const auto texture_size = _resources->_button_render_target_texture->_render_target_texture->GetSize();
            const auto texture_viewport_size = _resources->_button_render_target_texture->_render_target_texture->GetViewportSize();
            effect_const_buffer._texture_param_0[0] = static_cast<float>(texture_viewport_size.GetX());
            effect_const_buffer._texture_param_0[1] = static_cast<float>(texture_viewport_size.GetY());
            effect_const_buffer._texture_param_0[2] = static_cast<float>(texture_size.GetX());
            effect_const_buffer._texture_param_0[3] = static_cast<float>(texture_size.GetY());

            auto shader = _resources->_ui_manager->GetEffectDropShadowShader();

            shader->SetShaderResourceViewHandle(
                0,
                _resources->_button_render_target_texture->_render_target_texture->GetShaderResourceHeapWrapperItem()
            );
            frame->SetShader(
                shader,
                _resources->_effect_constant_buffer
            );
            frame->Draw(_resources->_ui_manager->GetGeometryFullQuadPosUv());
            frame->SetRenderTarget(nullptr);
        }

        // root draw
        {
            frame->SetRenderTarget(_draw_system->GetRenderTargetBackBuffer());
            auto& panel_const_buffer = _resources->_root_constant_buffer->GetConstant<DscUi::TUiPanelShaderConstantBuffer>(0);
            panel_const_buffer._pos_size[0] = -0.959999979f;
            panel_const_buffer._pos_size[1] = 1.0f;
            panel_const_buffer._pos_size[2] = 1.91f;
            panel_const_buffer._pos_size[3] = 1.91f;
            panel_const_buffer._uv_size[0] = 0.00000000f;
            panel_const_buffer._uv_size[1] = 0.00000000f;
            panel_const_buffer._uv_size[2] = 1.00000000f;
            panel_const_buffer._uv_size[3] = 0.912500024f;

            auto shader = _resources->_ui_manager->GetUiPanelShader();

            shader->SetShaderResourceViewHandle(
                0,
                _resources->_effect_render_target_texture->_render_target_texture->GetShaderResourceHeapWrapperItem()
            );
            frame->SetShader(
                shader,
                _resources->_root_constant_buffer
            );
            frame->Draw(_resources->_ui_manager->GetGeometryUiPanel());
            frame->SetRenderTarget(nullptr);
        }
    }

    return _keep_running;
}

void Application::OnWindowSizeChanged(const DscCommon::VectorInt2& in_size, const float in_monitor_scale)
{
    BaseType::OnWindowSizeChanged(in_size, in_monitor_scale);

    DSC_LOG_DIAGNOSTIC(LOG_TOPIC_APPLICATION, "OnWindowSizeChanged size:%d %d scale:%f\n", in_size.GetX(), in_size.GetY(), in_monitor_scale);

    if (_draw_system)
    {
        _draw_system->OnResize();
    }

    return;
}


