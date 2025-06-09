#include "text_run.h"

#include "text_run_icon.h"
#include "text_run_text.h"
#include "text_pre_vertex.h"
#include <dsc_render_resource\geometry_generic.h>


std::unique_ptr<DscText::ITextRun> DscText::TextRun::MakeTextRunDataString(
	const std::string& in_string_utf8,
	TextLocale* const in_locale_token,
	GlyphCollectionText* const in_text_font,
	const int in_font_size,
	const float in_new_line_gap_ratio,
	const DscCommon::VectorFloat4& in_colour
)
{
	return std::make_unique<DscText::TextRunText>(
		in_string_utf8,
		in_text_font,
		in_locale_token,
		in_font_size,
		in_new_line_gap_ratio,
		in_colour
		);
}

std::unique_ptr<DscText::ITextRun> DscText::TextRun::MakeTextRunDataIcon(
	const int in_icon_id,
	GlyphCollectionIcon* const in_icon_font,
	const DscCommon::VectorFloat4& in_colour_tint,
	const float in_new_line_gap_ratio
)
{
	return std::make_unique<DscText::TextRunIcon>(
		in_icon_id,
		in_icon_font,
		in_colour_tint,
		in_new_line_gap_ratio
		);
}

// Do we keep the defaults, and each run has the settings? or simplify so we just have global settings at top, and data for each run
DscText::TextRun::TextRun(
	std::vector<std::unique_ptr<ITextRun>>&& in_text_run_array,
	const DscCommon::VectorInt2& in_container_size,
	const bool in_width_limit_enabled,
	const int in_width_limit,
	//const TextEnum::HorizontalLineAlignment in_horizontal_line_alignment = TextEnum::HorizontalLineAlignment::Left,
	//const TextEnum::VerticalBlockAlignment in_vertical_block_alignment = TextEnum::VerticalBlockAlignment::Top,
	const int in_em_size,
	const float in_ui_scale
)
: _text_run_array(std::move(in_text_run_array))
, _container_size(in_container_size)
, _width_limit_enabled(in_width_limit_enabled)
, _width_limit(in_width_limit)
, _em_size(in_em_size)
, _ui_scale(in_ui_scale)
, _calculate_dirty(true)
, _pre_vertex_data()
, _text_bounds()
, _geometry_dirty(true)
, _geometry()
{
	//nop
}

DscText::TextRun::~TextRun()
{
	//nop
}

// Get the natural size required by the text using current width limit if enabled
DscCommon::VectorInt2 DscText::TextRun::GetTextBounds()
{
	if (true == _calculate_dirty)
	{
		_calculate_dirty = false;

		_pre_vertex_data = std::make_unique<TextPreVertex>(_em_size);
		DscCommon::VectorInt2 cursor;
		for (const auto& item : _text_run_array)
		{
			item->BuildPreVertexData(
				*_pre_vertex_data,
				cursor,
				_width_limit_enabled,
				_width_limit,
				_ui_scale
			);
		}
		_text_bounds = _pre_vertex_data->GetBounds(false);
	}

	return _text_bounds;
}

void DscText::TextRun::SetTextRunArray(
	std::vector<std::unique_ptr<ITextRun>>&& in_text_run_array
)
{
	_text_run_array = std::move(in_text_run_array);
	_calculate_dirty = true;
	_geometry_dirty = true;
}

void DscText::TextRun::SetTextContainerSize(
	const DscCommon::VectorInt2& in_container_size
)
{
	if (_container_size != in_container_size)
	{
		_container_size = in_container_size;
		_geometry_dirty = true;
	}
}

void DscText::TextRun::SetWidthLimit(
	const bool in_width_limit_enabled,
	const int in_width_limit
)
{
	if (_width_limit_enabled != in_width_limit_enabled)
	{
		_width_limit_enabled = in_width_limit_enabled;
		_calculate_dirty = true;
		_geometry_dirty = true;
	}
	if (_width_limit != in_width_limit)
	{
		_width_limit = in_width_limit;
		if (true == _width_limit_enabled)
		{
			_calculate_dirty = true;
			_geometry_dirty = true;
		}
	}
}

//const bool SetHorizontalLineAlignment(
//	const TextEnum::HorizontalLineAlignment in_horizontal_line_alignment
//);
//const bool SetVerticalBlockAlignment(
//	const TextEnum::VerticalBlockAlignment in_vertical_block_alignment
//);

void DscText::TextRun::SetEMSize(
	const int in_em_size
)
{
	if (_em_size != in_em_size)
	{
		_em_size = in_em_size;
		_calculate_dirty = true;
		_geometry_dirty = true;
	}
}

void DscText::TextRun::SetUIScale(
	const float in_ui_scale
)
{
	if (_ui_scale != in_ui_scale)
	{
		_ui_scale = in_ui_scale;
		_calculate_dirty = true;
		_geometry_dirty = true;
	}
}
