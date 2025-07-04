#pragma once
#include "dsc_ui.h"
#include <dsc_dag/dag_group.h>

namespace DscDag
{
	template <typename ENUM, std::size_t SIZE>
	class DagGroup;
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
		TCanvas
	};


	enum class TUiDrawType : uint8
	{
		TUiPanel, // array of chlidren
		TDebugGrid,
		TFill,
		TImage,
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
		TArrayChildUiNodeGroup,
		TAvaliableSize, // the initial layout size we were told by the parent that we had avaliable
		TRenderRequestSize, // the size that is used to request the render target from the resource pool (or the viewport size of the external render target given to the top level node) (max of desired size and geometry size)
		TGeometryOffset, // public so parent can panel draw this node
		TGeometrySize, // public so parent can panel draw this node
		TScrollPos, // where is the geometry size quad is on the render target texture
		TScreenSpaceSize, // from top left as 0,0, what is our on screen geometry footprint. for example, this is in mouse space, so if mouse is at [500,400] we want to know if it is inside our screen space to detect rollover

		TCount
	};
	typedef DscDag::DagGroup<TUiNodeGroup, static_cast<std::size_t>(TUiNodeGroup::TCount)> UiNodeGroup;
} // DscUi

template <>
const DscDag::DagGroupNodeMetaData& DscDag::GetDagGroupMetaData(const DscUi::TUiNodeGroup in_value);

namespace DscUi
{
	//this is ment as the data store for a UiComponent, but we replaced the UiComponent with a dag node/ node group...
	enum class TUiComponentResourceNodeGroup : uint8
	{
		//TUiRenderTarget, // the root has a token for this, but nothing else directly need this, just have node ownership into the TArrayOwnedNodes
		// don't animate ClearColour, is used as the clear colour value with the render target texture. make better to animate a effect tint param?
		TClearColour,
		TFillColour,
		TTexture,
		TEffectParamArray,
		THasManualScrollX,
		TManualScrollX,
		THasManualScrollY,
		TManualScrollY,

		TUiPanelShaderConstantBuffer, // keep on hand the resources for any child to draw in a parent canvas or similar, or should this be in TUiNodeGroup?
		
		TArrayOwnedNodes, //what nodes to delete if this component is removed

		TCount
	};
	typedef DscDag::DagGroup<TUiComponentResourceNodeGroup, static_cast<std::size_t>(TUiComponentResourceNodeGroup::TCount)> UiComponentResourceNodeGroup;
}

template <>
const DscDag::DagGroupNodeMetaData& DscDag::GetDagGroupMetaData(const DscUi::TUiComponentResourceNodeGroup in_value);

