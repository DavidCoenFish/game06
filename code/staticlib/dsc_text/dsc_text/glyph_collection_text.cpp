#include "glyph_collection_text.h"

#include "glyph.h"
#include "text_locale.h"
#include "text_pre_vertex.h"
#include "glyph_atlas_texture.h"
#include <dsc_common\file_system.h>
#include <dsc_common\utf8.h>
#include <dsc_common\log_system.h>

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
	: _texture(in_texture)
{
	FT_Error error = 0;

	if (false == in_file_system.LoadFile(_font_data, in_font_file_path))
	{
		DSC_LOG_WARNING(LOG_TOPIC_DSC_TEXT, "failed to load font data:%s\n", in_font_file_path.c_str());
		return;
	}

	error = FT_New_Memory_Face(
		in_library,
		_font_data.data(),
		static_cast<FT_Long>(_font_data.size()),
		0,
		&_face);
    if (error)
    {
        DSC_LOG_WARNING(LOG_TOPIC_DSC_TEXT, "Freetype font face error:%d path:%s\n", error, in_font_file_path.c_str());
        return;
    }

	FT_F26Dot6 FONT_SIZE = 128;
	error = FT_Set_Char_Size(_face, FONT_SIZE * 64, FONT_SIZE * 64, 0, 0);
	if (error)
	{
		DSC_LOG_WARNING(LOG_TOPIC_DSC_TEXT, "Freetype FT_Set_Char_Size error:%d\n", error);
		return;
	}

	//https://github.com/tangrams/harfbuzz-example/blob/master/src/freetypelib.cpp
	for (int i = 0; i < _face->num_charmaps; i++)
	{
		if (((_face->charmaps[i]->platform_id == 0)
			&& (_face->charmaps[i]->encoding_id == 3))
			|| ((_face->charmaps[i]->platform_id == 3)
				&& (_face->charmaps[i]->encoding_id == 1)))
		{
			FT_Set_Charmap(_face, _face->charmaps[i]);
		}
	}

	const int32 face_count = _face->num_faces;
	const int32 instances_count = _face->style_flags >> 16;

	DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_TEXT, "font face count:%d instances count:%d path:%s\n", face_count, instances_count, in_font_file_path.c_str());

	_harf_buzz_font = hb_ft_font_create_referenced(_face);

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
	hb_font_destroy(_harf_buzz_font);
	FT_Done_Face(_face);
}

void DscText::GlyphCollectionText::ClearAllGlyphUsage()
{
	_map_size_glyph_cell.clear();
}

void DscText::GlyphCollectionText::BuildPreVertexData(
	TextPreVertex& in_out_text_pre_vertex,
	DscCommon::VectorInt2& in_out_cursor, // allow multiple fonts to append pre vertex data
	const std::string& in_string_utf8,
	const TextLocale* const in_locale_token,
	const int in_font_size,
	const bool in_width_limit_enabled,
	const int in_width_limit,
	const int in_new_line_height,
	const int32 in_colour
)
{
	auto map_codepoint_glyph = FindMapCodepointGlyph(in_font_size);
	SetScale(in_font_size);

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
		in_new_line_height,
		in_colour
	);
	hb_buffer_destroy(buffer);
}

DscText::GlyphCollectionText::TMapCodepointGlyph* const DscText::GlyphCollectionText::FindMapCodepointGlyph(const int in_glyph_size)
{
	TMapCodepointGlyph* map_glyph_cell = nullptr;
	auto found = _map_size_glyph_cell.find(in_glyph_size);
	if (found == _map_size_glyph_cell.end())
	{
		auto temp = std::make_unique<TMapCodepointGlyph>();
		map_glyph_cell = temp.get();
		_map_size_glyph_cell[in_glyph_size] = std::move(temp);
	}
	else
	{
		map_glyph_cell = found->second.get();
	}

	return map_glyph_cell;
}

void DscText::GlyphCollectionText::SetScale(const int32 in_glyph_size)
{
	//FT_Set_Char_Size(_face, in_glyph_size * 64, in_glyph_size * 64, 0, 0);
	FT_Set_Pixel_Sizes(_face, in_glyph_size, in_glyph_size);
	return;
}

void DscText::GlyphCollectionText::ShapeText(
	DscText::TextPreVertex& in_out_text_pre_vertex,
	DscCommon::VectorInt2& in_out_cursor, // allow multiple fonts to append pre vertex data
	const std::string& in_string_utf8,
	hb_buffer_t* in_buffer,
	DscText::GlyphCollectionText::TMapCodepointGlyph& in_out_map_glyph_cell,
	const bool in_width_limit_enabled,
	const int32 in_width_limit,
	const int32 in_width_limit_new_line_height,
	const int32 in_colour
)
{
	hb_shape(_harf_buzz_font, in_buffer, _features.empty() ? NULL : _features.data(), static_cast<unsigned int>(_features.size()));

	unsigned int glyph_count = 0;
	hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(in_buffer, &glyph_count);
	hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(in_buffer, &glyph_count);

	in_out_text_pre_vertex.Reserve(glyph_count);

	// start a new line each time you hit this cluster for the width limit
	std::vector<unsigned int> width_line_clusert_index;
	if (true == in_width_limit_enabled)
	{
		int cursor_x = in_out_cursor.GetX();
		// Work out what line each glpyh should be on
		unsigned int current_cluster = std::numeric_limits<unsigned int>::max();// -1;
		for (unsigned int i = 0; i < glyph_count; i++)
		{
			hb_glyph_info_t& info = glyph_info[i];
			bool start_new_line = false;

			// Have at least one cluster per line
			if (std::numeric_limits<unsigned int>::max() == current_cluster)
			{
				current_cluster = info.cluster;
			}
			else if (info.cluster != current_cluster)
			{
				int trace_x = cursor_x;
				for (unsigned int ahead = i + 0; ahead < glyph_count; ahead++)
				{
					const int x_advance = glyph_pos[ahead].x_advance / 64;
					trace_x += x_advance;

					// We want to break the line if cluster goes past in_width_limit, and we can break on the cluster
					const hb_glyph_flags_t flag = hb_glyph_info_get_glyph_flags(&glyph_info[ahead]);
					if ((in_width_limit < trace_x) &&
						(0 == (flag & HB_GLYPH_FLAG_UNSAFE_TO_BREAK)))
					{
						// we don't set current_cluster to max (flag empty) as the new line is not empty, it will start with our "ahead" cluster
						width_line_clusert_index.push_back(glyph_info[ahead].cluster);
						i = ahead;
						cursor_x = x_advance;
						start_new_line = true;
						break;
					}
				}
				if (false == start_new_line)
				{
					// If we did not find a width limited new line, just break? no more work?
					break;
				}
			}

			if (false == start_new_line)
			{
				const int x_advance = glyph_pos[i].x_advance / 64;
				cursor_x += x_advance;
			}
		}
	}
	// Now place each glyph
	int line_index = 0;
	for (unsigned int i = 0; i < glyph_count; i++)
	{
		// catch any new line characters from the input string
		const unsigned int current_cluster = glyph_info[i].cluster;
		if (in_string_utf8[current_cluster] == '\n')
		{
			in_out_text_pre_vertex.StartNewLine(in_out_cursor);
			continue;
		}

		// Start new line
		if ((line_index < (int)width_line_clusert_index.size()) &&
			(width_line_clusert_index[line_index] == current_cluster))
		{
			line_index += 1;
			in_out_text_pre_vertex.StartNewLine(in_out_cursor);
		}

		hb_codepoint_t codepoint = glyph_info[i].codepoint;

		const int x_offset = glyph_pos[i].x_offset / 64;
		const int y_offset = glyph_pos[i].y_offset / 64;

		FT_Error error = FT_Load_Glyph(_face, codepoint, FT_LOAD_DEFAULT);
		if (error)
		{
			DSC_LOG_WARNING(LOG_TOPIC_DSC_TEXT, "Freetype loaf glyph error:%d codepoint:%d\n", error, codepoint);
			return;
		}

		FT_GlyphSlot slot = _face->glyph;
		auto cell = FindCell(codepoint, in_out_map_glyph_cell);
		if (nullptr == cell)
		{
			// only ask to render the glyph to the slot->bitmap if we didn't find the cell, ie, don't have cell to re-use
			FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);

			cell = MakeGlyph(codepoint, slot, in_out_map_glyph_cell);
		}

		if (nullptr != cell)
		{
			in_out_text_pre_vertex.AddPreVertex(
				*cell,
				in_out_cursor[0] + x_offset,
				in_out_cursor[1] + y_offset,
				in_width_limit_new_line_height,
				in_colour
			);
		}
		//const int x_advance = slot->advance.x / 64;
		//const int delta = (slot->lsb_delta - slot->rsb_delta) / 64;
		//const int y_advance = slot->advance.y / 64;
		const int x_advance = glyph_pos[i].x_advance / 64;
		const int y_advance = glyph_pos[i].y_advance / 64;

		in_out_cursor[0] += x_advance;
		in_out_cursor[1] += y_advance;

		in_out_text_pre_vertex.AddCursor(
			in_out_cursor[0]
		);
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
		in_slot->bitmap.width,
		in_slot->bitmap.rows,
		in_slot->bitmap_left,
		in_slot->bitmap_top,
		in_slot->bitmap.buffer
	);

	Glyph* result = cell.get();
	in_out_map_glyph_cell[in_codepoint] = std::move(cell);
	return result;
}
