#include "i_ui_component.h"
#include <dsc_common\vector_int2.h>
#include <dsc_common\vector_float4.h>
#include <dsc_dag\dag_collection.h>

DscUi::IUiComponent::~IUiComponent()
{
	// nop
}

void DscUi::IUiComponent::SetNode(const DagGroupUiComponent& in_ui_component_group)
{
	_ui_component_group = in_ui_component_group;
	return;
}

void DscUi::IUiComponent::SetClearColour(const DscCommon::VectorFloat4& in_colour)
{
	DscDag::DagCollection::SetValueType(_ui_component_group.GetNodeToken(TUiComponentGroup::TClearColourNode), in_colour);
	return;
}

void DscUi::IUiComponent::SetParentChildIndex(const int32 in_parent_child_index)
{
	DscDag::DagCollection::SetValueType(_ui_component_group.GetNodeToken(TUiComponentGroup::TParentChildIndex), in_parent_child_index);
	return;
}

void DscUi::IUiComponent::SetManualScrollX(const float in_x)
{
	DscDag::DagCollection::SetValueType(_ui_component_group.GetNodeToken(TUiComponentGroup::TManualScrollX), in_x);
	return;
}

void DscUi::IUiComponent::SetManualScrollY(const float in_y)
{
	DscDag::DagCollection::SetValueType(_ui_component_group.GetNodeToken(TUiComponentGroup::TManualScrollY), in_y);
	return;
}

void DscUi::IUiComponent::SetEffectData(const int32 in_index, const TEffectComponentData& in_effect_data)
{
	if (static_cast<int32>(_effect_data_array.size()) <= in_index)
	{
		_effect_data_array.resize(in_index + 1);
	}
	_effect_data_array[in_index] = in_effect_data;
	return;
}

DscUi::IUiComponent::TEffectComponentData DscUi::IUiComponent::GetEffectData(const int32 in_index)
{
	if ((0 <= in_index) && (in_index < static_cast<int32>(_effect_data_array.size())))
	{
		return _effect_data_array[in_index];
	}
	return TEffectComponentData();
}

void DscUi::IUiComponent::Draw(
	DscRenderResource::Frame&,
	DscRender::IRenderTarget&,
	const float
)
{
	//nop
}

const DscCommon::VectorInt2 DscUi::IUiComponent::ConvertAvaliableSizeToDesiredSize(const DscCommon::VectorInt2&, const DscCommon::VectorInt2& in_avaliable_size, const float)
{
	return in_avaliable_size;
}

const DscCommon::VectorInt2 DscUi::IUiComponent::GetChildAvaliableSize(const DscCommon::VectorInt2& in_our_desired_size, const int32, const float) const
{
	return in_our_desired_size;
}

const DscCommon::VectorInt2 DscUi::IUiComponent::GetChildGeometrySize(const DscCommon::VectorInt2&, const DscCommon::VectorInt2& in_child_avaliable_size) const
{
	return in_child_avaliable_size;
}

const DscCommon::VectorInt2 DscUi::IUiComponent::GetChildGeometryOffset(const DscCommon::VectorInt2&, const int32, const float) const
{
	return DscCommon::VectorInt2::s_zero;
}

const bool DscUi::IUiComponent::HasManualScrollX() const
{
	return false;
}

const bool DscUi::IUiComponent::HasManualScrollY() const
{
	return false;
}
