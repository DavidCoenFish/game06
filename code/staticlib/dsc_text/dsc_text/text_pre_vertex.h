#pragma once
#include "dsc_text.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_int2.h>
#include <dsc_common/vector_int4.h>
#include <dsc_common/vector_float4.h>

namespace DscText
{
	class Glyph;
	enum class THorizontalAlignment;

	/*
	collect data about a text run before converting it to vertex data for the gpu
	*/
	class TextPreVertex
	{
	public:
		struct PreVertexData
		{
			DscCommon::VectorInt4 _pos_low_high;
			DscCommon::VectorFloat4 _uv_low_high;
			int32 _mask;//DscCommon::VectorFloat4 _mask;
			int32 _colour; // DscCommon::VectorFloat4 _colour;
			int _line_index;
		};

		TextPreVertex(
			const int _default_line_height
		);
		~TextPreVertex();

		void Reserve(const unsigned int glyph_count);

		void AddPreVertexScale(
			const Glyph& in_cell,
			const int32 in_pos_x,
			const int32 in_pos_y,
			const float in_new_line_gap_ratio,
			const int32 in_colour, // DscCommon::VectorFloat4& in_colour,
			const float in_ui_scale
		);

		void AddPreVertex(
			const Glyph& in_cell,
			const int32 in_pos_x,
			const int32 in_pos_y,
			const int32 in_line_height,
			const int32 in_colour // in_colour
		);

		void AddPreVertex(
			const DscCommon::VectorInt2& in_width_height,
			const DscCommon::VectorInt2& in_bearing,
			const DscCommon::VectorFloat4& in_uv,
			const int32 in_mask,
			const int32 in_pos_x,
			const int32 in_pos_y,
			const int32 in_line_height,
			const int32 in_colour // const DscCommon::VectorFloat4& in_colour
		);

		void AddCursor(
			const int in_pos_x
		);

		void StartNewLine(
			DscCommon::VectorInt2& in_out_cursor
		);

		const DscCommon::VectorInt2 GetBounds(const bool in_use_em_height);

		void BuildVertexData(
			std::vector<uint8_t>& out_vertex_raw_data,
			const DscCommon::VectorInt2& in_container_size,
			THorizontalAlignment in_horizontal_line_alignment,
			const bool in_use_em_height,
			const int in_em_size // Used for alignments MiddleEM, TopEM, BottomEM
		);

	private:
		void FinishLine();

	private:
		std::vector<PreVertexData> _pre_vertex_data;
		DscCommon::VectorInt2 _vertical_bounds;
		std::vector<DscCommon::VectorInt2> _horizontal_bounds;

		bool _bound_dirty;
		DscCommon::VectorInt2 _bounds;
		int _line_index;

		bool _line_dirty; // Pre vertex added to line, but not adjusted for max height on line
		DscCommon::VectorInt2 _line_vertical_bounds;
		int _default_line_height; // If nothing added to the line, use this height
		int _current_line_height; // Allow for things added to the line to be taller

	};
}
