#include "render_text.h"
#include "application.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_common/math.h>
#include <dsc_common/i_file_overlay.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render_resource/frame.h>
#include <dsc_text/text_manager.h>
#include <dsc_text/text_run.h>
#include <dsc_text/text_run_text.h>
#include <dsc_text/glyph_collection_text.h>
#include <dsc_png/dsc_png.h>

#include <harfbuzz/hb-ft.h>

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
}

Application::Resources::Resources() 
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
        const int32 icon_a = AddIconToTextManager(*_resources->_text_manager, *_file_system, DscCommon::FileSystem::JoinPath("data", "sample_png", "a.png"), 54);
        const int32 icon_b = AddIconToTextManager(*_resources->_text_manager, *_file_system, DscCommon::FileSystem::JoinPath("data", "sample_png", "b.png"), 54);
        DscText::GlyphCollectionText* font = _resources->_text_manager->LoadFont(*_file_system, DscCommon::FileSystem::JoinPath("data", "font", "code2000.ttf"));

        std::vector<std::unique_ptr<DscText::ITextRun>> text_run_array;
        DscCommon::VectorInt2 container_size = _draw_system->GetRenderTargetBackBuffer()->GetSize();
        const DscText::TextLocale* const pLocale = _resources->_text_manager->GetLocaleToken(DscLocale::LocaleISO_639_1::English);

        //https://r12a.github.io/app-conversion/
        text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
            "Non fixed width layout.\nLigature " "\xC3" "\xA6" ".\n" "\xE4" "\xBD" "\xA0" "\xE5" "\xA5" "\xBD" "\xE4" "\xBA" "\xBA",
            pLocale,
            font,
            64,
            DscCommon::Math::ConvertColourToInt(255, 255, 255, 255)
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
        text_run_array.push_back(DscText::TextRun::MakeTextRunDataIcon(
            icon_a,
            _resources->_text_manager->GetIconFont(),
            DscCommon::Math::ConvertColourToInt(255, 255, 255, 255)
        ));
        text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
            " (and",
            pLocale,
            font,
            64,
            DscCommon::Math::ConvertColourToInt(255, 255, 255, 255)
        ));
        text_run_array.push_back(DscText::TextRun::MakeTextRunDataIcon(
            icon_b,
            _resources->_text_manager->GetIconFont(),
            DscCommon::Math::ConvertColourToInt(255, 255, 255, 255)
        ));
        text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
            ") and now icons.",
            pLocale,
            font,
            64,
            DscCommon::Math::ConvertColourToInt(255, 255, 255, 255)
        ));

        const int32 current_width = _draw_system->GetRenderTargetBackBuffer()->GetSize().GetX();
        _resources->_text_run = std::make_unique<DscText::TextRun>(
            std::move(text_run_array),
            container_size,
            true,
            current_width,
            DscText::THorizontalAlignment::TNone,
            DscText::TVerticalAlignment::TTop,
            24
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
        frame->SetRenderTarget(_draw_system->GetRenderTargetBackBuffer());

        auto geometry = _resources->_text_run->GetGeometry(_draw_system.get(), frame.get());
        auto shader = _resources->_text_manager->GetShader(_draw_system.get(), frame.get());
        frame->SetShader(shader);
        frame->Draw(geometry);
    }
    
    return true;
}
void Application::OnWindowSizeChanged(const DscCommon::VectorInt2& in_size, const float in_monitor_scale)
{
    BaseType::OnWindowSizeChanged(in_size, in_monitor_scale);
    if (_draw_system)
    {
        _draw_system->OnResize();
    }

    if (_resources && _resources->_text_run)
    {
        _resources->_text_run->SetWidthLimit(true, in_size.GetX());
        _resources->_text_run->SetTextContainerSize(in_size);
    }

    return;
}


