#include "application.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_common/math.h>
#include <dsc_common/i_file_overlay.h>
#include <dsc_common/vector_int2.h>
#include <dsc_dag/dag_collection.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render_resource/frame.h>
#include <dsc_render_resource/shader.h>
#include <dsc_render_resource/shader_constant_buffer.h>
#include <dsc_text/text_manager.h>
#include <dsc_onscreen_version/onscreen_version.h>
#include <dsc_ui/component_construction_helper.h>
#include <dsc_ui/ui_manager.h>
#include <dsc_ui/ui_render_target.h>
#include <dsc_ui/ui_input_state.h>

namespace
{
}

Application::Resources::Resources()
    //: _ui_root_node_group(nullptr)
{
    //nop
}

Application::Application(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight)
    : DscWindows::IWindowApplication(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight)
{
    _file_system = std::make_unique<DscCommon::FileSystem>();
    _draw_system = DscRender::DrawSystem::FactoryClearColour(in_hwnd, DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 0.0f));

    _resources = std::make_unique<Resources>();
    if ((nullptr != _file_system) && (nullptr != _draw_system))
    {
        _resources->_text_manager = std::make_unique<DscText::TextManager>(*_draw_system, *_file_system);
        _resources->_onscreen_version = std::make_unique<DscOnscreenVersion::OnscreenVersion>(*_draw_system, *_file_system, *(_resources->_text_manager));
        _resources->_dag_collection = std::make_unique<DscDag::DagCollection>();
        _resources->_ui_manager = std::make_unique<DscUi::UiManager>(*_draw_system, *_file_system, *(_resources->_dag_collection));
    }

    {
        std::vector<DscUi::UiManager::TEffectConstructionHelper> effect_array = {};
        effect_array.push_back({ DscUi::TUiEffectType::TEffectCorner, DscCommon::VectorFloat4(64.0f, 64.0f, 64.0f, 64.0f) });
        effect_array.push_back({ DscUi::TUiEffectType::TEffectInnerShadow, DscCommon::VectorFloat4(4.0f, 2.0f, 5.0f, 0.0f), DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 1.0f) });
        effect_array.push_back({DscUi::TUiEffectType::TEffectTint, DscCommon::VectorFloat4(), DscCommon::VectorFloat4(0.05f, 0.0f, 0.0f, 0.05f) });
        auto top_texture = _resources->_ui_manager->MakeUiRenderTarget(_draw_system->GetRenderTargetBackBuffer(), true);
        _resources->_ui_root_node_group = _resources->_ui_manager->MakeRootNode(
            DscUi::MakeComponentDebugGrid(),
            *_draw_system,
            *_resources->_dag_collection,
            top_texture,
            effect_array
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

    //DSC_DEBUG_ONLY(DscDag::DebugPrintRecurseInputs(_resources->_ui_root_node_group));

    _resources->_ui_manager->DestroyNode(
        *(_resources->_dag_collection),
        _resources->_ui_root_node_group
    );
    _resources->_ui_root_node_group = nullptr;

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

        frame->SetRenderTarget(_draw_system->GetRenderTargetBackBuffer());

        if (_resources->_ui_manager)
        {
            _resources->_ui_manager->Draw(
                _resources->_ui_root_node_group,
                *_resources->_dag_collection,
                *frame,
                true, //false,
                _draw_system->GetRenderTargetBackBuffer()
            );
        }

        if (_resources->_onscreen_version)
        {
            _resources->_onscreen_version->Update(*_draw_system, *frame, *_resources->_text_manager);
        }
    }

    return true;
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


