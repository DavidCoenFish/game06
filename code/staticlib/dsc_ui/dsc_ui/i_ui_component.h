#pragma once
#include "dsc_ui.h"
#include "ui_enum.h"
#include <dsc_dag\dag_group.h>

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
	template <typename ENUM, std::size_t SIZE>
	class DagGroup;
}

namespace DscRender
{
	class HeapWrapperItem;
	class IRenderTarget;
}

namespace DscRenderResource
{
	class Frame;
	class ShaderConstantBuffer;
}

namespace DscUi
{
	typedef DscDag::DagGroup<TUiComponentGroup, static_cast<std::size_t>(TUiComponentGroup::TCount)> DagGroupUiComponent;

	class IUiComponent
	{
	public:
		virtual ~IUiComponent();

		// in theory it is only the node the ui component writes to, but possibly need more coupling to read values as well, like parent avaliable size, but that is a slippery slope of missing dirty calls
		void SetNode(const DagGroupUiComponent& in_ui_component_group);
		void SetClearColour(const DscCommon::VectorFloat4& in_colour);
		void SetParentChildIndex(const int32 in_parent_child_index);
		void SetManualScrollX(const float in_x);
		void SetManualScrollY(const float in_y);

		struct TEffectComponentData
		{
			TEffectData _effect_data = {};
			std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _shader_constant_buffer = {};
		};
		void SetEffectData(const int32 in_index, const TEffectComponentData& in_effect_data);
		TEffectComponentData GetEffectData(const int32 in_index);

		// so, components that want to have a desired size other than the avalaible size will not play nice with being top level ui components, that has the render viewport of the externaliily provided size
		// workaround is to just nest in a canvas or other accomidating component (padding? stack?)
		// can not set the top level render target clear colour, parent index should never be set, 
		// if we end up with a fixed size canvas, then that could allow the top level have desired size != avaliable size, also ui component stack? but that doesn't make sense, as we don't control the render viewport size of the top level render target...
		// this may make ui component stack unable to be a top level component?
		virtual const bool IsAllowedToBeTopLevelUiComponent() = 0;

		virtual void Draw(
			DscRenderResource::Frame& in_frame,
			DscRender::IRenderTarget& in_render_target,
			const float in_ui_draw_scale
			);

		// is this going to change to be like SetManualScrollX?
		virtual const bool HasManualScrollX() const;
		virtual const bool HasManualScrollY() const;

		// so, a UiComponentText may do word wrap and have a desired size shrunk in the vertical (or longer in vertical), parent still has GetChildGeometrySize to deal with shrink
		virtual const DscCommon::VectorInt2 ConvertAvaliableSizeToDesiredSize(const DscCommon::VectorInt2& in_parent_avaliable_size, const DscCommon::VectorInt2& in_avaliable_size, const float in_ui_scale);

		// following only needed if this is a component type that can have children
		virtual const DscCommon::VectorInt2 GetChildAvaliableSize(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index, const float in_ui_scale) const;
		// combine the desired size and the avalaible, in case we want to shring the geometry size if smaller than the avaliable, for stack children?
		// may need to fetch the desired size of all the children first, ie, for stack
		virtual const DscCommon::VectorInt2 GetChildGeometrySize(const DscCommon::VectorInt2& in_child_desired_size, const DscCommon::VectorInt2& in_child_avaliable_size) const;
		// may need to fetch the desired size of all the children first, ie, for stack
		virtual const DscCommon::VectorInt2 GetChildGeometryOffset(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index, const float in_ui_scale) const;

	//protected:
	private:
		DagGroupUiComponent _ui_component_group = {};
		std::vector<TEffectComponentData> _effect_data_array = {};

	};
}