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
		TGridFill
	};


	enum class TUiDrawType : uint8
	{
		TUiPanel, // array of chlidren
		TGridFill,
		TFill,
		TImage,
		TEffectDropShadow,
		TEffectInnerShadow,
		TEffectCorner,
		TEffectStroke,

		TCount
	};

	enum class TUiEffectType : uint8
	{
		TEffectDropShadow,
		TEffectInnerShadow,
		TEffectCorner,
		TEffectStroke
	};

	enum class TUiRootNodeGroup : uint8
	{
		TDrawNode,
		TUiComponentType,
		TArrayChildUiNodeGroup,
		TForceDraw, // the draw method sets this if at least the top level render needs to run, useful if something else is writing to the render target
		TUiRenderTarget, // UiTexture passed in with creation of the root node, and pass in an otional IRenderTarget on draw. if the client want to update the UiTexture (reference to back buffer texture?)
		TRenderTargetViewportSize,
		TScreenSpaceSize, // from top left as 0,0, what is our on screen geometry footprint
		TUiScale,
		TEffectParamArray, // std::vector<DagNodeValue<VectorFloat4>>> two vectfloat4 for each effect?

		TFrame, // no dirty on set
		TTimeDelta, // dirty if not zero
		TInputState, // dirty if 

		TCount
	};
	typedef DscDag::DagGroup<TUiRootNodeGroup, static_cast<std::size_t>(TUiRootNodeGroup::TCount)> UiRootNodeGroup;
} // DscUi
template <>
const DscDag::DagGroupNodeMetaData& DscDag::GetDagGroupMetaData(const DscUi::TUiRootNodeGroup in_value);

namespace DscUi
{
	// is there a node which is the array of children nodes? or the array of 

	enum class TUiNodeGroup : uint8
	{
		TDrawNode, // returns a std::shared_ptr<RenderTargetTexture> _render_target_texture (some draw functions need texture size? or no) could this return a shared shader resource (texture)?
		TUiComponentType,
		TArrayChildUiNodeGroup,
		TUiRenderTarget,
		TRenderTargetSize,
		TScreenSpaceSize, // from top left as 0,0, what is our on screen geometry footprint
		TGeometrySize,
		TScrollPos, // where is the geometry size quad is on the render target texture
		TClearColour,
		THasManualScrollX,
		TManualScrollX,
		THasManualScrollY,
		TManualScrollY,
		TEffectParamArray, // std::vector<DagNodeValue<VectorFloat4>>>

		TCount
	};
	typedef DscDag::DagGroup<TUiNodeGroup, static_cast<std::size_t>(TUiNodeGroup::TCount)> UiNodeGroup;
} // DscUi

template <>
const DscDag::DagGroupNodeMetaData& DscDag::GetDagGroupMetaData(const DscUi::TUiNodeGroup in_value);
