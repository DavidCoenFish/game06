#include "application_ui.h"
#include "test_util.h"
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
} // namespace

DscWindows::IWindowApplication* const ApplicationUi::Factory(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight)
{
    return new ApplicationUi(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight);
}

ApplicationUi::ApplicationUi(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight)
    : DscWindows::IWindowApplication(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight)
{
    _file_system = std::make_unique<DscCommon::FileSystem>();
    _draw_system = DscRender::DrawSystem::FactoryClearColour(in_hwnd, DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 0.0f));

    bool ok = true;

    ok &= TestCreateDeleteRootAndChild();
    ok &= TestRemoveChild();

    SetExitCode(ok ? 1 : 0);

    return;
}

ApplicationUi::~ApplicationUi()
{
    if (_draw_system)
    {
        _draw_system->WaitForGpu();
    }

    _draw_system.reset();
    _file_system.reset();
}

const bool ApplicationUi::Update()
{
    BaseType::Update();
    return false;
}

void ApplicationUi::OnWindowSizeChanged(const DscCommon::VectorInt2& in_size, const float in_monitor_scale)
{
    BaseType::OnWindowSizeChanged(in_size, in_monitor_scale);

    DSC_LOG_DIAGNOSTIC(LOG_TOPIC_APPLICATION, "OnWindowSizeChanged size:%d %d scale:%f\n", in_size.GetX(), in_size.GetY(), in_monitor_scale);

    if (_draw_system)
    {
        _draw_system->OnResize();
    }

    return;
}

const bool ApplicationUi::TestCreateDeleteRootAndChild()
{
    bool ok = true;

    std::unique_ptr<DscDag::DagCollection> dag_collection = std::make_unique<DscDag::DagCollection>();
    std::unique_ptr<DscUi::UiManager> ui_manager = std::make_unique<DscUi::UiManager>(*_draw_system, *_file_system, *dag_collection);
    DscDag::NodeToken ui_root_node_group = {};

    const int dag_node_count_before = dag_collection->GetNodeCount();

    {
        auto top_texture = ui_manager->MakeUiRenderTarget(_draw_system->GetRenderTargetBackBuffer(), true);
        ui_root_node_group = ui_manager->MakeRootNode(
            DscUi::MakeComponentCanvas().SetClearColour(DscCommon::VectorFloat4::s_zero),
            *_draw_system,
            *dag_collection,
            top_texture
        );
    }

    {
        ui_manager->AddChildNode(
            DscUi::MakeComponentDebugGrid().SetChildSlot(
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f))
            ),
            *_draw_system,
            *dag_collection,
            ui_root_node_group,
            ui_root_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "child one")
        );
    }

    ui_manager->DestroyNode(
        *dag_collection,
        ui_root_node_group
        );

    const int dag_node_count_after = dag_collection->GetNodeCount();

    // allow resources to get off the gpu before shutdown of ui_manager
    _draw_system->WaitForGpu();

    ok = TEST_UTIL_EQUAL(ok, dag_node_count_before, dag_node_count_after);

    return ok;
}

const bool ApplicationUi::TestRemoveChild()
{
    bool ok = true;

    std::unique_ptr<DscDag::DagCollection> dag_collection = std::make_unique<DscDag::DagCollection>();
    std::unique_ptr<DscUi::UiManager> ui_manager = std::make_unique<DscUi::UiManager>(*_draw_system, *_file_system, *dag_collection);
    DscDag::NodeToken ui_root_node_group = {};

    {
        auto top_texture = ui_manager->MakeUiRenderTarget(_draw_system->GetRenderTargetBackBuffer(), true);
        ui_root_node_group = ui_manager->MakeRootNode(
            DscUi::MakeComponentCanvas().SetClearColour(DscCommon::VectorFloat4::s_zero),
            *_draw_system,
            *dag_collection,
            top_texture
        );
    }

    {
        ui_manager->AddChildNode(
            DscUi::MakeComponentDebugGrid().SetChildSlot(
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f))
            ),
            *_draw_system,
            *dag_collection,
            ui_root_node_group,
            ui_root_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "child one")
        );
    }

    DscDag::NodeToken child_to_remove = {};
    const int dag_node_count_before = dag_collection->GetNodeCount();
    {
        child_to_remove = ui_manager->AddChildNode(
            DscUi::MakeComponentDebugGrid().SetChildSlot(
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f))
            ),
            *_draw_system,
            *dag_collection,
            ui_root_node_group,
            ui_root_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "child two")
        );
    }
    ui_manager->RemoveDestroyChild(
        *dag_collection,
        ui_root_node_group,
        child_to_remove
    );

    const int dag_node_count_after = dag_collection->GetNodeCount();

    // allow resources to get off the gpu before shutdown of ui_manager
    _draw_system->WaitForGpu();

    ok = TEST_UTIL_EQUAL(ok, dag_node_count_before, dag_node_count_after);

    return ok;

}

