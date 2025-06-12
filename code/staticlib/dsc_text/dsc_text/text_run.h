#pragma once
#include "dsc_text.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_int2.h>
#include <dsc_common/vector_float4.h>
#include <dsc_text/i_text_run.h>
#include <dsc_text/text_enum.h>

namespace DscRender
{
	class DrawSystem;
}

namespace DscRenderResource
{
	class GeometryGeneric;
	class Frame;
}

namespace DscText
{
	class ITextRun;
	class GlyphCollectionIcon;
	class GlyphCollectionText;
	class TextLocale;

	class TextRun
	{
	public:
		static std::unique_ptr<ITextRun> MakeTextRunDataString(
			const std::string& in_string_utf8 = std::string(""),
			const TextLocale* const in_locale_token = nullptr,
			GlyphCollectionText* const in_text_font = nullptr,
			const int32 in_font_size = 0,
			const int32 in_colour = 0xff000000,
			const int32 in_line_minimum_height = 0
		);
		static std::unique_ptr<ITextRun> MakeTextRunDataIcon(
			const int in_icon_id = 0,
			GlyphCollectionIcon* const in_icon_font = nullptr,
			const int32 in_colour = 0xff000000,
			const int32 in_line_minimum_height = 0
		);

		TextRun() = delete;
		TextRun& operator=(const TextRun&) = delete;
		TextRun(const TextRun&) = delete;

		// Do we keep the defaults, and each run has the settings? or simplify so we just have global settings at top, and data for each run
		TextRun(
			std::vector<std::unique_ptr<ITextRun>>&& in_text_run_array,
			const DscCommon::VectorInt2& in_container_size,
			const bool in_width_limit_enabled = false,
			const int in_width_limit = 0,
			const THorizontalAlignment in_horizontal_line_alignment = THorizontalAlignment::TNone,
			const TVerticalAlignment in_vertical_block_alignment = TVerticalAlignment::TTop,
			const int32 in_lineGapPixels = 0,
			const float in_ui_scale = 1.0f
		);

		~TextRun();

		// build geoemtry if needed
		const std::shared_ptr<DscRenderResource::GeometryGeneric>& GetGeometry(
			DscRender::DrawSystem* const in_draw_system,
			DscRenderResource::Frame* const in_draw_system_frame
			);

		// Get the natural size required by the text using current width limit if enabled
		DscCommon::VectorInt2 GetTextBounds();

		void SetTextRunArray(
			std::vector<std::unique_ptr<ITextRun>>&& in_text_run_array
		);
		void SetTextContainerSize(
			const DscCommon::VectorInt2& in_size
		);
		void SetWidthLimit(
			const bool in_width_limit_enabled,
			const int in_width_limit
		);
		void SetHorizontalLineAlignment(
			const THorizontalAlignment in_horizontal_line_alignment
		);
		void SetVerticalBlockAlignment(
			const TVerticalAlignment in_vertical_block_alignment
		);
		void SetLineGapPixels(
			const int in_line_gap_pixels
		);
		void SetUIScale(
			const float in_ui_scale
		);

	private:
		std::vector<std::unique_ptr<ITextRun>> _text_run_array;
		DscCommon::VectorInt2 _container_size;
		bool _width_limit_enabled;
		int32 _width_limit;
		THorizontalAlignment _horizontal_line_alignment;
		TVerticalAlignment _vertical_block_alignment;
		int32 _line_gap_pixels;
		float _ui_scale;

		bool _calculate_dirty;
		std::unique_ptr<TextPreVertex> _pre_vertex_data;
		DscCommon::VectorInt2 _text_bounds;
		bool _geometry_dirty;
		std::shared_ptr<DscRenderResource::GeometryGeneric> _geometry;

	};
}
