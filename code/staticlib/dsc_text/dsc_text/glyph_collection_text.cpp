#include "glyph_collection_text.h"

#include "glyph.h"
#include "text_locale.h"
#include "text_pre_vertex.h"
#include "glyph_atlas_texture.h"
#include <dsc_common\file_system.h>
#include <dsc_common\utf8.h>
#include <dsc_common\log_system.h>
#include <dsc_common\vector_int2.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <harfbuzz\hb.h>
#include <harfbuzz\hb-ft.h>


namespace
{
	const hb_tag_t KernTag = HB_TAG('k', 'e', 'r', 'n'); // kerning operations
	const hb_tag_t LigaTag = HB_TAG('l', 'i', 'g', 'a'); // standard ligature substitution
	const hb_tag_t CligTag = HB_TAG('c', 'l', 'i', 'g'); // contextual ligature substitution

	static hb_feature_t LigatureOff = { LigaTag, 0, 0, std::numeric_limits<unsigned int>::max() };
	static hb_feature_t LigatureOn = { LigaTag, 1, 0, std::numeric_limits<unsigned int>::max() };
	static hb_feature_t KerningOff = { KernTag, 0, 0, std::numeric_limits<unsigned int>::max() };
	static hb_feature_t KerningOn = { KernTag, 1, 0, std::numeric_limits<unsigned int>::max() };
	static hb_feature_t CligOff = { CligTag, 0, 0, std::numeric_limits<unsigned int>::max() };
	static hb_feature_t CligOn = { CligTag, 1, 0, std::numeric_limits<unsigned int>::max() };

	// hb_buffer_destroy(buffer);
	hb_buffer_t* MakeBuffer(
		const std::string& in_string_utf8,
		const DscText::TextLocale* const in_locale_token
	)
	{
		hb_buffer_t* buffer = hb_buffer_create();
		hb_buffer_add_utf8(buffer, in_string_utf8.c_str(), -1, 0, -1);

		//set script, language, direction
		if (nullptr != in_locale_token)
		{
			hb_buffer_set_direction(buffer, in_locale_token->GetDirection());
			hb_buffer_set_script(buffer, in_locale_token->GetScript());
			hb_buffer_set_language(buffer, in_locale_token->GetLanguage());
		}
		else
		{
			hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
			hb_buffer_set_script(buffer, HB_SCRIPT_LATIN);
			hb_buffer_set_language(buffer, hb_language_from_string("en", -1));
		}

		hb_buffer_guess_segment_properties(buffer);

		return buffer;
	}
}

DscText::GlyphCollectionText::GlyphCollectionText(
	FT_Library in_library,
	GlyphAtlasTexture* in_texture,
	DscCommon::FileSystem& in_file_system,
	const std::string& in_font_file_path
)
	: _library(in_library)
	, _texture(in_texture)
{
	if (false == in_file_system.LoadFile(_font_data, in_font_file_path))
	{
		DSC_LOG_ERROR(LOG_TOPIC_DSC_TEXT, "failed to load font data:%s\n", in_font_file_path.c_str());
	}

	// i don't seem to be able to effect change
	//_features.push_back(LigatureOff);
	//_features.push_back(LigatureOn); //when a and e are stuck together, utf8 "\xC3" "\xA6"
	//_features.push_back(KerningOff);
	//_features.push_back(KerningOn);
	//_features.push_back(CligOff);
	//_features.push_back(CligOn);
}

DscText::GlyphCollectionText::~GlyphCollectionText()
{
	for (auto& item : _map_size_font_face)
	{
		hb_font_destroy(item.second->_harf_buzz_font);
		FT_Done_Face(item.second->_face);
	}
	_map_size_font_face.clear();
}

void DscText::GlyphCollectionText::ClearAllGlyphUsage()
{
	for (auto& item : _map_size_font_face)
	{
		item.second->_map_codepoint_glyph.clear();
	}
}

void DscText::GlyphCollectionText::BuildPreVertexData(
	TextPreVertex& in_out_text_pre_vertex,
	int32& in_out_cursor, // allow multiple fonts to append pre vertex data
	const std::string& in_string_utf8,
	const TextLocale* const in_locale_token,
	const int32 in_font_size,
	const bool in_width_limit_enabled,
	const int32 in_width_limit,
	const int32 in_colour,
	const int32 in_line_minimum_height,
	const int32 in_line_minimum_depth,
	const int32 in_line_gap_pixel
)
{
	auto map_codepoint_glyph = FindMapCodepointGlyph(in_font_size);

	hb_buffer_t* const buffer = MakeBuffer(
		in_string_utf8,
		in_locale_token
	);

	//const int32 colour = DscCommon::Math::ConvertColourToInt(in_colour);

	ShapeText(
		in_out_text_pre_vertex,
		in_out_cursor,
		in_string_utf8,
		buffer,
		*map_codepoint_glyph,
		in_width_limit_enabled,
		in_width_limit,
		in_line_minimum_height,
		in_line_minimum_depth,
		in_colour,
		in_line_gap_pixel
	);
	hb_buffer_destroy(buffer);
}

DscText::GlyphCollectionText::FontFace* const DscText::GlyphCollectionText::FindMapCodepointGlyph(const int in_glyph_size)
{
	FontFace* map_glyph_cell = nullptr;

	auto found = _map_size_font_face.find(in_glyph_size);
	if (found == _map_size_font_face.end())
	{
		auto temp = std::make_unique<FontFace>();

		FT_Error error = 0;
		error = FT_New_Memory_Face(
			_library,
			_font_data.data(),
			static_cast<FT_Long>(_font_data.size()),
			0,
			&temp->_face);
		if (error)
		{
			DSC_LOG_WARNING(LOG_TOPIC_DSC_TEXT, "Freetype font face error:%d\n", error);
		}

		error = FT_Set_Char_Size(temp->_face, in_glyph_size * 64, in_glyph_size * 64, 0, 0);
		if (error)
		{
			DSC_LOG_WARNING(LOG_TOPIC_DSC_TEXT, "Freetype FT_Set_Char_Size error:%d\n", error);
		}

		//https://github.com/tangrams/harfbuzz-example/blob/master/src/freetypelib.cpp
		for (int i = 0; i < temp->_face->num_charmaps; i++)
		{
			if (((temp->_face->charmaps[i]->platform_id == 0)
				&& (temp->_face->charmaps[i]->encoding_id == 3))
				|| ((temp->_face->charmaps[i]->platform_id == 3)
					&& (temp->_face->charmaps[i]->encoding_id == 1)))
			{
				FT_Set_Charmap(temp->_face, temp->_face->charmaps[i]);
			}
		}

		temp->_harf_buzz_font = hb_ft_font_create_referenced(temp->_face);

		map_glyph_cell = temp.get();
		_map_size_font_face[in_glyph_size] = std::move(temp);
	}
	else
	{
		map_glyph_cell = found->second.get();
	}

	return map_glyph_cell;
}

void DscText::GlyphCollectionText::ShapeText(
	DscText::TextPreVertex& in_out_text_pre_vertex,
	int32& in_out_cursor,
	const std::string& in_string_utf8,
	hb_buffer_t* in_buffer,
	FontFace& in_font_face,
	const bool in_width_limit_enabled,
	const int32 in_width_limit,
	const int32 in_line_minimum_height,
	const int32 in_line_minimum_depth,
	const int32 in_colour,
	const int32 in_line_gap_pixel
)
{
	hb_shape(in_font_face._harf_buzz_font, in_buffer, _features.empty() ? NULL : _features.data(), static_cast<unsigned int>(_features.size()));

	unsigned int glyph_count = 0;
	hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(in_buffer, &glyph_count);
	hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(in_buffer, &glyph_count);

	// this still isn't perfect, it can split glyphs of the same cluster up over a line break
	// better would be to check that ALL the glyphs of a cluster fit on the line if needed, but this is non trivial and needing multiple lookups of ghyph cells...
	for (unsigned int i = 0; i < glyph_count; i++)
	{
		// catch any new line characters from the input string
		int32 largest_written_pixel_x = 0;
		if (in_string_utf8[glyph_info[i].cluster] == '\n')
		{
			in_out_text_pre_vertex.StartNewLine(in_out_cursor, in_line_gap_pixel);
			continue;
		}

		hb_codepoint_t codepoint = glyph_info[i].codepoint;

		const int x_offset = glyph_pos[i].x_offset / 64;
		const int y_offset = glyph_pos[i].y_offset / 64;

		FT_Error error = FT_Load_Glyph(in_font_face._face, codepoint, FT_LOAD_DEFAULT);
		if (error)
		{
			DSC_LOG_ERROR(LOG_TOPIC_DSC_TEXT, "Freetype load glyph error:%d codepoint:%d\n", error, codepoint);
			continue;
		}

		FT_GlyphSlot slot = in_font_face._face->glyph;
		auto cell = FindCell(codepoint, in_font_face._map_codepoint_glyph);
		if (nullptr == cell)
		{
			// only ask to render the glyph to the slot->bitmap if we didn't find the cell, ie, don't have cell to re-use
			FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);

			cell = MakeGlyph(codepoint, slot, in_font_face._map_codepoint_glyph);
		}

		if (nullptr != cell)
		{
			largest_written_pixel_x = in_out_cursor + x_offset + cell->GetWidthHeightRef().GetX() + cell->GetBearingRef().GetX();
			if (in_width_limit_enabled)
			{
				// the (in_out_cursor != 0) is an attempt to at least has something on the line
				const hb_glyph_flags_t flag = hb_glyph_info_get_glyph_flags(&glyph_info[i]);
				if ((in_width_limit < largest_written_pixel_x) &&
					(0 == (flag & HB_GLYPH_FLAG_UNSAFE_TO_BREAK)) &&
					(in_out_cursor != 0))
				{
					in_out_text_pre_vertex.StartNewLine(in_out_cursor, in_line_gap_pixel);
					largest_written_pixel_x = in_out_cursor + x_offset + cell->GetWidthHeightRef().GetX() + cell->GetBearingRef().GetX();
				}
			}
			in_out_text_pre_vertex.AddPreVertex(
				*cell,
				in_out_cursor + x_offset,
				y_offset,
				in_line_minimum_height,
				in_line_minimum_depth,
				in_colour
			);
		}

		const int x_advance = glyph_pos[i].x_advance / 64;
		const int y_advance = glyph_pos[i].y_advance / 64;

		in_out_cursor += x_advance;
		DSC_ASSERT(0 == y_advance, "can this happen");
	}

	return;
}

DscText::Glyph* DscText::GlyphCollectionText::FindCell(
	hb_codepoint_t in_codepoint,
	TMapCodepointGlyph& in_out_map_glyph_cell
)
{
	auto found = in_out_map_glyph_cell.find(in_codepoint);
	if (found != in_out_map_glyph_cell.end())
	{
		return found->second.get();
	}
	return nullptr;
}

DscText::Glyph* DscText::GlyphCollectionText::MakeGlyph(
	hb_codepoint_t in_codepoint,
	FT_GlyphSlot in_slot,
	TMapCodepointGlyph& in_out_map_glyph_cell
)
{
	auto cell = _texture->AddGlyph(
		DscCommon::VectorInt2(in_slot->bitmap.width,
		in_slot->bitmap.rows),
		in_slot->bitmap_left,
		in_slot->bitmap_top,
		in_slot->bitmap.buffer
	);

	Glyph* result = cell.get();
	in_out_map_glyph_cell[in_codepoint] = std::move(cell);
	return result;
}
