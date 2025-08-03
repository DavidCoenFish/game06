#include "ui_instance_factory.h"
#include "i_ui_instance.h"

#if defined(_DEBUG)
template <>
const DscDag::DagNodeGroupMetaData& DscDag::GetDagNodeGroupMetaData(const DscUi::TUiNodeGroupDataSource in_value)
{
	switch (in_value)
	{
	default:
		DSC_ASSERT_ALWAYS("invalid switch");
		break;
	case DscUi::TUiNodeGroupDataSource::TTemplateName:
	{
		static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(std::string) };
		return s_meta_data;
	}
	}
	static DscDag::DagNodeGroupMetaData s_dummy = { false, typeid(nullptr) };
	return s_dummy;
}
#endif //#if defined(_DEBUG)
