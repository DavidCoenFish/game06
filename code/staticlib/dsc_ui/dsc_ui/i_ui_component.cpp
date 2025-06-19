#include "i_ui_component.h"
#include <dsc_common\vector_int2.h>
#include <dsc_common\vector_float4.h>

DscUi::IUiComponent::~IUiComponent()
{
	// nop
}

const DscCommon::VectorFloat4& DscUi::IUiComponent::GetClearColour() const
{
	return DscCommon::VectorFloat4::s_zero;
}

const DscCommon::VectorInt2 DscUi::IUiComponent::CalculateDesiredSize(const DscCommon::VectorInt2& in_avaliable_size) const
{
	return in_avaliable_size;
}

DscDag::NodeToken DscUi::IUiComponent::GetChildAvalableSizeNode(const int32) const
{
	return nullptr;
}
