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
#include <dsc_render_resource_png/dsc_render_resource_png.h>
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

    if (nullptr != _resources)
    {
        _resources->_timer = std::make_unique<DscCommon::Timer>();
    }

    if ((nullptr != _file_system) && (nullptr != _draw_system))
    {
        _resources->_text_manager = std::make_unique<DscText::TextManager>(*_draw_system, *_file_system);
        _resources->_onscreen_version = std::make_unique<DscOnscreenVersion::OnscreenVersion>(*_draw_system, *_file_system, *(_resources->_text_manager));
        _resources->_dag_collection = std::make_unique<DscDag::DagCollection>();
        _resources->_ui_manager = std::make_unique<DscUi::UiManager>(*_draw_system, *_file_system, *(_resources->_dag_collection));
    }

    {
        auto top_texture = _resources->_ui_manager->MakeUiRenderTarget(_draw_system->GetRenderTargetBackBuffer(), true);
        _resources->_ui_root_node_group = _resources->_ui_manager->MakeRootNode(
            DscUi::MakeComponentCanvas().SetClearColour(DscCommon::VectorFloat4::s_zero),
            *_draw_system,
            *_resources->_dag_collection,
            top_texture
        );

        auto root_as_parent = DscUi::UiManager::ConvertRootNodeGroupToNodeGroup(*_resources->_dag_collection, _resources->_ui_root_node_group);

        _resources->_ui_manager->AddChildNode(
            DscUi::MakeComponentDebugGrid().SetChildSlot(
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f))
            ),
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            root_as_parent,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "child one")
        );
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

        float time_delta = 0.0f;
        if (_resources && _resources->_timer)
        {
            time_delta = _resources->_timer->GetDeltaSeconds();
        }

        DscUi::UiInputParam input_param = {};
        {
            DscCommon::VectorInt2 pos = {};
            bool left_button = false;
            bool right_button = false;

            DscWindows::GetMouseState(
                GetHwnd(),
                pos,
                left_button,
                right_button
            );
            input_param.SetMouseTouch(
                pos,
                left_button,
                right_button
                );
        }

        if (_resources->_ui_manager)
        {
            _resources->_ui_manager->Update(
                _resources->_ui_root_node_group,
                time_delta,
                input_param,
                _draw_system->GetRenderTargetBackBuffer()
            );

            _resources->_ui_manager->Draw(
                _resources->_ui_root_node_group,
                *_resources->_dag_collection,
                *frame,
                true,
                _draw_system->GetRenderTargetBackBuffer()
            );
        }

        if (_resources->_onscreen_version)
        {
            _resources->_onscreen_version->Update(*_draw_system, *frame, *_resources->_text_manager);
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


