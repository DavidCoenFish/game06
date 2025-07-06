#include "ui_enum.h"
#include "ui_render_target.h"
#include "ui_input_state.h"
#include "vector_ui_coord2.h"
#include <dsc_dag/dag_group.h>
#include <dsc_render_resource/shader_resource.h>
#include <dsc_render_resource/shader_constant_buffer.h>

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
	case DscUi::TUiRootNodeGroup::TScreenSpaceSize:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscCommon::VectorFloat4) };
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
	case DscUi::TUiNodeGroup::TScreenSpaceSize:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscCommon::VectorFloat4) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TUiPanelShaderConstantBuffer:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(std::shared_ptr<DscRenderResource::ShaderConstantBuffer>) };
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
	//case DscUi::TUiComponentResourceNodeGroup::TArrayOwnedNodes:
	//{
	//	static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(std::vector<DscDag::NodeToken>) };
	//	return s_meta_data;
	//}

	case DscUi::TUiComponentResourceNodeGroup::TClearColour:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscCommon::VectorFloat4) };
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
	case DscUi::TUiComponentResourceNodeGroup::TEffectParamArray:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(std::vector<DscDag::NodeToken>) };
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

	}

	static DscDag::DagGroupNodeMetaData s_dummy = { false, typeid(nullptr) };
	return s_dummy;
}
