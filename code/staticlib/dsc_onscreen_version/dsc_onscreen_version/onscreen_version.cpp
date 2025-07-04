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

    const int32 text_colour = DscCommon::Math::ConvertColourToInt(192, 192, 192, 255);

    text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
        DscCommon::LogSystem::Printf("%s %s\n", DscVersion::GetConfiguration(), DscVersion::GetPlatform()),
        pLocale,
        font,
        18,
        text_colour
    ));

    text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
        DscCommon::LogSystem::Printf("%s\n", DscVersion::GetGitRevision()),
        pLocale,
        font,
        16,
        text_colour
    ));

    text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
        DscCommon::LogSystem::Printf("%s", DscVersion::GetTimestamp()),
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
    DscCommon::VectorInt2 text_size_padded(text_size.GetX() + 16, text_size.GetY() + 16);
    DscCommon::VectorInt2 text_size_half(text_size.GetX() + 8, text_size.GetY() + 12);
    _text_run->SetTextContainerSize(text_size_half);
    // make a screen quad the size of the text, to the bottom right of the screen
    _screen_quad = std::make_unique<DscUi::ScreenQuad>(
        DscUi::VectorUiCoord2(DscUi::UiCoord(text_size_padded.GetX(), 0.0f), DscUi::UiCoord(text_size_padded.GetY(), 0.0f)),
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
                DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.25f)
            )
        );

        _render_target_texture = std::make_shared<DscRenderResource::RenderTargetTexture>(
            &in_draw_system,
            target_format_data_array,
            DscRender::RenderTargetDepthData(),
            text_size_padded,
            false,
            true,
            text_size_half
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
        DscRenderResource::ShaderPipelineStateData::FactoryBlendDescAlphaPremultiplied(),  //CD3DX12_BLEND_DESC(D3D12_DEFAULT),
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
    // draw the version text to our render target
    {
        auto geometry =_text_run->GetGeometry(&in_draw_system, &in_frame);
        auto shader = in_text_manager.GetShader(&in_draw_system, &in_frame);

        in_frame.SetRenderTarget(_render_target_texture.get());
        in_frame.SetShader(shader);
        in_frame.Draw(geometry);
    }

    // draw out render target texture to the backbuffer
    {
        auto geometry_version = _screen_quad->GetGeometry(in_draw_system, in_frame.GetCommandList());
        in_frame.SetRenderTarget(in_draw_system.GetRenderTargetBackBuffer(), in_allow_clear_backbuffer);
        in_frame.SetShader(_screen_quad_shader);
        in_frame.Draw(geometry_version);
    }

    return;
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