#include "i_ui_component.h"
#include <dsc_common\vector_int2.h>
#include <dsc_common\vector_float4.h>

DscUi::IUiComponent::~IUiComponent()
{
	// nop
}

void DscUi::IUiComponent::Draw(
	DscRenderResource::Frame&,
	DscRender::IRenderTarget&
)
{
	//nop
}

const DscCommon::VectorFloat4& DscUi::IUiComponent::GetClearColour() const
{
	return DscCommon::VectorFloat4::s_zero;
}

const bool DscUi::IUiComponent::SetClearColour(const DscCommon::VectorFloat4&)
{
	return false;
}

const DscCommon::VectorInt2 DscUi::IUiComponent::ConvertAvaliableSizeToDesiredSize(const DscCommon::VectorInt2& in_avaliable_size)
{
	return in_avaliable_size;
}

const DscCommon::VectorInt2 DscUi::IUiComponent::GetChildAvaliableSize(const DscCommon::VectorInt2& in_our_desired_size, const int32) const
{
	return in_our_desired_size;
}

const DscCommon::VectorInt2 DscUi::IUiComponent::GetChildGeometrySize(const DscCommon::VectorInt2&, const DscCommon::VectorInt2& in_child_avaliable_size) const
{
	return in_child_avaliable_size;
}

const DscCommon::VectorInt2 DscUi::IUiComponent::GetChildGeometryOffset(const DscCommon::VectorInt2&, const int32) const
{
	return DscCommon::VectorInt2::s_zero;
}

void DscUi::IUiComponent::Update(const float)
{
	//nop
}

//void DscUi::IUiComponent::AddChildRef(IUiComponent*)
//{
//	DSC_ASSERT_ALWAYS("unimplemented");
//}
//
//void DscUi::IUiComponent::RemoveChild(IUiComponent*)
//{
//	DSC_ASSERT_ALWAYS("unimplemented");
//}

//void DscUi::IUiComponent::SetScrollTraveralPixelDistance(const DscCommon::VectorInt2&)
//{
//	// nop
//}
//
//void DscUi::IUiComponent::SetScrollNode(DscDag::NodeToken)
//{
//	//DSC_ASSERT_ALWAYS("unimplemented");
//}
//
//void DscUi::IUiComponent::SetChildGeometrySizeNode(DscDag::NodeToken, const int32)
//{
//	DSC_ASSERT_ALWAYS("unimplemented");
//}
//
//void DscUi::IUiComponent::SetChildGeometryOffsetNode(DscDag::NodeToken, const int32)
//{
//	DSC_ASSERT_ALWAYS("unimplemented");
//}

void DscUi::IUiComponent::SetNode(DscDag::NodeToken, DscDag::NodeToken, DscDag::NodeToken, DscDag::NodeToken, DscDag::NodeToken)
{
	DSC_ASSERT_ALWAYS("unimplemented");
}

void DscUi::IUiComponent::BuildUiPanelShaderConstant(TUiPanelShaderConstantBuffer&)
{
	DSC_ASSERT_ALWAYS("unimplemented");
}

std::shared_ptr<DscRender::HeapWrapperItem> DscUi::IUiComponent::GetRenderTexture()
{
	DSC_ASSERT_ALWAYS("unimplemented");
	return nullptr;
}
