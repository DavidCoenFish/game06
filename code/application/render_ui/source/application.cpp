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
#include <dsc_render_resource/shader_resource.h>
#include <dsc_text/text_manager.h>
#include <dsc_onscreen_version/onscreen_version.h>
#include <dsc_ui/ui_manager.h>
#include <dsc_ui/ui_render_target.h>
#include <dsc_ui/ui_input_state.h>
#include <dsc_locale/dsc_locale.h>
#include <dsc_png/dsc_png.h>
#include <dsc_text/text_manager.h>
#include <dsc_text/text_run.h>
#include <dsc_text/text_run_text.h>
#include <dsc_text/glyph_collection_text.h>


namespace
{
/*
    std::shared_ptr<DscRenderResource::ShaderResource> MakeShaderResource(DscCommon::FileSystem& in_file_system, DscRender::DrawSystem& in_draw_system, const std::string& in_file_path)
    {
        std::vector<uint8> data = {};
        int32 byte_per_pixel = 0;
        DscCommon::VectorInt2 size = {};

        DscPng::LoadPng(
            data,
            byte_per_pixel,
            size,
            in_file_system,
            in_file_path
        );
        DscPng::ForceRgba(
            data,
            byte_per_pixel,
            size
        );
        std::shared_ptr<DscRenderResource::ShaderResource> result = {};
        if (0 < data.size())
        {
            D3D12_RESOURCE_DESC desc = {
                D3D12_RESOURCE_DIMENSION_TEXTURE2D, //D3D12_RESOURCE_DIMENSION Dimension;
                D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, //UINT64 Alignment;
                (UINT64)size.GetX(), //UINT64 Width;
                (UINT)size.GetY(), //UINT Height;
                1, //UINT16 DepthOrArraySize;
                1, //UINT16 MipLevels;
                DXGI_FORMAT_R8G8B8A8_UNORM, //DXGI_FORMAT Format;
                DXGI_SAMPLE_DESC{ 1, 0 }, //DXGI_SAMPLE_DESC SampleDesc;
                D3D12_TEXTURE_LAYOUT_UNKNOWN, //D3D12_TEXTURE_LAYOUT Layout;
                D3D12_RESOURCE_FLAG_NONE //D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE //D3D12_RESOURCE_FLAGS Flags;
            };
            // Describe and create a SRV for the texture.
            D3D12_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
            shader_resource_view_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            shader_resource_view_desc.Format = desc.Format;
            shader_resource_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            shader_resource_view_desc.Texture2D.MipLevels = 1;

            result = std::make_shared<DscRenderResource::ShaderResource>(
                &in_draw_system,
                in_draw_system.MakeHeapWrapperCbvSrvUav(), //const std::shared_ptr < HeapWrapperItem >&in_shader_resource,
                desc, //const D3D12_RESOURCE_DESC & in_desc,
                shader_resource_view_desc, //const D3D12_SHADER_RESOURCE_VIEW_DESC & in_shader_resource_view_desc,
                data //const std::vector<uint8_t>&in_data
                );
        }
        return result;
    }
*/

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
            DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "zero size image:%s\n", in_path.c_str());
            return -1;
        }

        const int32 index = in_text_manager.AddIcon(image_size, in_bearing, image_data.data());
        return index;
    }

    std::shared_ptr<DscText::TextRun> MakeTextRun(DscText::TextManager& in_text_manager, DscCommon::FileSystem& in_file_system)
    {
        const int32 icon_a = AddIconToTextManager(in_text_manager, in_file_system, DscCommon::FileSystem::JoinPath("data", "sample_png", "a.png"), 54);
        const int32 icon_b = AddIconToTextManager(in_text_manager, in_file_system, DscCommon::FileSystem::JoinPath("data", "sample_png", "b.png"), 54);
        DscText::GlyphCollectionText* font = in_text_manager.LoadFont(in_file_system, DscCommon::FileSystem::JoinPath("data", "font", "code2000.ttf"));

        std::vector<std::unique_ptr<DscText::ITextRun>> text_run_array;
        DscCommon::VectorInt2 container_size(100, 100);
        const DscText::TextLocale* const pLocale = in_text_manager.GetLocaleToken(DscLocale::LocaleISO_639_1::English);

        //https://r12a.github.io/app-conversion/
        text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
            "Non fixed width layout with working word wrap.\nAnd newlines\nin text.\nLocale:" "\xE4" "\xBD" "\xA0" "\xE5" "\xA5" "\xBD" "\xE4" "\xBA" "\xBA",
            pLocale,
            font,
            64,
            DscCommon::Math::ConvertColourToInt(255, 255, 255, 255)
            //, 48
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
        auto text_run = std::make_shared<DscText::TextRun>(
            std::move(text_run_array),
            container_size,
            true,
            current_width,
            DscText::THorizontalAlignment::TNone,
            DscText::TVerticalAlignment::TTop
            //, 24
            );
        return text_run;
    }
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
        auto top_texture = _resources->_ui_manager->MakeUiRenderTarget(_draw_system->GetRenderTargetBackBuffer(), true);

        _resources->_ui_root_node_group = _resources->_ui_manager->MakeRootNode(
            DscUi::UiManager::MakeComponentCanvas(DscCommon::VectorFloat4::s_zero),
            *_draw_system,
            *_resources->_dag_collection,
            top_texture
        );

        auto root_as_parent = DscUi::UiManager::ConvertRootNodeGroupToNodeGroup(*_resources->_dag_collection, _resources->_ui_root_node_group);

        _resources->_ui_manager->AddChildNode(
            DscUi::UiManager::MakeComponentDebugGrid().SetChildSlot(
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

        _resources->_ui_manager->AddChildNode(
            DscUi::UiManager::MakeComponentFill(DscCommon::VectorFloat4(0.5f, 0.0f, 0.0f, 0.5f)).SetChildSlot(
                DscUi::VectorUiCoord2(DscUi::UiCoord(256, 0.0f), DscUi::UiCoord(512, 0.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(32, 0.0f), DscUi::UiCoord(192, 0.0f))
                ),
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            root_as_parent,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "child two")
            );

        _resources->_ui_manager->AddChildNode(
            DscUi::UiManager::MakeComponentText(
                MakeTextRun(*_resources->_text_manager, *_file_system),
                _resources->_text_manager.get(),
                DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.25f)
            ).SetChildSlot(
                DscUi::VectorUiCoord2(DscUi::UiCoord(512 + 128 + 64, 0.0f), DscUi::UiCoord(512, 0.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(64, 0.0f), DscUi::UiCoord(64, 0.0f))
            ),
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            root_as_parent,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "child two")
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

        if (_resources->_ui_manager)
        {
            _resources->_ui_manager->Draw(
                _resources->_ui_root_node_group,
                *_resources->_dag_collection,
                *frame,
                true, //false,
                0.0f,
                DscUi::UiInputState(),
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


