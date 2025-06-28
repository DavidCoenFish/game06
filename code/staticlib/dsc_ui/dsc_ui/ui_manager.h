#pragma once
#include "dsc_ui.h"
#include "ui_enum.h"

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
	class Shader;
	class ShaderConstantBuffer;
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
* example: canvas, 2 children
externally provided render target (R0)
root ui node 0 (N0)
	component (C0), canvas

ui node 1 child (N1)
	component (C1), debug fill?
	render target of desired size (R1)
	calculate node that updates (R1) with correct contents of any input change

ui node 2 child (N2)
	component (C2), fill?
	render target of desired size (R2)
	calculate node that updates (R2) with correct contents of any input change

UiManager::Draw
	invokes get value on (N0)
		invokes get value on (N1)
			draw (C1)
				set render target (R1)
				debug fill
		invokes get value on (N2)
			draw (C2)
				set render target (R2)
				fill
		draw (C0)
			set render target (R0)
			use ui panel to draw texture R1 to R0
			use ui panel to draw texture R2 to R0


	size calculation
from parent, what is our avaliable size
from our component, given the avaliable size, what is our desired size
from parent, given child index and parent avaliable size, what is the geometry offset 
	// parent avaliable size for offset calculation, trying to avoid cyclic dependecy, but offset more relevant for canvas? can stack be funky with horizontal pivot to make centered stack?
from parent, given avaliable and desired size, what is the geometry size 
	// what is the case for this to not be the avalaible size? if desired size is bigger than avaliable? what about shrinking, stack may want to use desired size, a dialog may want min avaliable and desired?
	// is this a parent or a child determined size? parent is doing layout?
for a stack component parent, the parent can now calculate it's desired size? given each child geometry offset and size
to calculate geometry/ shader constant buffer [parent render size, geometry offset, geometry size, 

//[avaliable size, desired size] -> [render size] // redundant? always min? parent can always shring geometry size if we want to shrink?

// logic check, using [desired size] as render target size
[parent ui component, parent child index] -> [avaliable size]
[ui component, avaliable size] -> [desired size]
[parent ui component, parent child index] -> [geometry offset]
[parent ui component, parent child index, desired size] -> [geometry size]
[parent.desired size, geometry size, geometry offset, desired size, scroll] -> [shader constants]
[ui component] -> [clear colour]
[desired size, geometry size, render target pool, clear colour] -> [render target]  // if desired size is smaller than geometry size, expand to geometry size
[all the inputs via enum UiRootNodeInputIndex or UiNodeInputIndex] -> [Render target Shader Resource]

// the UiComponent has logic for if scroll is automatic or manual, and only sets [Scroll] value appropriatly. 
// alternativly, need a way of dag conditional to not dirty decendants on dead branch. (ie, updating tick would dirty draw, even if not auto scroll)
// following this, then a force update set of node that conditionally write to other nodes may be needed? for now just do the tick in the UiComponent and work out DagNode split latter?

IUiComponent::Draw(frame, IRenderTarget) // called once render target is set, expect children to be previously update/ rendered
IUiComponent::SetScrollNode(NodeToken)
IUiComponent::SetScrollPixelTraveralDistance(VectorInt2)
IUiComponent::Update(const float in_time_delta)
IUiComponent::SetDesiredSizeNode(NodeToken) // for making shader constants for ui panel draw
IUiComponent::SetDrawNode(NodeToken) // for parent to get the render target shader resource to draw as texture

*/

namespace DscUi
{
	class IUiComponent;
	class UiCoord;
	class VectorUiCoord2;

	class UiManager
	{
	public:
		UiManager() = delete;
		UiManager& operator=(const UiManager&) = delete;
		UiManager(const UiManager&) = delete;

		//DscDag::DagCollection
		UiManager(DscRender::DrawSystem& in_draw_system, DscCommon::FileSystem& in_file_system, DscDag::DagCollection& in_dag_collection);
		~UiManager();

		std::unique_ptr<IUiComponent> MakeComponentDebugFill(DscRender::DrawSystem& in_draw_system);
		std::unique_ptr<IUiComponent> MakeComponentFill();
		std::unique_ptr<IUiComponent> MakeComponentCanvas();
		std::unique_ptr<IUiComponent> MakeComponentText(
			DscText::TextManager& in_text_manager,
			std::unique_ptr<DscText::TextRun>&& in_text_run,
			const TUiComponentBehaviour in_behaviour
			);
		std::unique_ptr<IUiComponent> MakeComponentStack(
			const UiCoord& in_item_gap,
			const TUiFlow in_ui_flow
		);
		std::unique_ptr<IUiComponent> MakeComponentMargin(
			const UiCoord& in_left,
			const UiCoord& in_top,
			const UiCoord& in_right,
			const UiCoord& in_bottom
		);

		DagGroupUiRootNode MakeUiRootNode(
			DscDag::DagCollection& in_dag_collection,
			std::unique_ptr<IUiComponent>&& in_component

			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		);

		static DagGroupUiParentNode ConvertUiRootNodeToParentNode(const DagGroupUiRootNode& in_ui_root_node_group);

		// calls through to MakeUiNode, and on adding a child to a parent is when the ui component gets its ParentChildIndex set?
		DagGroupUiParentNode MakeUiNodeCanvasChild(
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::unique_ptr<IUiComponent>&& in_component,
			const DscCommon::VectorFloat4& in_clear_colour,

			const DagGroupUiRootNode& in_root_node,
			const DagGroupUiParentNode& in_parent_node,

			const VectorUiCoord2& in_child_size, 
			const VectorUiCoord2& in_child_pivot, 
			const VectorUiCoord2& in_attach_point

			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		);

		// calls through to MakeUiNode, and on adding a child to a parent is when the ui component gets its ParentChildIndex set?
		DagGroupUiParentNode MakeUiNodeStackChild(
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::unique_ptr<IUiComponent>&& in_component,
			const DscCommon::VectorFloat4& in_clear_colour,

			const DagGroupUiRootNode& in_root_node,
			const DagGroupUiParentNode& in_parent_node

			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		);

		// calls through to MakeUiNode, and on adding a child to a parent is when the ui component gets its ParentChildIndex set?
		DagGroupUiParentNode MakeUiNodeMarginChild(
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::unique_ptr<IUiComponent>&& in_component,
			const DscCommon::VectorFloat4& in_clear_colour,

			const DagGroupUiRootNode& in_root_node,
			const DagGroupUiParentNode& in_parent_node

			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		);

		// could be combined with the DrawUiSystem, but doesn't need to be
		void UpdateUiSystem(
			DagGroupUiRootNode& in_ui_root_node_group, // not const as setting values on it
			const float in_time_delta
			// input state? keys down, touch pos, gamepad
			);

		void DrawUiSystem(
			DagGroupUiRootNode& in_ui_root_node_group, // not const as resolve conditional nodes may change data...
			DscRender::IRenderTarget* const in_render_target,
			DscRenderResource::Frame& in_frame,
			const bool in_force_top_level_draw, // if this render target is shared, need to at least redraw the top level ui
			const bool in_clear_on_draw, // clear the top level render target before we draw to it
			const float in_ui_scale = 1.0f
		);

	private:
		// if this has a reason to be public, then lets make it puplic
		// on adding a child to a parent is when it's clear colour is set...
		DagGroupUiParentNode MakeUiNode(
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::unique_ptr<IUiComponent>&& in_component,
			const DscCommon::VectorFloat4& in_clear_colour,

			const DagGroupUiRootNode& in_root_node,
			const DagGroupUiParentNode& in_parent_node

			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
		);

	private:
		/// dag resource hooks into the render system "callbacks" as to know when the device is restored
		std::unique_ptr < DscDagRender::DagResource> _dag_resource = {};
		
		std::shared_ptr<DscRenderResource::Shader> _debug_grid_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _ui_panel_shader = {};

		std::shared_ptr<DscRenderResource::GeometryGeneric> _full_target_quad = {};
		std::shared_ptr<DscRenderResource::GeometryGeneric> _ui_panel_geometry = {};

		std::unique_ptr<DscRenderResource::RenderTargetPool> _render_target_pool = {};

	};
}