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
root node is paired with an externally supplied UiRenderTaget, which could be created from the backbuffer or an arbitary render texture, which provides the initial avalaible size for the layout

avaliable size:
a child node makes an avaliable size based on the parent avalable size, if it has child slot data, use that, if it has padding, use that to effect the avaliable size

desired size:
the child node uses that calculated avaliable size to make a desired size which is based on content, like the bounds of a text block, or the max of all children (geometry offset + geometry size)

geometry size: 
based on the parent component type, canvas geometry size is the avaliable size for the child. for a stack, it is the desired size for certain axis and avalaible size for other axis

geometry offset:
relative to the parent top left, where to position the geometry to draw on parent

render request size:
what size to request a render target to draw the child onto, max of desired and geometry size

scroll:
if the desired size is bigger than the the geometry size, automatically scroll the visible area of the child in the parent geometry window, can also be scrolled manually

note, some of the steps may feel redundant, but breaking up the steps so that a parent like a stack, can use the children geometry size in it's desired size caculation, based on it's own avaliable size handed down to the children
so trying to keep the steps descrete as to any avoid cyclic dependencies on various permutations of ui component types
*/
/*
if this ends up having performance issues on cost of the dynamic casts, then could remove the casts. 
could change the root node group to derrive of node group and make them solid classes with members of known type
additionally the Dag graph could be replaced by something similar with a base class with the dirty and output array but not virtual, and just implement a class for each variation
may be a bit of a mess for some of the switch functionality paths...

the number of render targets could also be a resource cost, possibly need a less memory intensive way of drawing all the steps/ reusing render targets better
*/
/*
may need to be careful of cross contamination of dirty against the draw chain and other chains of dirty
possible improvements is to have the MakeRootNode return a [RootNodeGroup, UiNodeGroup, DrawNode, DrawBase] but then end up with some mess of threading in the draw nodes for add child...
*/
namespace DscUi
{
	/*
	so, have a struct to hold DagNodeGroups, of just put more in the node groups?
	went with bloated node groupds, but that messed up the single chanel dirty of the node groups
	ie, fetching layoutsize was updating the render, if the entire node group was updated
	*/
	//struct UiNode
	//{
	//	DscDag::NodeToken _ui_node_group;
	//	DscDag::NodeToken _draw_node;
	//	DscDag::NodeToken _base_draw_node;
	//};

	class CelticKnot;
	class IUiComponent;
	class UiCoord;
	class UiInputParam;
	class UiInputState;
	class UiRenderTarget;
	class VectorUiCoord2;
	struct ComponentConstructionHelper;

	class UiManager
	{
	public:
		UiManager() = delete;
		UiManager& operator=(const UiManager&) = delete;
		UiManager(const UiManager&) = delete;

		struct TEffectConstructionHelper
		{
			TUiEffectType _effect_type = TUiEffectType::TCount;
			DscCommon::VectorFloat4 _effect_param = {};
			DscCommon::VectorFloat4 _effect_param_tint = {};
			bool _use_rollover_param_lerp = false;
			DscCommon::VectorFloat4 _effect_param_rollover = {};
			DscCommon::VectorFloat4 _effect_param_tint_rollover = {};
		};

		UiManager(
			DscRender::DrawSystem& in_draw_system, 
			DscCommon::FileSystem& in_file_system, 
			DscDag::DagCollection& in_dag_collection,
			const std::vector<TEffectConstructionHelper>& in_scrollbar_effect_array = std::vector<TEffectConstructionHelper>()
			);
		~UiManager();

		DscDag::NodeToken MakeRootNode(
			const ComponentConstructionHelper& in_construction_helper,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			const std::vector<TEffectConstructionHelper>& in_effect_array = std::vector<TEffectConstructionHelper>(),
			const UiCoord& in_scrollbar_thickness = UiCoord(16, 0.0f)
		);

		// what about when we want a child to be at an index? set child of "application layer set"? put optional index in construction helper
		DscDag::NodeToken AddChildNode(
			const ComponentConstructionHelper& in_construction_helper,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			DscDag::NodeToken in_root_node_group,
			DscDag::NodeToken in_parent,
			const std::vector<TEffectConstructionHelper>& in_effect_array = std::vector<TEffectConstructionHelper>()
			DSC_DEBUG_ONLY(DSC_COMMA const std::string & in_debug_name = "")
		);

		/// also destroys all children, but doesn't know about parent to unlink it, so main useage is for the root node
		void DestroyNode(
			DscDag::DagCollection& in_dag_collection,
			DscDag::NodeToken in_node_group
		);

		/// detach the child from the parent then recusivly destroy the child
		void RemoveDestroyChild(
			DscDag::DagCollection& in_dag_collection,
			DscDag::NodeToken in_parent,
			DscDag::NodeToken in_child_to_destroy
		);

		void Update(
			DscDag::NodeToken in_root_node_group,
			const float in_time_delta,
			const UiInputParam& in_input_param,
			const DscCommon::VectorInt2& in_layout_target_size
		);

		// return the UiRenderTarget of the full ui image, size is from Update::in_layout_target_size
		UiRenderTarget* const Draw(
			DscDag::NodeToken in_root_node_group,
			DscDag::DagCollection& in_dag_collection,
			DscRenderResource::Frame& in_frame
		);

		void DrawUiTextureToCurrentRenderTarget(
			DscRenderResource::Frame& in_frame,
			DscDag::NodeToken in_root_node_group,
			UiRenderTarget* const in_ui_texture,
			const DscCommon::VectorFloat4& in_tint_colour = DscCommon::VectorFloat4(1.0f, 1.0f, 1.0f, 1.0f)
		);

	private:
		// so, if MakeDrawStack creates a UiRenderTaget, how does that get back into the parent, TUiNodeGroup::TUiRenderTarget
		DscDag::NodeToken MakeDrawStack(
			const ComponentConstructionHelper& in_construction_helper,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			const std::vector<TEffectConstructionHelper>& in_effect_array,
			DscDag::NodeToken in_root_node_group,
			DscDag::NodeToken in_render_request_size,
			DscDag::NodeToken in_visible,
			DscDag::NodeToken in_child_array_node_or_null,
			DscDag::NodeToken in_component_resource_group,
			DscDag::NodeToken in_parent,
			DscDag::NodeToken& out_draw_base_node
			DSC_DEBUG_ONLY(DSC_COMMA const std::string & in_debug_name = "")
		);

		DscDag::NodeToken MakeDrawNode(
			const TUiDrawType in_type,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::vector<DscDag::NodeToken>& in_array_input_stack,
			DscDag::NodeToken in_frame_node,
			DscDag::NodeToken in_visible,
			DscDag::NodeToken in_ui_render_target_node,
			DscDag::NodeToken in_ui_render_target_node_b,
			DscDag::NodeToken in_ui_scale,
			DscDag::NodeToken in_effect_strength_or_null,
			DscDag::NodeToken in_effect_param_or_null,
			DscDag::NodeToken in_effect_tint_or_null,
			DscDag::NodeToken in_child_array_node_or_null,
			DscDag::NodeToken in_component_resource_group
			DSC_DEBUG_ONLY(DSC_COMMA const std::string & in_debug_name = "")
		);

		void AddScrollbar(
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			DscDag::NodeToken in_root_node_group,
			DscDag::NodeToken in_parent,
			DscDag::NodeToken in_node_to_scroll,
			DscDag::NodeToken in_pixel_traversal_node
			);

	private:
		/// dag resource hooks into the render system "callbacks" as to know when the device is restored
		std::unique_ptr<DscDagRender::DagResource> _dag_resource = {};
		std::unique_ptr<DscRenderResource::RenderTargetPool> _render_target_pool = {};
		std::unique_ptr<CelticKnot> _celtic_knot = {};
		std::vector<TEffectConstructionHelper> _scrollbar_effect_array = {};

		std::shared_ptr<DscRenderResource::Shader> _debug_grid_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _ui_panel_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _image_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _fill_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _gradient_fill_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _scroll_bar_shader = {};

		std::shared_ptr<DscRenderResource::Shader> _effect_round_corner_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_drop_shadow_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_inner_shadow_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_stroke_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_tint_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_burn_blot_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_burn_present_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_blur_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_desaturate_shader = {};

		// a full quad is of pos range [-1 ... 1] and uv range of [0 ... 1] (uv 0,0 top left)
		std::shared_ptr<DscRenderResource::GeometryGeneric> _full_quad_pos_uv = {};
		// a full quad is of pos range [-1 ... 1]
		std::shared_ptr<DscRenderResource::GeometryGeneric> _full_quad_pos = {};
		// panel geometry is vertex of pos range [0 ... 1] intended to be affected by shader variables
		std::shared_ptr<DscRenderResource::GeometryGeneric> _ui_panel_geometry = {};

	};
}
