#pragma once
#include "dsc_ui.h"
#include "ui_coord.h"
#include <dsc_dag/dag_node_group.h>

namespace DscDag
{
	template <typename ENUM, std::size_t SIZE>
	class DagGroup;
}

namespace DscText
{
	class TextManager;
	class TextRun;
}

namespace DscUi
{
	enum class TUiFlow : uint8
	{
		THorizontal,
		TVertical,
		TCount
	};

	enum class TUiScrollbarAxis : uint8
	{
		TNone = 0,
		THorizontal = 1 << 0,
		TVertical = 2 << 0
	};
}
const DscUi::TUiScrollbarAxis operator& (const DscUi::TUiScrollbarAxis in_lhs, const DscUi::TUiScrollbarAxis in_rhs);
const bool operator!= (const int32 in_lhs, const DscUi::TUiScrollbarAxis in_rhs);

namespace DscUi
{

	/// will these need to be flags, allow "shift control right click"? start with simple
	enum class TUiTouchFlavour : uint8
	{
		TNone,
		TMouseLeft,
		TMouseRight,
	};

	enum class TUiInputStateFlag : uint8
	{
		TNone,
		TRollover = 1 << 0, // use rollover for keyboard navigation?
		TClick = 1 << 1,
		TSelection = 1 << 2,
		//TVisualMask = TRollover | TClick | TSelection,

		//TDisable = 1 << 3,
		//TAbortInputTraveral = 1 << 4,

		// flashing white on click start/ end seems like a good idea but looks kind of flashy, won't do this for now
		// is this the frame the click started, should also have TClick set
		//TClickStart = 1 << 3,
		// did click end this frame, can be true without TClick set
		//TClickEnd = 1 << 4
	};
}

const DscUi::TUiInputStateFlag operator &= (DscUi::TUiInputStateFlag& in_out_lhs, const DscUi::TUiInputStateFlag in_rhs);
const DscUi::TUiInputStateFlag operator |= (DscUi::TUiInputStateFlag& in_out_lhs, const DscUi::TUiInputStateFlag in_rhs);
const DscUi::TUiInputStateFlag operator | (const DscUi::TUiInputStateFlag in_lhs, const DscUi::TUiInputStateFlag in_rhs);
const DscUi::TUiInputStateFlag operator& (const DscUi::TUiInputStateFlag in_lhs, const DscUi::TUiInputStateFlag in_rhs);
const DscUi::TUiInputStateFlag operator~ (const DscUi::TUiInputStateFlag in_value);
const bool operator!= (const int32 in_lhs, const DscUi::TUiInputStateFlag in_rhs);

namespace DscUi
{
	// do we clear the TUiInputStateFlag flag on ignored/skipped over nodes of the hierarchy
	enum class TUiInputFlowBehaviour : uint8
	{
		TNormal,
		TIgnore, // do not process this node or it's children
		TModal, // deal with this node and children, but ignore node traversal of the remaining hierarchy
	};

	/// should this just be changed to directly using windows virtual key id? or vaugely allow remapping
	/// This was intended for ui navigation control, but what if we want general key presses to be comunicated with some component
	enum class TUiNavigationType : uint8
	{
		TNone,
		TLeft, // keys have amount 1?
		TRight,
		TUp,
		TDown,
		TScroll, //mouse scroll has an ammount? or just add an amount to everything, allows dpad?
		TDpadHorizontal,
		TDpadVertical
	};

	enum class TUiComponentType : uint8
	{
		TCanvas,
		TCelticKnotFill,
		TCrossFade, // one child is set to be active and fade in, another array of children to fade out. up to child if it removes tself on crossfade == 0?
		TDebugGrid,
		TFill,
		TGradientFill,
		TImage,
		TScrollBar,
		TStack,
		TText,
		// Todo, merge TTextNode into TText
		TTextNode, // alternative data path for TText, but from a DagNode returning a shared_ptr<TextRun>

		// there is no button component, components are for layout and draw style
		//TButton, // has input, filter draw of children for input flag if they have a for_input_flag node
	};

	enum class TUiDrawType : uint8
	{
		TUiPanel, // array of chlidren
		TDebugGrid,
		TFill,
		TGradientFill,
		TCelticKnotFill,
		TImage,
		TText,
		TScrollBar,
		TEffectDropShadow,
		TEffectInnerShadow,
		TEffectCorner,
		TEffectStroke,
		TEffectTint,
		TEffectBurnBlot,
		TEffectBurnPresent,
		TEffectBlur,
		TEffectDesaturate,

		TCount
	};

	enum class TUiEffectType : uint8
	{
		TEffectDropShadow,
		TEffectInnerShadow,
		TEffectCorner,
		TEffectStroke,
		TEffectTint,
		TEffectBurnBlot,
		TEffectBurnPresent,
		TEffectBlur,
		TEffectDesaturate,

		TCount
	};

	struct ScreenSpace
	{
		/// top left, top right, bottom left, bottom right in mouse coord relative to ui root top left
		/// if our nodes' geometry size was full size on screen, where would it be. ie, easily test if touch intersects with node
		DscCommon::VectorFloat4 _screen_space = {};
		// the [top left, top right, bottom left, bottom right] bounds of valid screen area (
		DscCommon::VectorFloat4 _screen_valid = {};

		const bool operator==(const ScreenSpace& in_rhs) const;
	};

	//enum class TUiNode : uint8
	//{
	//	TDrawNode, // returns a std::shared_ptr<RenderTargetTexture> _render_target_texture (some draw functions need texture size? or no) could this return a shared shader resource (texture)?
	//	TDrawBaseNode, // TDrawNode may be the ui component or an effect, if we want to add a dependency, it needs to be to the base of the draw chain
	//	TUiNodeGroup,

	//	TCount
	//};

	// the collection of data for a non root node, see TUiComponentResourceNodeGroup for values provided to a client to manipulate
	// this is ment to be the minimal set of values for the UiManager to handle layout
	enum class TUiNodeGroup : uint8
	{
		TDrawNode, // returns a RenderTargetTexture* of the last thing drawn by the ui node (ie, could be a effect, or the results of the node draw)
		TDrawBaseNode, // TDrawNode may be the ui component or an effect, if we want to add a dependency, it needs to be to the base of the draw chain. ie, children draw nodes add themselves as inputs to the base draw
		TUiComponentType,
		TUiComponentResources, // somewhere to access the text run or other resources kept for the component, an array of nodes? node group? hold the effect param?
		TArrayChildUiNodeGroup, // so, there is an issue with this, handing around a COPY of a UiNodeGroup is a slight risk of stale data if reference is held elsewhere, use with care
		TScreenSpace, // struct ScreenSpace

		TAvaliableSize, // the initial layout size we were told by the parent that we had avaliable
		TRenderRequestSize, // the size that is used to request the render target from the resource pool (or the viewport size of the external render target given to the top level node) (max of desired size and geometry size)
		TGeometryOffset, // public so parent can panel draw this node
		TGeometrySize, // public so parent can panel draw this node
		TScrollPos, // where is the geometry size quad is on the render target texture
		TUiPanelShaderConstantBuffer, // keep on hand the resources for any child to draw in a parent canvas or similar, or should this be in TUiComponentResourceNodeGroup or TUiNodeGroup?
		// trying to have everything to do with layout, or for parent to draw, in the TUiNodeGroup layer
		// for everything else, put into the TUiComponentResources UiComponentResourceNodeGroup
		TUiPanelTint, // for crossfade, start with a VectorFloat4, but may neet to change to a TUiPanelShaderConstantBufferPS?

		TVisible, // for scroll bar, possibly an externally supplied node of if we are visible or not. in ui panel draw, skip children with disabled ui render targets...

		TCount
	};

	// extend TUiNodeGroup with some root node extras
	enum class TUiRootNodeGroup : uint8
	{
		//TForceDraw = static_cast<uint8>(TUiNodeGroup::TCount), // the draw method sets this if at least the top level render needs to run, useful if something else is writing to the render target
		TRenderTargetViewportSize = static_cast<uint8>(TUiNodeGroup::TCount),
		//TUiPanelShaderConstantBuffer,
		TUiScale,
		TFrame, // no dirty on set
		TTimeDelta, // dirty if not zero
		TInputState, // hold state of what node had click down started, 
		TScrollBarThickness, // the width for vertical scroll bar, or height for vertical, ui coord using TRenderTargetViewportSize and ui scale

		TCount
	};

} // DscUi

#if defined(_DEBUG)
//template <>
//const DscDag::DagNodeGroupMetaData& DscDag::GetDagNodeGroupMetaData(const DscUi::TUiNode in_value);
template <>
const DscDag::DagNodeGroupMetaData& DscDag::GetDagNodeGroupMetaData(const DscUi::TUiNodeGroup in_value);
template <>
const DscDag::DagNodeGroupMetaData& DscDag::GetDagNodeGroupMetaData(const DscUi::TUiRootNodeGroup in_value);
#endif //#if defined(_DEBUG)

namespace DscUi
{
	struct TUiComponentTextData
	{
		std::shared_ptr<DscText::TextRun> _text_run = {};
		DscText::TextManager* _text_manager = nullptr;
	};
	struct TUiComponentScrollbarData
	{
		TUiScrollbarAxis _scrollbar_axis_flag = TUiScrollbarAxis::TNone;
		UiCoord _scrollbar_knot_min_dimention_x = UiCoord();
		UiCoord _scrollbar_knot_min_dimention_y = UiCoord();
	};

	//this is ment as the data store for a UiComponent, but we replaced the UiComponent with a dag node/ node group...
	enum class TUiComponentResourceNodeGroup : uint8
	{
		//TArrayOwnedNodes, //what nodes do we need to delete if this component is removed, try to list every created node

		//TUiRenderTarget, // the root has a token for this, but nothing else directly need this, just have node ownership into the TArrayOwnedNodes, and ref by required draw nodes
		// don't animate ClearColour, is used as the clear colour value with the render target texture. make better to animate a effect tint param?
		TClearColour,
		TUiScale, // either a ref to the root ui scale, or a UiScale created by the TWidthUiScale
		TFillColour,
		TTexture,

		TText,

		TFlow,
		TGap,

		TGradientFill,

		// moving away from this, we hook things up if needed, not search for things latter
		// ie, if we have an animated effect param, we hook it up to the flag to turn the animation on/ off, and when on, link to the time delta
		//TEffectParamArray, // only if there are effects, currently "n x [effect param, effect tint]"

		THasManualScrollX,
		TManualScrollX,
		THasManualScrollY,
		TManualScrollY,

		// pass these down directly with scroll bar creation
		//TRenderBiggerThanGeometrySizeX,
		//TRenderBiggerThanGeometrySizeY,

		//TScrollBarShaderConstantBuffer, // this could just be an input node to the draw calculate node
		TScrollBarKnotTint,
		TScrollBarData, // axis flag, min knot size, step
		TScrollBarWriteX,
		TScrollBarRangeReadX,
		TScrollBarWriteY,
		TScrollBarRangeReadY,

		// child slots of presumably a canvas parent, but there may be things other than a canvas that can hold child slots?
		TChildSlotSize,
		TChildSlotPivot,
		TChildSlotParentAttach,

		TDesiredSize, // name too close to the TUiNodeGroup? but it doesnt have a desired size

		// lower priority than child slots, but still modify the avaliable(layout) size handed down by the parent
		TPaddingLeft,
		TPaddingTop,
		TPaddingRight,
		TPaddingBottom,

		// child stack data
		TChildStackSize,
		TChildStackPivot,
		TChildStackParentAttach,

		TInputStateFlag,
		TInputFlowBehaviour,
		TInputData,
		TInputRolloverAccumulate, // [0.0 ... 1.0] over some time period while acumulates to 1 when node is rolled over and drain back to zero when not rolled over
		TInputActiveTouchPos, // float x, y, of cursor pixel coord from panel top left

		TCrossfadeActiveChild, // use the DagNodeGroup<TUiNodeGroup> node to identify which child UiNodeGroup to fade up to full visiblity
		TCrossfadeChildAmount, // for the child of the cross fade, the amount of fade (alpha)
		//TCrossfadeCondition, // true if the fade is active
		//TCrossfadeNode, // the crossfade node that needs to be set as an input to the crossfade base draw

		TCelticKnotSize,
		TCelticKnotTint,

		TEffectStrength, // [0.0f ... 1.0] effect strength

		TSelectedChildIndex, // for the parent of the selected set of items
		TSelectedItemIndex, // for the child of the selected set, the index of the item in the selection set
		TSelectedItemSelected, // for the child of the selected set, a true, false value for if we are the selected item

		TCount
	};
	typedef DscDag::DagGroup<TUiComponentResourceNodeGroup, static_cast<std::size_t>(TUiComponentResourceNodeGroup::TCount)> UiComponentResourceNodeGroup;
}

#if defined(_DEBUG)
template <>
const DscDag::DagNodeGroupMetaData& DscDag::GetDagNodeGroupMetaData(const DscUi::TUiComponentResourceNodeGroup in_value);
#endif //#if defined(_DEBUG)

namespace DscUi
{
	struct TUiComponentInputData
	{
		// clicked node, node relative click pos
		std::function<void(DscDag::NodeToken, const DscCommon::VectorFloat2&)> _click_callback = {};
		// node relative click start, node relative click current. use node token to get current node size via screen coords?
		std::function<void(DscDag::NodeToken, const DscCommon::VectorFloat2&, const DscCommon::VectorFloat2&)> _drag_callback = {};

		// use a different node for TUiInputStateFlag
		//bool _rollover = false;
		//// is there a click active on this node
		//bool _active_click = false;
		//bool _first_frame_click = false;
	};
} // DscUi