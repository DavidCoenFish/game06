#include "ui_enum.h"
#include "ui_render_target.h"
#include "ui_input_state.h"
#include "vector_ui_coord2.h"
#include <dsc_dag/dag_group.h>
#include <dsc_render_resource/shader_resource.h>
#include <dsc_render_resource/shader_constant_buffer.h>

const DscUi::TUiInputStateFlag operator |= (DscUi::TUiInputStateFlag& in_out_lhs, const DscUi::TUiInputStateFlag in_rhs)
{
	const int32 temp = static_cast<int32>(in_out_lhs) | static_cast<int32>(in_rhs);
	in_out_lhs = static_cast<DscUi::TUiInputStateFlag>(temp);
	return in_out_lhs;
}

const DscUi::TUiInputStateFlag operator | (const DscUi::TUiInputStateFlag in_lhs, const DscUi::TUiInputStateFlag in_rhs)
{
	const int32 temp = static_cast<int32>(in_lhs) | static_cast<int32>(in_rhs);
	return static_cast<DscUi::TUiInputStateFlag>(temp);
}

const DscUi::TUiInputStateFlag operator& (const DscUi::TUiInputStateFlag in_lhs, const DscUi::TUiInputStateFlag in_rhs)
{
	const int32 temp = static_cast<int32>(in_lhs) & static_cast<int32>(in_rhs);
	return static_cast<DscUi::TUiInputStateFlag>(temp);
}

const bool operator!= (const int32 in_lhs, const DscUi::TUiInputStateFlag in_rhs)
{
	return in_lhs != static_cast<int32>(in_rhs);
}

template class DscDag::DagGroup<DscUi::TUiRootNodeGroup, static_cast<std::size_t>(DscUi::TUiRootNodeGroup::TCount)>;

template class DscDag::DagGroup<DscUi::TUiNodeGroup, static_cast<std::size_t>(DscUi::TUiNodeGroup::TCount)>;

template <>
const DscDag::DagGroupNodeMetaData& DscDag::GetDagGroupMetaData(const DscUi::TUiRootNodeGroup in_value)
{
	switch (in_value)
	{
	default:
		DSC_ASSERT_ALWAYS("invalid switch");
		break;
	case DscUi::TUiRootNodeGroup::TDrawNode:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscUi::UiRenderTarget*) };
		return s_meta_data;
	}
	case DscUi::TUiRootNodeGroup::TDrawBaseNode:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscUi::UiRenderTarget*) };
		return s_meta_data;
	}
	case DscUi::TUiRootNodeGroup::TUiComponentType:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscUi::TUiComponentType) };
		return s_meta_data;
	}
	case DscUi::TUiRootNodeGroup::TUiComponentResources:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscUi::UiComponentResourceNodeGroup) };
		return s_meta_data;
	}
	case DscUi::TUiRootNodeGroup::TArrayChildUiNodeGroup:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(std::vector<DscUi::UiNodeGroup>) };
		return s_meta_data;
	}
	case DscUi::TUiRootNodeGroup::TForceDraw:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(bool) };
		return s_meta_data;
	}
	case DscUi::TUiRootNodeGroup::TUiRenderTarget:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(std::shared_ptr<DscUi::UiRenderTarget>) };
		return s_meta_data;
	}
	case DscUi::TUiRootNodeGroup::TRenderTargetViewportSize:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscCommon::VectorInt2) };
		return s_meta_data;
	}
	case DscUi::TUiRootNodeGroup::TScreenSpace:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscUi::ScreenSpace) };
		return s_meta_data;
	}
	case DscUi::TUiRootNodeGroup::TUiScale:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(float) };
		return s_meta_data;
	}
	case DscUi::TUiRootNodeGroup::TFrame:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscRenderResource::Frame*) };
		return s_meta_data;
	}
	case DscUi::TUiRootNodeGroup::TTimeDelta:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(float) };
		return s_meta_data;
	}
	case DscUi::TUiRootNodeGroup::TInputState:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscUi::UiInputState) };
		return s_meta_data;
	}
	}
	static DscDag::DagGroupNodeMetaData s_dummy = { false, typeid(nullptr) };
	return s_dummy;
}

template <>
const DscDag::DagGroupNodeMetaData& DscDag::GetDagGroupMetaData(const DscUi::TUiNodeGroup in_value)
{
	switch (in_value)
	{
	default:
		DSC_ASSERT_ALWAYS("invalid switch");
		break;
	case DscUi::TUiNodeGroup::TDrawNode:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscUi::UiRenderTarget*) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TDrawBaseNode:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscUi::UiRenderTarget*) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TUiComponentType:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscUi::TUiComponentType) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TUiComponentResources:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscUi::UiComponentResourceNodeGroup) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TArrayChildUiNodeGroup:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(std::vector<DscUi::UiNodeGroup>) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TAvaliableSize: // the initial layout size we were told by the parent that we had avaliable
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscCommon::VectorInt2) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TRenderRequestSize:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscCommon::VectorInt2) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TGeometryOffset: // public so parent can panel draw this node
	{
		// does this need to be a float so we can animate move? dont animate via this value, used for layout
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscCommon::VectorInt2) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TGeometrySize:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscCommon::VectorInt2) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TScrollPos:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscCommon::VectorFloat2) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TScreenSpace:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscUi::ScreenSpace) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TUiPanelShaderConstantBuffer:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(std::shared_ptr<DscRenderResource::ShaderConstantBuffer>) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TUiPanelTint:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscCommon::VectorFloat4) };
		return s_meta_data;
	}
	}
	static DscDag::DagGroupNodeMetaData s_dummy = { false, typeid(nullptr) };
	return s_dummy;
}

template <>
const DscDag::DagGroupNodeMetaData& DscDag::GetDagGroupMetaData(const DscUi::TUiComponentResourceNodeGroup in_value)
{
	switch (in_value)
	{
	default:
		DSC_ASSERT_ALWAYS("invalid switch");
		break;

	case DscUi::TUiComponentResourceNodeGroup::TClearColour:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscCommon::VectorFloat4) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TUiScale:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(float) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TFillColour:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscCommon::VectorFloat4) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TTexture:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(std::shared_ptr<DscRenderResource::ShaderResource>) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TText:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::TUiComponentTextData) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TFlow:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::TUiFlow) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TGap:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::UiCoord) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TGradientFill:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::TGradientFillConstantBuffer) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::THasManualScrollX:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(bool) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TManualScrollX:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(float) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::THasManualScrollY:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(bool) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TManualScrollY:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(float) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TChildSlotSize:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::VectorUiCoord2) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TChildSlotPivot:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::VectorUiCoord2) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TChildSlotParentAttach:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::VectorUiCoord2) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TDesiredSize:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::VectorUiCoord2) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TPaddingLeft:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::UiCoord) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TPaddingTop:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::UiCoord) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TPaddingRight:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::UiCoord) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TPaddingBottom:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::UiCoord) };
		return s_meta_data;
	}

	case DscUi::TUiComponentResourceNodeGroup::TChildStackSize:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::UiCoord) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TChildStackPivot:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::UiCoord) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TChildStackParentAttach:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::UiCoord) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TInputStateFlag:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::TUiInputStateFlag) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TInputData:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscUi::TUiComponentInputData) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TInputRolloverAccumulate:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(float) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TInputActiveTouchPos:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscCommon::VectorFloat2) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TCrossfadeActiveChild:
	{
		// use the TDrawNode node to identify a UiNodeGroup, so the draw node (at the end of the draw chain for the ui hierarchy node) of the active child
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(DscDag::NodeToken) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TCrossfadeChildAmount:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(float) };
		return s_meta_data;
	}
	case DscUi::TUiComponentResourceNodeGroup::TCrossfadeNode:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(bool) };
		return s_meta_data;
	}
	}

	static DscDag::DagGroupNodeMetaData s_dummy = { false, typeid(nullptr) };
	return s_dummy;
}

const bool DscUi::ScreenSpace::operator==(const ScreenSpace& in_rhs) const
{
	if (_screen_space != in_rhs._screen_space)
	{
		return false;
	}

	if (_screen_valid != in_rhs._screen_valid)
	{
		return false;
	}

	return true;
}
