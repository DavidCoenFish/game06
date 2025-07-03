#include "ui_enum.h"
#include "ui_render_target.h"
#include "ui_input_state.h"
#include <dsc_dag/dag_group.h>

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
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(std::nullptr_t) };
		return s_meta_data;
	}
	case DscUi::TUiRootNodeGroup::TUiComponentType:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscUi::TUiComponentType) };
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
		// std::unique_ptr via DagNodeValueUnique
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscUi::UiRenderTarget) };
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
		// std::unique_ptr via DagNodeValueUnique
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
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(std::shared_ptr<DscRenderResource::RenderTargetTexture>) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TUiComponentType:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscUi::TUiComponentType) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TArrayChildUiNodeGroup:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(std::vector<DscUi::UiNodeGroup>) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TUiRenderTarget:
	{
		// std::unique_ptr via DagNodeValueUnique
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscUi::UiRenderTarget) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TRenderTargetSize:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscCommon::VectorInt2) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TScreenSpaceSize:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscCommon::VectorFloat4) };
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
	case DscUi::TUiNodeGroup::TClearColour:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(DscCommon::VectorFloat4) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::THasManualScrollX:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(bool) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TManualScrollX:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(float) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::THasManualScrollY:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(bool) };
		return s_meta_data;
	}
	case DscUi::TUiNodeGroup::TManualScrollY:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(float) };
		return s_meta_data;
	}

	}
	static DscDag::DagGroupNodeMetaData s_dummy = { false, typeid(nullptr) };
	return s_dummy;
}