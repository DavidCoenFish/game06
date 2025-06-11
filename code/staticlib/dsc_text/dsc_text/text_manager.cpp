#include "text_manager.h"

#include "text_locale.h"
#include "glyph.h"
#include "glyph_atlas_row.h"
#include "glyph_atlas_texture.h"
#include "glyph_collection_icon.h"
#include "glyph_collection_text.h"
#include <dsc_common\file_system.h>
#include <dsc_common\log_system.h>
#include <dsc_render\draw_system.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\shader.h>
#include <dsc_render_resource\shader_resource_info.h>
#include <dsc_locale\dsc_locale.h>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace
{
	static const std::vector<D3D12_INPUT_ELEMENT_DESC> s_input_element_desc_array({
		D3D12_INPUT_ELEMENT_DESC
		{
			"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
		},
		D3D12_INPUT_ELEMENT_DESC
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
		},
		// Mask to filter data out of font texture, ie, which channel of font texture has the glyph alpha, or all channels for icon
		D3D12_INPUT_ELEMENT_DESC
		{
			"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
		},
		// text colour/ icon tint
		D3D12_INPUT_ELEMENT_DESC
		{
			"COLOR", 1, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
		}
		});

	static std::shared_ptr<DscText::TextLocale> s_locale_en = std::make_shared<DscText::TextLocale>(
		HB_DIRECTION_LTR,
		HB_SCRIPT_LATIN,
		hb_language_from_string("en", -1)
		);
	static std::map<DscLocale::LocaleISO_639_1, std::shared_ptr<DscText::TextLocale>> s_map_text_locale({
		{
			DscLocale::LocaleISO_639_1::Arabic,
			std::make_shared<DscText::TextLocale>(
			HB_DIRECTION_RTL,
			HB_SCRIPT_ARABIC,
			hb_language_from_string("ar", -1)
		)},
		{
			DscLocale::LocaleISO_639_1::Chinese_Simplified,
			std::make_shared<DscText::TextLocale>(
			HB_DIRECTION_LTR, //HB_DIRECTION_TTB,
			HB_SCRIPT_HAN,
			hb_language_from_string("ch", -1)
		)},
		{
			DscLocale::LocaleISO_639_1::English,
			s_locale_en
		},
		{
			DscLocale::LocaleISO_639_1::French,
			std::make_shared<DscText::TextLocale>(
			HB_DIRECTION_LTR,
			HB_SCRIPT_LATIN,
			hb_language_from_string("fr", -1)
		)},
		{
			DscLocale::LocaleISO_639_1::Hindi,
			std::make_shared<DscText::TextLocale>(
			HB_DIRECTION_LTR, //HB_DIRECTION_TTB,
			HB_SCRIPT_DEVANAGARI,
			hb_language_from_string("hi", -1)
		)},
		{
			DscLocale::LocaleISO_639_1::Russian,
			std::make_shared<DscText::TextLocale>(
			HB_DIRECTION_LTR,
			HB_SCRIPT_CYRILLIC,
			hb_language_from_string("ru", -1)
		)},
	});
} // namespace

const std::vector<D3D12_INPUT_ELEMENT_DESC>& DscText::TextManager::GetInputElementDesc()
{
	return s_input_element_desc_array;
}

DscText::TextManager::TextManager(DscRender::DrawSystem& draw_system, DscCommon::FileSystem& file_system)
{
	FT_Error error = 0;
	error = FT_Init_FreeType(&_library);
	if (error)
	{
		DSC_LOG_ERROR(LOG_TOPIC_DSC_TEXT, "Freetype init error:%d\n", error);
		return;
	}

#if defined(DSC_LOG)
	FT_Int major = 0;
	FT_Int minor = 0;
	FT_Int patch = 0;
	FT_Library_Version(_library,
		&major,
		&minor,
		&patch);
	DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_TEXT, "Freetype FT_Library_Version major:%d minor:%d patch:%d\n", major, minor, patch);
#endif // log

	_texture = std::make_unique<GlyphAtlasTexture>(draw_system);

	//shader
	{
		std::vector<uint8> vertex_shader_data;
		if (false == file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_text", "text_vs.cso")))
		{
			DSC_LOG_WARNING(LOG_TOPIC_DSC_TEXT, "failed to load vertex shader\n");
		}
		std::vector<uint8> pixel_shader_data;
		if (false == file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_text", "text_ps.cso")))
		{
			DSC_LOG_WARNING(LOG_TOPIC_DSC_TEXT, "failed to load pixel shader\n");
		}
		std::vector < DXGI_FORMAT > render_target_format;
		render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
		DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
			s_input_element_desc_array,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			DXGI_FORMAT_UNKNOWN,
			// DXGI_FORMAT_D32_FLOAT,
			render_target_format,
			DscRenderResource::ShaderPipelineStateData::FactoryBlendDescAlphaPremultiplied(),  //CD3DX12_BLEND_DESC(D3D12_DEFAULT),
			CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
			CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
		);
		std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
		array_shader_resource_info.push_back(
			DscRenderResource::ShaderResourceInfo::FactoryDataSampler(
				_texture->GetHeapWrapperItem(),
				D3D12_SHADER_VISIBILITY_PIXEL
			)
		);
		_shader = std::make_shared<DscRenderResource::Shader>(
			&draw_system,
			shader_pipeline_state_data,
			vertex_shader_data,
			std::vector<uint8_t>(),
			pixel_shader_data,
			array_shader_resource_info
			);
	}

}

DscText::TextManager::~TextManager()
{
	_map_path_font.clear();

	FT_Done_FreeType(_library);
}

const DscText::TextLocale* const DscText::TextManager::GetLocaleToken(const DscLocale::LocaleISO_639_1 in_locale) const
{
	const auto found = s_map_text_locale.find(in_locale);
	if (found != s_map_text_locale.end())
	{
		return found->second.get();
	}
	return s_locale_en.get();
}

// Find or make a new text face
DscText::GlyphCollectionText* DscText::TextManager::LoadFont(DscCommon::FileSystem& in_file_system, const std::string& in_font_path)
{
	auto found = _map_path_font.find(in_font_path);
	if (found != _map_path_font.end())
	{
		return found->second.get();
	}

	auto font = std::make_unique<GlyphCollectionText>(_library, _texture.get(), in_file_system, in_font_path);
	DscText::GlyphCollectionText* glyph_collection_text = font.get();
	_map_path_font.insert(std::make_pair(in_font_path, std::move(font)));

	return glyph_collection_text;
}

void DscText::TextManager::SetShader(
	DscRender::DrawSystem* const in_draw_system,
	DscRenderResource::Frame* const in_draw_system_frame
	)
{
	_texture->UploadTexture(in_draw_system, in_draw_system_frame->GetCommandList());
	in_draw_system_frame->AddFrameResource(_texture->GetResource());
	_shader->SetShaderResourceViewHandle(0, _texture->GetHeapWrapperItem());
	in_draw_system_frame->SetShader(_shader);

	return;
}

