#pragma once
#include "dsc_ui.h"
#include <dsc_dag/dag_group.h>

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

	enum class TUiComponentBehaviour : uint32
	{
		TNone = 0,
		//TVisible = 1 << 0, ? why have the node in the tree if it is invisible? convienience? or use UiComponentSwitch/ UiComponentCrossfade for that?
		THasManualScrollX = 1 << 0,
		THasManualScrollY = 1 << 1,

		TDisabled = 1 << 2,
		// or are these inputs/ state?
		//TRollover = 1 << 3,
		//TClicked = 1 << 4
	};

	enum class TUiComponentType : uint8
	{
		TDebugGrid,
		TFill,
		TImage,
		TCanvas,
		TText
	};


	enum class TUiDrawType : uint8
	{
		TUiPanel, // array of chlidren
		TDebugGrid,
		TFill,
		TImage,
		TText,
		TEffectDropShadow,
		TEffectInnerShadow,
		TEffectCorner,
		TEffectStroke,
		TEffectTint,

		TCount
	};

	enum class TUiEffectType : uint8
	{
		TEffectDropShadow,
		TEffectInnerShadow,
		TEffectCorner,
		TEffectStroke,
		TEffectTint
	};

	enum class TUiRootNodeGroup : uint8
	{
		TDrawNode,
		TUiComponentType,
		TUiComponentResources, // somewhere to access the text run or other resources kept for the component, an array of nodes? node group? hold the effect param?
		TArrayChildUiNodeGroup,
		TForceDraw, // the draw method sets this if at least the top level render needs to run, useful if something else is writing to the render target
		TRenderTargetViewportSize,
		TScreenSpaceSize, // from top left as 0,0, what is our on screen geometry footprint
		TUiScale,

		TFrame, // no dirty on set
		TTimeDelta, // dirty if not zero
		TInputState, // dirty if 

		TUiRenderTarget, // UiTexture passed in with creation of the root node, and pass in an otional IRenderTarget on draw. if the client want to update the UiTexture (reference to back buffer texture?)

		TCount
	};
	typedef DscDag::DagGroup<TUiRootNodeGroup, static_cast<std::size_t>(TUiRootNodeGroup::TCount)> UiRootNodeGroup;
} // DscUi
template <>
const DscDag::DagGroupNodeMetaData& DscDag::GetDagGroupMetaData(const DscUi::TUiRootNodeGroup in_value);

namespace DscUi
{
	// the collection of data for a non root node, see TUiComponentResourceNodeGroup for values provided to a client to manipulate
	// this is ment to be the minimal set of values for the UiManager to handle layout
	enum class TUiNodeGroup : uint8
	{
		TDrawNode, // returns a std::shared_ptr<RenderTargetTexture> _render_target_texture (some draw functions need texture size? or no) could this return a shared shader resource (texture)?
		TUiComponentType,
		TUiComponentResources, // somewhere to access the text run or other resources kept for the component, an array of nodes? node group? hold the effect param?
		TArrayChildUiNodeGroup, // so, there is an issue with this, handing around a COPY of a UiNodeGroup is a slight risk of stale data if reference is held elsewhere, use with care
		TAvaliableSize, // the initial layout size we were told by the parent that we had avaliable
		TRenderRequestSize, // the size that is used to request the render target from the resource pool (or the viewport size of the external render target given to the top level node) (max of desired size and geometry size)
		TGeometryOffset, // public so parent can panel draw this node
		TGeometrySize, // public so parent can panel draw this node
		TScrollPos, // where is the geometry size quad is on the render target texture
		TScreenSpaceSize, // from top left as 0,0, what is our on screen geometry footprint. for example, this is in mouse space, so if mouse is at [500,400] we want to know if it is inside our screen space to detect rollover
		TUiPanelShaderConstantBuffer, // keep on hand the resources for any child to draw in a parent canvas or similar, or should this be in TUiComponentResourceNodeGroup or TUiNodeGroup?
		// trying to have everything to do with layout, or for parent to draw, in the TUiNodeGroup layer
		// for everything else, put into the TUiComponentResources UiComponentResourceNodeGroup

		TCount
	};
	typedef DscDag::DagGroup<TUiNodeGroup, static_cast<std::size_t>(TUiNodeGroup::TCount)> UiNodeGroup;
} // DscUi

template <>
const DscDag::DagGroupNodeMetaData& DscDag::GetDagGroupMetaData(const DscUi::TUiNodeGroup in_value);

namespace DscUi
{
	struct TUiComponentTextData
	{
		std::shared_ptr<DscText::TextRun> _text_run = {};
		DscText::TextManager* _text_manager = nullptr;
	};
	//struct TUiComponentWidthUiScale
	//{
	//	int32 _scale_width_low_threashhold = 0; // example 800
	//	float _scale_factor = 0.0f; // 0.0015789 for scale of 4.8 when width is 3040 more than 800,
	//};

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
		//TWidthUiScale,

		TEffectParamArray, // only if there are effects, currently "n x [effect param, effect tint]"
		THasManualScrollX,
		TManualScrollX,
		THasManualScrollY,
		TManualScrollY,

		// child slots of presumably a canvas parent, but there may be things other than a canvas that can hold child slots?
		TChildSlotSize,
		TChildSlotPivot,
		TChildSlotParentAttach,

		// lower priority than child slots, but still modify the avaliable(layout) size handed down by the parent
		TPaddingLeft,
		TPaddingTop,
		TPaddingRight,
		TPaddingBottom,

		TCount
	};
	typedef DscDag::DagGroup<TUiComponentResourceNodeGroup, static_cast<std::size_t>(TUiComponentResourceNodeGroup::TCount)> UiComponentResourceNodeGroup;
}

template <>
const DscDag::DagGroupNodeMetaData& DscDag::GetDagGroupMetaData(const DscUi::TUiComponentResourceNodeGroup in_value);

