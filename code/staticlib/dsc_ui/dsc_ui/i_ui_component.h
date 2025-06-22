#pragma once
#include "dsc_ui.h"

namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
	template <typename TYPE>
	class Vector4;
	typedef Vector4<float> VectorFloat4;
}

namespace DscDag
{
	class IDagNode;
	typedef IDagNode* NodeToken;
}

namespace DscRender
{
	class IRenderTarget;
}

namespace DscRenderResource
{
	class Frame;
}

namespace DscUi
{
	class IUiComponent
	{
	public:
		virtual ~IUiComponent();

		// does the UiComponent have it's own geometry (or a shared ref to the UiManager's full quad)
		// if false, then geometry needs to be externally created, like via a dag node calculate, and passed into the draw function
		virtual const bool HasCustomGeometry() const;

		virtual void DrawCustomGeometry(
			DscRenderResource::Frame& in_frame,
			const DscCommon::VectorInt2& in_target_size
			);

		virtual const DscCommon::VectorFloat4& GetClearColour() const;
		// return true if value changes
		virtual const bool SetClearColour(const DscCommon::VectorFloat4& in_colour);

		virtual const DscCommon::VectorInt2 GetChildAvaliableSize(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index) const;
		virtual const DscCommon::VectorInt2 ConvertAvaliableSizeToDesiredSize(const DscCommon::VectorInt2& in_avaliable_size);
		virtual const DscCommon::VectorInt2 GetChildGeometrySize(const DscCommon::VectorInt2& in_child_desired_size, const DscCommon::VectorInt2& in_child_avaliable_size) const;// may need to fetch the desired size of all the children first, ie, for stack
		virtual const DscCommon::VectorInt2 GetChildGeometryOffset(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index) const;// may need to fetch the desired size of all the children first, ie, for stack

		virtual const int32 GetParentChildIndex() const = 0; // what child index are we of out parent
		virtual void Update(const float in_time_delta);
		virtual void AddChildRef(IUiComponent* in_ui_component);
		virtual void RemoveChild(IUiComponent* in_ui_component);
		virtual void SetScrollTraveralPixelDistance(const DscCommon::VectorInt2& in_traveral_pixel_distance);
		virtual void SetScrollNode(DscDag::NodeToken in_scroll_node);
		virtual void SetChildGeometrySizeNode(DscDag::NodeToken in_geometry_size_node, const int32 in_child_index);
		virtual void SetChildGeometryOffsetNode(DscDag::NodeToken in_geometry_offset_node, const int32 in_child_index);

	};
}