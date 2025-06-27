#include "application.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/file_system.h>
#include <dsc_common/i_file_overlay.h>
#include <dsc_common/log_system.h>
#include <dsc_common/math.h>
#include <dsc_common/timer.h>
#include <dsc_common/vector_int2.h>
#include <dsc_dag/dag_collection.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render_resource/frame.h>
#include <dsc_render_resource/shader.h>
#include <dsc_render_resource/shader_constant_buffer.h>
#include <dsc_text/text_manager.h>
#include <dsc_onscreen_version/onscreen_version.h>
#include <dsc_ui/i_ui_component.h>
#include <dsc_ui/ui_manager.h>
#include <dsc_ui/ui_coord.h>
#include <dsc_ui/vector_ui_coord2.h>
#include <dsc_locale/dsc_locale.h>
#include <dsc_png/dsc_png.h>
#include <dsc_text/text_manager.h>
#include <dsc_text/text_run.h>
#include <dsc_text/text_run_text.h>
#include <dsc_text/glyph_collection_text.h>

namespace
{
    const int32 AddIconToTextManager(DscText::TextManager& in_text_manager, DscCommon::FileSystem& in_file_system, const std::string& in_path, const int32 in_bearing)
    {
        std::vector<uint8_t> image_data = {};
        int32_t byte_per_pixel = 0;
        DscCommon::VectorInt2 image_size;

        DscPng::LoadPng(
            image_data,
            byte_per_pixel,
            image_size,
            in_file_system,
            in_path
        );

        if (0 == image_data.size())
        {
            DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "zero size image:", in_path.c_str());
            return -1;
        }

        const int32 index = in_text_manager.AddIcon(image_size, in_bearing, image_data.data());
        return index;
    }

    std::unique_ptr<DscText::TextRun> MakeTextRun(DscText::TextManager& in_text_manager, DscCommon::FileSystem& in_file_system)
    {
        const int32 icon_a = AddIconToTextManager(in_text_manager, in_file_system, DscCommon::FileSystem::JoinPath("data", "sample_png", "a.png"), 54);
        const int32 icon_b = AddIconToTextManager(in_text_manager, in_file_system, DscCommon::FileSystem::JoinPath("data", "sample_png", "b.png"), 54);
        DscText::GlyphCollectionText* font = in_text_manager.LoadFont(in_file_system, DscCommon::FileSystem::JoinPath("data", "font", "code2000.ttf"));

        std::vector<std::unique_ptr<DscText::ITextRun>> text_run_array;
        DscCommon::VectorInt2 container_size(100,100);
        const DscText::TextLocale* const pLocale = in_text_manager.GetLocaleToken(DscLocale::LocaleISO_639_1::English);

        //https://r12a.github.io/app-conversion/
        text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
            "Non fixed width layout with working word wrap.\nAnd newlines\nin text.\nLocale:" "\xE4" "\xBD" "\xA0" "\xE5" "\xA5" "\xBD" "\xE4" "\xBA" "\xBA",
            pLocale,
            font,
            64,
            DscCommon::Math::ConvertColourToInt(255, 255, 255, 255),
            48
        ));
        text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
            "red",
            pLocale,
            font,
            80,
            DscCommon::Math::ConvertColourToInt(255, 0, 0, 255)
        ));
        text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
            "green",
            pLocale,
            font,
            64,
            DscCommon::Math::ConvertColourToInt(0, 255, 0, 255)
        ));
        text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
            "blue.\n",
            pLocale,
            font,
            48,
            DscCommon::Math::ConvertColourToInt(0, 0, 255, 255)
        ));
        text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
            "RGBA icons:",
            pLocale,
            font,
            64,
            DscCommon::Math::ConvertColourToInt(255, 255, 255, 255)
        ));
        text_run_array.push_back(DscText::TextRun::MakeTextRunDataIcon(
            icon_a,
            in_text_manager.GetIconFont(),
            DscCommon::Math::ConvertColourToInt(255, 255, 255, 255)
        ));
        text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
            " (mixed with text",
            pLocale,
            font,
            64,
            DscCommon::Math::ConvertColourToInt(255, 255, 255, 255)
        ));
        text_run_array.push_back(DscText::TextRun::MakeTextRunDataIcon(
            icon_b,
            in_text_manager.GetIconFont(),
            DscCommon::Math::ConvertColourToInt(255, 255, 255, 255)
        ));
        text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
            ")",
            pLocale,
            font,
            64,
            DscCommon::Math::ConvertColourToInt(255, 255, 255, 255)
        ));

        const int32 current_width = 800;
        auto text_run = std::make_unique<DscText::TextRun>(
            std::move(text_run_array),
            container_size,
            true,
            current_width,
            DscText::THorizontalAlignment::TNone,
            DscText::TVerticalAlignment::TTop,
            24
            );
        return text_run;
    }
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
            *_resources->_dag_collection,
            std::move(ui_canvas_commponent)

            DSC_DEBUG_ONLY(DSC_COMMA "Canvas")
            );
        auto parent_node_group = DscUi::UiManager::ConvertUiRootNodeToParentNode(_resources->_ui_root_node_group);

        auto ui_component_debug_fill = _resources->_ui_manager->MakeComponentDebugFill(*_draw_system);
        auto node_1_result = _resources->_ui_manager->MakeUiNodeCanvasChild(
            *_draw_system,
            *_resources->_dag_collection,
            std::move(ui_component_debug_fill),
            DscCommon::VectorFloat4(1.0f, 0.0f, 0.0f, 1.0f),
            _resources->_ui_root_node_group,
            parent_node_group,
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
            DscUi::VectorUiCoord2(),
            DscUi::VectorUiCoord2()

            DSC_DEBUG_ONLY(DSC_COMMA "Debug fill")
        );

#if 0
        auto ui_component_padding = _resources->_ui_manager->MakeComponentPadding(
            DscUi::UiCoord(16, 0.0f),
            DscUi::UiCoord(16, 0.0f),
            DscUi::UiCoord(16, 0.0f),
            DscUi::UiCoord(16, 0.0f)
        );
        auto ui_padding_node = _resources->_ui_manager->MakeUiNodeCanvasChild(
            *_draw_system,
            *_resources->_dag_collection,
            std::move(ui_component_padding),
            DscCommon::VectorFloat4(0.0f, 0.5f, 0.0f, 0.5f),
            _resources->_ui_root_node_group,
            parent_node_group,
            DscUi::VectorUiCoord2(DscUi::UiCoord(400, 0.0f), DscUi::UiCoord(0, 1.0f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f))

            DSC_DEBUG_ONLY(DSC_COMMA "Padding")
        );

        auto text_run = MakeTextRun(*_resources->_text_manager, *_file_system);
        auto ui_component_text = _resources->_ui_manager->MakeComponentText(
            *_resources->_text_manager,
            std::move(text_run),
            DscUi::TUiComponentBehaviour::TNone
        );
        auto stack_node = _resources->_ui_manager->MakeUiNodePaddingChild(
            *_draw_system,
            *_resources->_dag_collection,
            std::move(ui_component_text),
            DscCommon::VectorFloat4(0.0f, 0.0f, 0.5f, 0.5f),
            _resources->_ui_root_node_group,
            ui_padding_node

            DSC_DEBUG_ONLY(DSC_COMMA "Text")
        );
#else
        auto text_run = MakeTextRun(*_resources->_text_manager, *_file_system);
        auto ui_component_text = _resources->_ui_manager->MakeComponentText(
            *_resources->_text_manager,
            std::move(text_run),
            DscUi::TUiComponentBehaviour::TNone
            );
        auto node_2_result = _resources->_ui_manager->MakeUiNodeCanvasChild(
            *_draw_system,
            *_resources->_dag_collection,
            std::move(ui_component_text),
            DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.0f),
            _resources->_ui_root_node_group,
            parent_node_group,
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
            DscUi::VectorUiCoord2(),
            DscUi::VectorUiCoord2()

            DSC_DEBUG_ONLY(DSC_COMMA "Text")
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


