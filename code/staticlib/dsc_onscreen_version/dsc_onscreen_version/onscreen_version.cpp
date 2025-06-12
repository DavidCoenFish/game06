#include "onscreen_version.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_common/math.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_resource.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render_resource/frame.h>
#include <dsc_text/text_manager.h>
#include <dsc_text/text_run.h>
#include <dsc_text/text_run_text.h>
#include <dsc_text/glyph_collection_text.h>
#include <dsc_version/dsc_version.h>
#include <dsc_locale/dsc_locale.h>

DscOnscreenVersion::OnscreenVersion::OnscreenVersion(
	DscRender::DrawSystem* const in_draw_system,
	DscCommon::FileSystem* const in_file_system,
	DscText::TextManager* const in_text_manager
	)
	: DscRender::IResource(in_draw_system)
{
	DscText::GlyphCollectionText* font = in_text_manager->LoadFont(*in_file_system, DscCommon::FileSystem::JoinPath("data", "font", "code2000.ttf"));

	std::vector<std::unique_ptr<DscText::ITextRun>> text_run_array;
	DscCommon::VectorInt2 container_size = _draw_system->GetRenderTargetBackBuffer()->GetSize();
	const DscText::TextLocale* const pLocale = in_text_manager->GetLocaleToken(DscLocale::LocaleISO_639_1::English);

    const std::string version = DscCommon::LogSystem::Printf("%s\n%s %s\n%s\n%s",
        DscVersion::GetVersionString(),
        DscVersion::GetConfiguration(),
        DscVersion::GetPlatform(),
        DscVersion::GetTimestamp(),
        DscVersion::GetGitRevision()
    );

    //https://r12a.github.io/app-conversion/
    text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
        version.c_str(),
        pLocale,
        font,
        32,
        DscCommon::Math::ConvertColourToInt(127, 127, 127, 127)
    ));

    _text_run = std::make_unique<DscText::TextRun>(
        std::move(text_run_array),
        DscCommon::VectorInt2(),
        false,
        0,
        DscText::THorizontalAlignment::TRight
        );
}

void DscOnscreenVersion::OnscreenVersion::Update(
	DscRenderResource::Frame& in_frame
)
{
    in_frame->SetRenderTarget(_render_target_texture);

    auto geometry =_text_run->GetGeometry(_draw_system.get(), &in_frame);
    _text_manager->SetShader(_draw_system.get(), &in_frame);

    frame->Draw(geometry);

}

void DscOnscreenVersion::OnscreenVersion::OnDeviceLost()
{

}

void DscOnscreenVersion::OnscreenVersion::OnDeviceRestored(
	ID3D12GraphicsCommandList* const in_command_list,
	ID3D12Device2* const in_device
)
{

}

void DscOnscreenVersion::OnscreenVersion::OnResize(
	ID3D12GraphicsCommandList* const in_command_list,
	ID3D12Device2* const in_device,
	const DscCommon::VectorInt2& in_size
)
{

}