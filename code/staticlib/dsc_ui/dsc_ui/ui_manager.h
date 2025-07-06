#pragma once
#include "dsc_ui.h"
#include "ui_enum.h"
#include "ui_coord.h"
#include "vector_ui_coord2.h"
#include <dsc_common\vector_float4.h>

namespace DscCommon
{
	class FileSystem;
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
	template <typename TYPE>
	class Vector4;
	typedef Vector4<float> VectorFloat4;
}

namespace DscDag
{
	class DagCollection;
	class IDagNode;
	typedef IDagNode* NodeToken;
}

namespace DscRender
{
	class DrawSystem;
	class IRenderTarget;
}

namespace DscRenderResource
{
	class Frame;
	class GeometryGeneric;
	class RenderTargetPool;
	class RenderTargetTexture;
	class Shader;
	class ShaderConstantBuffer;
	class ShaderResource;
}

namespace DscDagRender
{
	class DagResource;
}

namespace DscText
{
	class TextManager;
	class TextRun;
}

/*
layout calculation summary
root node is paired with an externally supplied UiRenderTaget, which could be created from  the backbuffer or an arbitary texture, that is the initial avalaible size for the layout

avaliable size:
a child node makes an avaliable size based on the parent avalable size, if it has child slot data, use that, if it has padding, use that to effect the avaliable size

desired size:
the child node uses that calculated avaliable size to make a desired size which is based on content, like the bounds of a text block, or the max of all children (geometry offset + size)

geometry size: 
based on the parent component type, canvas geometry size is the avaliable size for the child. for a stack, it is the desired size. 

geometry offset:
relative to the parent top left, where to position the gaometry to draw on parent

render request size:
what size to request a render target to draw the child onto, max of desired and geometry size

scroll:
if the desired size is bigger than the the geometry size, automatically scroll the visible area of the child in the parent geometry window, can also be scrolled manually

note, some of the steps may feel redundant, but breaking up the steps so that a parent like a stack, can use the children geometry size in it's desired size caculation, based on it's own avaliable ise handed down to the children
*/
namespace DscUi
{
	class IUiComponent;
	class UiCoord;
	class UiInputState;
	class UiRenderTarget;
	class VectorUiCoord2;

	class UiManager
	{
	public:
		UiManager() = delete;
		UiManager& operator=(const UiManager&) = delete;
		UiManager(const UiManager&) = delete;

		UiManager(DscRender::DrawSystem& in_draw_system, DscCommon::FileSystem& in_file_system, DscDag::DagCollection& in_dag_collection);
		~UiManager();

		// these are std::shared_ptr as do not have a DagNodeCalculate unique, and we are 
		std::shared_ptr<UiRenderTarget> MakeUiRenderTarget(
			DscRender::IRenderTarget* const in_render_target,
			const bool in_allow_clear_on_draw
			);
		std::shared_ptr<UiRenderTarget> MakeUiRenderTarget(
			const std::shared_ptr<DscRenderResource::RenderTargetTexture>& in_render_target_texture,
			const bool in_allow_clear_on_draw
			);

		struct TComponentConstructionHelper
		{
			TUiComponentType _component_type;
			DscCommon::VectorFloat4 _clear_colour;

			bool _has_fill = false;
			DscCommon::VectorFloat4 _fill;

			std::shared_ptr<DscRenderResource::ShaderResource> _texture;

			bool _has_child_slot_data = false;
			VectorUiCoord2 _child_size = {};
			VectorUiCoord2 _child_pivot = {};
			VectorUiCoord2 _attach_point = {};

			bool _has_padding = false;
			UiCoord _padding_left = {};
			UiCoord _padding_top = {};
			UiCoord _padding_right = {};
			UiCoord _padding_bottom = {};

			// gap between items for stack
			bool _has_gap = false;
			UiCoord _gap = {};

			bool _has_stack_attach = false;
			UiCoord _stack_pivot = {};
			UiCoord _stack_parent_attach_point = {};

			bool _desired_size_from_children_max = false; // otherwise from text? or matches avaliable

			// canvas- geometry size is avaliable
			// stack- geometry size is max (avaliable, desired)

			// canvas - geometry offset is from child slot
			// stack - geometry offset is from prev child plus gap, else from top left

			TComponentConstructionHelper& SetClearColour(
				const DscCommon::VectorFloat4& in_clear_colour
			)
			{
				_clear_colour = in_clear_colour;
				return *this;
			}
			TComponentConstructionHelper& SetChildSlot(
				const VectorUiCoord2& in_child_size,
				const VectorUiCoord2& in_child_pivot,
				const VectorUiCoord2& in_attach_point
				)
			{
				_has_child_slot_data = true;
				_child_size = in_child_size;
				_child_pivot = in_child_pivot;
				_attach_point = in_attach_point;
				return *this;
			}
			TComponentConstructionHelper& SetPadding(
				const UiCoord& in_padding_left,
				const UiCoord& in_padding_top,
				const UiCoord& in_padding_right,
				const UiCoord& in_padding_bottom
				)
			{
				_has_padding = true;
				_padding_left = in_padding_left;
				_padding_top = in_padding_top;
				_padding_right = in_padding_right;
				_padding_bottom = in_padding_bottom;
				return *this;
			}
		};
		static TComponentConstructionHelper MakeComponentDebugGrid();
		static TComponentConstructionHelper MakeComponentFill(const DscCommon::VectorFloat4& in_colour);
		static TComponentConstructionHelper MakeComponentImage(const std::shared_ptr<DscRenderResource::ShaderResource>& in_texture);
		static TComponentConstructionHelper MakeComponentCanvas(const DscCommon::VectorFloat4& in_clear_colour);

		struct TEffectConstructionHelper
		{
			TUiEffectType _effect_type;
			DscCommon::VectorFloat4 _effect_param;
			DscCommon::VectorFloat4 _effect_param_tint;
		};
		UiRootNodeGroup MakeRootNode(
			const TComponentConstructionHelper& in_construction_helper,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			const std::shared_ptr<UiRenderTarget>& in_ui_render_target,
			const std::vector<TEffectConstructionHelper>& in_effect_array = std::vector<TEffectConstructionHelper>()
			);
		static UiNodeGroup ConvertRootNodeGroupToNodeGroup(
			DscDag::DagCollection& in_dag_collection,
			const UiRootNodeGroup& in_ui_root_node_group
			);

		UiNodeGroup AddChildNode(
			const TComponentConstructionHelper& in_construction_helper,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			const UiRootNodeGroup& in_root_node_group,
			const UiNodeGroup& in_parent,
			const std::vector<TEffectConstructionHelper>& in_effect_array = std::vector<TEffectConstructionHelper>()
			DSC_DEBUG_ONLY(DSC_COMMA const std::string & in_debug_name = "")
		);

		/// also destroys all children, 
		//void DestroyRootNode(UiRootNodeGroup& in_root_node_group);
		/// we destoy the child, as it is not in a good way after being removed, a lot of it's links will be broken
		//void RemoveAndDestroyChild(const UiNodeGroup& in_parent, const UiNodeGroup& in_child)

		// no seperating update from draw as worried about not having the correct render size/ layout to consume input

		void Draw(
			const UiRootNodeGroup& in_root_node_group,
			DscDag::DagCollection& in_dag_collection,
			DscRenderResource::Frame& in_frame,
			const bool in_force_draw,
			const float in_time_delta,
			const UiInputState& in_input_state,
			DscRender::IRenderTarget* const in_external_render_target_or_null = nullptr
			);

	private:
		// so, if MakeDrawStack creates a UiRenderTaget, how does that get back into the parent, TUiNodeGroup::TUiRenderTarget
		DscDag::NodeToken MakeDrawStack(
			const TComponentConstructionHelper& in_construction_helper,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			const std::vector<TEffectConstructionHelper>& in_effect_array,
			const UiRootNodeGroup& in_root_node_group,
			DscDag::NodeToken in_last_render_target_or_null,
			DscDag::NodeToken in_render_request_size,
			DscDag::NodeToken in_child_array_node_or_null,
			UiComponentResourceNodeGroup& in_component_resource_group
			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		);

		DscDag::NodeToken MakeDrawNode(
			const TUiDrawType in_type,
			const TComponentConstructionHelper* const in_construction_helper_or_null,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::vector<DscDag::NodeToken>& in_array_input_stack,
			DscDag::NodeToken in_frame_node,
			DscDag::NodeToken in_ui_render_target_node,
			DscDag::NodeToken in_ui_scale,
			DscDag::NodeToken in_effect_param_or_null,
			DscDag::NodeToken in_effect_tint_or_null,
			DscDag::NodeToken in_child_array_node_or_null,
			UiComponentResourceNodeGroup& in_component_resource_group
			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		);

	private:
		/// dag resource hooks into the render system "callbacks" as to know when the device is restored
		std::unique_ptr < DscDagRender::DagResource> _dag_resource = {};

		std::shared_ptr<DscRenderResource::Shader> _debug_grid_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _ui_panel_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _image_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _fill_shader = {};

		std::shared_ptr<DscRenderResource::Shader> _effect_round_corner_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_drop_shadow_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_inner_shadow_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_stroke_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_tint_shader = {};

		// a full quad is of pos range [-1 ... 1] and uv range of [0 ... 1]
		std::shared_ptr<DscRenderResource::GeometryGeneric> _full_quad_pos_uv = {};
		// a full quad is of pos range [-1 ... 1]
		std::shared_ptr<DscRenderResource::GeometryGeneric> _full_quad_pos = {};
		// panel geometry is vertex of pos range [0 ... 1] intended to be affected by shader variables
		std::shared_ptr<DscRenderResource::GeometryGeneric> _ui_panel_geometry = {};

		std::unique_ptr<DscRenderResource::RenderTargetPool> _render_target_pool = {};
	};
}
