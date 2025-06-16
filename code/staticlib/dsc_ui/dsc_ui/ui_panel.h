#pragma once
#include "dsc_ui.h"
#include "vector_ui_coord2.h"
#include <dsc_common\vector_int2.h>

namespace DscRender
{
	class DrawSystem;
}

namespace DscRenderResource
{
	class Frame;
	class GeometryGeneric;
}

namespace DscUi
{

	///if content size is bigger than our calculated size, we auto scroll contents (unless auto scroll disabled)
	/// was trying to have one ui shader (with time?) without recalculating all the auto scroll param, but with [on/off] off auto scroll, kind of easier to do it in the panel
	///vertex format [2 float pos, 2 float uv 0, 2 float uv 1]
	///shader constants [2 float uv set bias]
	/// have a simpiler shader for "no need to auto scroll path"? but that would need a different vertex format... just use screen quad for that case?
	class UiPanel
	{
	public:
		static const std::vector<D3D12_INPUT_ELEMENT_DESC>& GetInputElementDesc();

		explicit UiPanel(
			const VectorUiCoord2& in_quad_size,
			const VectorUiCoord2& in_quad_pivot,
			const VectorUiCoord2& in_parent_attach,
			const DscCommon::VectorInt2& in_parent_size
		);

		void SetParentSize(
			const DscCommon::VectorInt2& in_parent_size
			);

		/// <summary>
		/// updates the geometry if inputs are dirty
		/// </summary>
		/// <param name="in_draw_system"></param>
		/// <param name="in_command_list"></param>
		/// <returns></returns>
		std::shared_ptr<DscRenderResource::GeometryGeneric> GetGeometry(
			DscRender::DrawSystem& in_draw_system,
			ID3D12GraphicsCommandList* const in_command_list
		);

	private:
		void BuildVertexData(std::vector<uint8_t>& in_vertex_raw_data);

	private:
		VectorUiCoord2 _quad_size;// = {};
		VectorUiCoord2 _quad_pivot;// = {};
		VectorUiCoord2 _parent_attach;// = {};
		DscCommon::VectorInt2 _parent_size;// = {};
		std::shared_ptr<DscRenderResource::GeometryGeneric> _geometry_generic = {};
		bool _dirty = true;

	};
}