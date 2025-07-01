#include "application.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_common/math.h>
#include <dsc_common/i_file_overlay.h>
#include <dsc_common/timer.h>
#include <dsc_common/vector_int2.h>
#include <dsc_dag/dag_collection.h>
#include <dsc_locale/dsc_locale.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render_resource/frame.h>
#include <dsc_render_resource/shader.h>
#include <dsc_render_resource/shader_constant_buffer.h>
#include <dsc_text/text_manager.h>
#include <dsc_text/text_run.h>
#include <dsc_text/i_text_run.h>
#include <dsc_onscreen_version/onscreen_version.h>
#include <dsc_ui/i_ui_component.h>
#include <dsc_ui/ui_manager.h>
#include <dsc_ui/ui_coord.h>
#include <dsc_ui/vector_ui_coord2.h>

namespace
{
}

Application::Resources::Resources() 
    : _ui_root_node_group(nullptr)
{
    //nop
}

Application::Application(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight)
    : DscWindows::IWindowApplication(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight)
{
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
        auto ui_canvas_commponent = _resources->_ui_manager->MakeComponentCanvas();
        _resources->_ui_root_node_group = _resources->_ui_manager->MakeUiRootNode(
            *_draw_system,
            *_resources->_dag_collection,
            std::move(ui_canvas_commponent),
            std::vector<DscUi::TEffectData>()
            DSC_DEBUG_ONLY(DSC_COMMA "canvas"));

        auto parent_node_group = DscUi::UiManager::ConvertUiRootNodeToParentNode(_resources->_ui_root_node_group);

        auto ui_component_debug_grid = _resources->_ui_manager->MakeComponentDebugGrid(*_draw_system);
        auto debug_grid_node = _resources->_ui_manager->MakeUiNodeCanvasChild(
            *_draw_system,
            *_resources->_dag_collection,
            std::move(ui_component_debug_grid),
            DscCommon::VectorFloat4(1.0f, 0.0f, 0.0f, 1.0f),
            _resources->_ui_root_node_group,
            parent_node_group,
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
            DscUi::VectorUiCoord2(),
            DscUi::VectorUiCoord2(),
            std::vector<DscUi::TEffectData>()
            DSC_DEBUG_ONLY(DSC_COMMA "debug grid"));

#if 1
        auto ui_component_fill = _resources->_ui_manager->MakeComponentFill();
        auto round_corner_node = _resources->_ui_manager->MakeUiNodeCanvasChild(
            *_draw_system,
            *_resources->_dag_collection,
            std::move(ui_component_fill),
            DscCommon::VectorFloat4(0.0f, 0.0f, 1.0f, 1.0f),
            _resources->_ui_root_node_group,
            parent_node_group,
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.25f), DscUi::UiCoord(0, 0.25f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
            std::vector<DscUi::TEffectData>({{
                    DscUi::TEffect::TRoundedCorner,
                    DscCommon::VectorFloat4(16.0f, 16.0f, 16.0f, 16.0f),
                    //DscCommon::VectorFloat4(64.0f, 64.0f, 64.0f, 64.0f),
                    DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.0f)}})

            DSC_DEBUG_ONLY(DSC_COMMA "round corner"));

#else
        auto margin = _resources->_ui_manager->MakeComponentMargin(
            DscUi::UiCoord(16, 0.0f),
            DscUi::UiCoord(16, 0.0f),
            DscUi::UiCoord(16, 0.0f),
            DscUi::UiCoord(16, 0.0f)
        );
        auto margin_node = _resources->_ui_manager->MakeUiNodeCanvasChild(
            *_draw_system,
            *_resources->_dag_collection,
            std::move(margin),
            DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 1.00f),
            _resources->_ui_root_node_group,
            parent_node_group,
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.25f), DscUi::UiCoord(0, 0.25f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
            std::vector<DscUi::TEffectData>({ {
                    DscUi::TEffect::TDropShadow,
                    DscCommon::VectorFloat4(2.0f, 4.0f, 2.0f, 0.0f),
                    DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.75f)} })
            DSC_DEBUG_ONLY(DSC_COMMA "margin"));

        auto ui_component_fill = _resources->_ui_manager->MakeComponentFill();
        auto round_corner_node = _resources->_ui_manager->MakeUiNodeMarginChild(
            *_draw_system,
            *_resources->_dag_collection,
            std::move(ui_component_fill),
            DscCommon::VectorFloat4(0.0f, 0.0f, 1.0f, 1.0f),
            _resources->_ui_root_node_group,
            margin_node,
            std::vector<DscUi::TEffectData>({ DscUi::TEffectData({
                    DscUi::TEffect::TRoundedCorner,
                    DscCommon::VectorFloat4(16.0f, 16.0f, 16.0f, 16.0f),
                    DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.0f)}) })

            DSC_DEBUG_ONLY(DSC_COMMA "round corner"));
#endif
#if 0
        auto ui_component_canvas = _resources->_ui_manager->MakeComponentCanvas();
        auto canvas_node = _resources->_ui_manager->MakeUiNodeEffectRounderCornerChild(
            *_draw_system,
            *_resources->_dag_collection,
            std::move(ui_component_canvas),
            DscCommon::VectorFloat4(0.0f, 1.0f, 0.0f, 1.0f),
            _resources->_ui_root_node_group,
            round_corner_node
            DSC_DEBUG_ONLY(DSC_COMMA "canvas"));

        auto ui_component_stroke = _resources->_ui_manager->MakeComponentEffectStroke(
            *_draw_system,
            DscCommon::VectorFloat4(4.0f, 0.0f, 0.0f, 0.0f),
            DscCommon::VectorFloat4(0.0f, 0.0f, 1.0f, 1.0f)
            );
        auto stroke_node = _resources->_ui_manager->MakeUiNodeCanvasChild(
            *_draw_system,
            *_resources->_dag_collection,
            std::move(ui_component_stroke),
            DscCommon::VectorFloat4(0.0f, 1.0f, 0.0f, 1.0f),
            _resources->_ui_root_node_group,
            canvas_node,

            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f))

            DSC_DEBUG_ONLY(DSC_COMMA "stroke"));

        DscText::GlyphCollectionText* font = _resources->_text_manager->LoadFont(*_file_system, DscCommon::FileSystem::JoinPath("data", "font", "code2000.ttf"));

        std::vector<std::unique_ptr<DscText::ITextRun>> text_run_array;
        const DscText::TextLocale* const pLocale = _resources->_text_manager->GetLocaleToken(DscLocale::LocaleISO_639_1::English);

        text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
            "Hello world",
            pLocale,
            font,
            32,
            DscCommon::Math::ConvertColourToInt(255, 255, 255, 255),
            24
        ));

        DscCommon::VectorInt2 container_size = {};
        const int32 current_width = 0;
        auto text_run = std::make_unique<DscText::TextRun>(
            std::move(text_run_array),
            container_size,
            true,
            current_width,
            DscText::THorizontalAlignment::TMiddle,
            DscText::TVerticalAlignment::TTop,
            12
            );

        auto ui_component_text = _resources->_ui_manager->MakeComponentText(
            *_resources->_text_manager,
            std::move(text_run),
            DscUi::TUiComponentBehaviour::TNone
        );
        _resources->_ui_manager->MakeUiNodeEffectStrokeChild(
            *_draw_system,
            *_resources->_dag_collection,
            std::move(ui_component_text),
            DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.0f),
            _resources->_ui_root_node_group,
            stroke_node
        );

#endif
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

        if (_resources->_ui_manager)
        {
            _resources->_ui_manager->UpdateUiSystem(
                _resources->_ui_root_node_group,
                time_delta
            );

            _resources->_ui_manager->DrawUiSystem(
                _resources->_ui_root_node_group,
                _draw_system->GetRenderTargetBackBuffer(),
                *frame,
                true,
                true
            );
        }

        if (_resources->_onscreen_version)
        {
            _resources->_onscreen_version->Update(*_draw_system, *frame, *_resources->_text_manager);
        }

        frame.reset();
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


