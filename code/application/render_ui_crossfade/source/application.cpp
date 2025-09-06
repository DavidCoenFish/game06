#include "application.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_common/math.h>
#include <dsc_common/i_file_overlay.h>
#include <dsc_common/timer.h>
#include <dsc_common/vector_int2.h>
#include <dsc_dag/dag_collection.h>
#include <dsc_dag/i_dag_owner.h>
#include <dsc_dag/debug_print.h>
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
        _resources->_ui_root_node_group = _resources->_ui_manager->MakeRootNode(
            DscUi::MakeComponentCanvas().SetClearColour(DscCommon::VectorFloat4::s_zero),
            *_draw_system,
            *_resources->_dag_collection
        );

        _resources->_ui_manager->AddChildNode(
            DscUi::MakeComponentDebugGrid().SetChildSlot(
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f))
            ),
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            _resources->_ui_root_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "child one")
        );

        // want a cross fade node, with two children that we can toggle activation on
        // how do we nominate the active child of the cross fade, have a Selected child 
        _resources->_ui_crossfade_node_group = _resources->_ui_manager->AddChildNode(
            DscUi::MakeComponentCrossfade(
               nullptr
            ).SetChildSlot(
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f))
            ).SetClearColour(
                DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.0f)
            ),
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            _resources->_ui_root_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "crossfade")
        );

        _resources->_ui_crossfade_child_a = _resources->_ui_manager->AddChildNode(
            DscUi::MakeComponentFill(
                DscCommon::VectorFloat4(1.0f, 0.0f, 0.0f, 1.0f)
                //DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 0.5f)
            ).SetCrossfadeChildAmount(
                1.0f
            ),
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            _resources->_ui_crossfade_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "crossfade child a")
        );

        _resources->_ui_crossfade_child_b = _resources->_ui_manager->AddChildNode(
            DscUi::MakeComponentFill(
                DscCommon::VectorFloat4(0.0f, 0.0f, 1.0f, 1.0f)
                //DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 0.5f)
            ).SetCrossfadeChildAmount(
                0.0f
            ),
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            _resources->_ui_crossfade_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "crossfade child b")
        );

        auto parent_resource_node = DscDag::DagNodeGroup::GetNodeTokenEnum(_resources->_ui_crossfade_node_group, DscUi::TUiNodeGroup::TUiComponentResources);
        auto cross_fade_active_node = parent_resource_node ? DscDag::DagNodeGroup::GetNodeTokenEnum(parent_resource_node, DscUi::TUiComponentResourceNodeGroup::TCrossfadeActiveChild) : nullptr;
        if (nullptr != cross_fade_active_node)
        {
            DscDag::SetValueType(cross_fade_active_node, _resources->_ui_crossfade_child_a);
        }
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

        float time_delta = 0.0f;
        if (_resources && _resources->_timer)
        {
            time_delta = _resources->_timer->GetDeltaSeconds();
        }

        if (_resources)
        {
            _resources->_time_accumulate += time_delta;
            if (2.0f < _resources->_time_accumulate)
            {
                _resources->_time_accumulate = 0.0f;

                DscDag::NodeToken active_child = nullptr;
				auto parent_resource_node = DscDag::DagNodeGroup::GetNodeTokenEnum(_resources->_ui_crossfade_node_group, DscUi::TUiNodeGroup::TUiComponentResources);
				auto cross_fade_active_node = parent_resource_node ? DscDag::DagNodeGroup::GetNodeTokenEnum(parent_resource_node, DscUi::TUiComponentResourceNodeGroup::TCrossfadeActiveChild) : nullptr;
				if (nullptr != cross_fade_active_node)
				{
					active_child = DscDag::GetValueType<DscDag::NodeToken>(cross_fade_active_node);
				}

                if (_resources->_ui_crossfade_child_a == active_child)
                {
                    active_child = _resources->_ui_crossfade_child_b;
                }
                else
                {
                    active_child = _resources->_ui_crossfade_child_a;
                }
                DscDag::SetValueType<DscDag::NodeToken>(cross_fade_active_node, active_child);
            }
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

        DscUi::UiRenderTarget* ui_texture = nullptr;
        if (_resources->_ui_manager)
        {
            _resources->_ui_manager->Update(
                _resources->_ui_root_node_group,
                time_delta,
                input_param,
                _draw_system->GetRenderTargetBackBuffer()->GetViewportSize()
            );

            ui_texture = _resources->_ui_manager->Draw(
                _resources->_ui_root_node_group,
                *_resources->_dag_collection,
                *frame
            );
        }

        frame->SetRenderTarget(_draw_system->GetRenderTargetBackBuffer());

        if (nullptr != ui_texture)
        {
            _resources->_ui_manager->DrawUiTextureToCurrentRenderTarget(
                *frame,
                _resources->_ui_root_node_group,
                ui_texture
            );
        }

        if (_resources->_onscreen_version)
        {
            _resources->_onscreen_version->Update(*_draw_system, *frame, *_resources->_text_manager);
        }

#if defined(_DEBUG)
        //DscDag::DagCollection::DebugDumpNode(_resources->_ui_root_node_group.GetNodeToken(DscUi::TUiRootNodeGroup::TDrawNode));
#endif //#if defined(_DEBUG)

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


