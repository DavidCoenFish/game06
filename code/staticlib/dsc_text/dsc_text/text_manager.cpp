#include "text_manager.h"

#include "text_locale.h"
#include "glyph.h"
#include "glyph_atlas_row.h"
#include "glyph_atlas_texture.h"
#include "glyph_collection_icon.h"
#include "glyph_collection_text.h"
#include <dsc_locale\dsc_locale.h>
#include <dsc_render_resource\shader.h>

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
		// Mask to pull data out of font texture, ie, which channel of font texture has the glyph alpha
		D3D12_INPUT_ELEMENT_DESC
		{
			//DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
		},
		// text colour
		D3D12_INPUT_ELEMENT_DESC
		{
			//"COLOR", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
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
}


DscText::TextManager::TextManager(DscRender::DrawSystem& drawSystem)
{
	FT_Error error;
	error = FT_Init_FreeType(&_library);

	_texture = std::make_unique<GlyphAtlasTexture>(drawSystem);

}

DscText::TextManager::~TextManager()
{
	FT_Done_FreeType(_library);
}

