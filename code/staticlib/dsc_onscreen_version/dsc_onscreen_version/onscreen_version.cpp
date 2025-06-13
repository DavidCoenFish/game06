#include "onscreen_version.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_common/math.h>
#include <dsc_common/vector_int2.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_resource.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render_resource/frame.h>
#include <dsc_render_resource/shader.h>
#include <dsc_render_resource/render_target_texture.h>
#include <dsc_render_resource/shader_pipeline_state_data.h>
#include <dsc_render_resource/shader_resource_info.h>
#include <dsc_text/text_manager.h>
#include <dsc_text/text_run.h>
#include <dsc_text/text_run_text.h>
#include <dsc_text/glyph_collection_text.h>
#include <dsc_version/dsc_version.h>
#include <dsc_locale/dsc_locale.h>
#include <dsc_ui/vector_ui_coord2.h>
#include <dsc_ui/screen_quad.h>
#include <dsc_ui/ui_coord.h>

DscOnscreenVersion::OnscreenVersion::OnscreenVersion(
	DscRender::DrawSystem& in_draw_system,
	DscCommon::FileSystem& in_file_system,
	DscText::TextManager& in_text_manager
	)
	: DscRender::IResource(&in_draw_system)
{
	DscText::GlyphCollectionText* font = in_text_manager.LoadFont(in_file_system, DscCommon::FileSystem::JoinPath("data", "font", "code2000.ttf"));

	std::vector<std::unique_ptr<DscText::ITextRun>> text_run_array;
	DscCommon::VectorInt2 container_size = in_draw_system.GetRenderTargetBackBuffer()->GetSize();
	const DscText::TextLocale* const pLocale = in_text_manager.GetLocaleToken(DscLocale::LocaleISO_639_1::English);

    const std::string version = DscCommon::LogSystem::Printf("%s\n%s %s\n%s\n%s",
        DscVersion::GetVersionString(),
        DscVersion::GetConfiguration(),
        DscVersion::GetPlatform(),
        DscVersion::GetTimestamp(),
        DscVersion::GetGitRevision()
    );

    const int32 text_colour = DscCommon::Math::ConvertColourToInt(127, 127, 127, 127);

    text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
        DscCommon::LogSystem::Printf("%s \n", DscVersion::GetVersionString()),
        pLocale,
        font,
        16,
        text_colour
    ));

    text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
        DscCommon::LogSystem::Printf("%s %s \n", DscVersion::GetConfiguration(), DscVersion::GetPlatform()),
        pLocale,
        font,
        18,
        text_colour
    ));

    text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
        DscCommon::LogSystem::Printf("%s \n%s ", DscVersion::GetTimestamp(), DscVersion::GetGitRevision()),
        pLocale,
        font,
        16,
        text_colour
    ));


    _text_run = std::make_unique<DscText::TextRun>(
        std::move(text_run_array),
        container_size,
        false,
        0,
        DscText::THorizontalAlignment::TRight,
        DscText::TVerticalAlignment::TMiddle,
        8
        );

    DscCommon::VectorInt2 text_size = _text_run->GetTextBounds();
    text_size.Set(text_size.GetX() + 4, text_size.GetY() + 8);
    _text_run->SetTextContainerSize(text_size);
    // make a screen quad the size of the text, to the bottom right of the screen
    _screen_quad = std::make_unique<DscUi::ScreenQuad>(
        DscUi::VectorUiCoord2(DscUi::UiCoord(text_size.GetX(), 0.0f), DscUi::UiCoord(text_size.GetY(), 0.0f)),
        DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
        DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
        container_size
        );

    {
        std::vector < DscRender::RenderTargetFormatData > target_format_data_array = {};
        target_format_data_array.push_back(
            DscRender::RenderTargetFormatData(
                DXGI_FORMAT_B8G8R8A8_UNORM,
                true,
                DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.5f)
            )
        );

        _render_target_texture = std::make_shared<DscRenderResource::RenderTargetTexture>(
            &in_draw_system,
            target_format_data_array,
            DscRender::RenderTargetDepthData(),
            text_size
            );
    }


    std::vector<uint8> vertex_shader_data;
    if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "screen_quad_texture_vs.cso")))
    {
        DSC_LOG_WARNING(LOG_TOPIC_DSC_ONSCREEN_VERSION, "failed to load triangle vertex shader\n");
    }
    std::vector<uint8> pixel_shader_data;
    if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "screen_quad_texture_ps.cso")))
    {
        DSC_LOG_WARNING(LOG_TOPIC_DSC_ONSCREEN_VERSION, "failed to triangle load pixel shader\n");
    }
    std::vector < DXGI_FORMAT > render_target_format;
    render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
    DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
        DscUi::ScreenQuad::GetInputElementDesc(),
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
        DXGI_FORMAT_UNKNOWN,
        // DXGI_FORMAT_D32_FLOAT,
        render_target_format,
        CD3DX12_BLEND_DESC(D3D12_DEFAULT),
        CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
        CD3DX12_DEPTH_STENCIL_DESC()
    );
    std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
    array_shader_resource_info.push_back(DscRenderResource::ShaderResourceInfo::FactorySampler(
        _render_target_texture->GetShaderResourceHeapWrapperItem(),
        D3D12_SHADER_VISIBILITY_PIXEL
        ));

    _screen_quad_shader = std::make_shared<DscRenderResource::Shader>(
        &in_draw_system,
        shader_pipeline_state_data,
        vertex_shader_data,
        std::vector<uint8_t>(),
        pixel_shader_data,
        array_shader_resource_info
        );
}

void DscOnscreenVersion::OnscreenVersion::Update(
    DscRender::DrawSystem& in_draw_system,
    DscRenderResource::Frame& in_frame,
    DscText::TextManager& in_text_manager,
    const bool in_allow_clear_backbuffer
)
{
    in_frame.SetRenderTarget(_render_target_texture.get());
    auto geometry =_text_run->GetGeometry(&in_draw_system, &in_frame);
    in_text_manager.SetShader(&in_draw_system, &in_frame);
    in_frame.Draw(geometry);

    // todo
    in_frame.SetRenderTarget(in_draw_system.GetRenderTargetBackBuffer(), in_allow_clear_backbuffer);
    in_frame.SetShader(_screen_quad_shader);
    _screen_quad->Draw(in_draw_system, in_frame);
}

void DscOnscreenVersion::OnscreenVersion::OnDeviceLost()
{
    //nop
}

void DscOnscreenVersion::OnscreenVersion::OnDeviceRestored(
	ID3D12GraphicsCommandList* const,
	ID3D12Device2* const
)
{
    //nop
}

void DscOnscreenVersion::OnscreenVersion::OnResize(
	ID3D12GraphicsCommandList* const, // in_command_list,
	ID3D12Device2* const,// in_device,
	const DscCommon::VectorInt2& in_size
)
{
    if (nullptr != _screen_quad)
    {
        _screen_quad->SetParentSize(in_size);
    }
}