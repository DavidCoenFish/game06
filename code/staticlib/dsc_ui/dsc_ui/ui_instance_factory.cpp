#include "ui_instance_factory.h"
#include "i_ui_instance.h"
#include <dsc_dag/dag_node_group.h>
#include <dsc_dag/dag_collection.h>
#include <dsc_dag/debug_print.h>

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

void DscUi::UiInstanceFactory::AddFactory(
	const std::string& in_template_name,
	const TUiInstanceFactory& in_factory
)
{
	DSC_ASSERT(false == in_template_name.empty(), "invalid param");
	DSC_ASSERT(nullptr != in_factory, "invalid param");
	_name_factory_map[in_template_name] = in_factory;
	return;
}

// make a DagNode with a std::shared_ptr<IUiInstance> value, 
DscDag::NodeToken DscUi::UiInstanceFactory::BuildInstance(
	DscDag::NodeToken in_data_source,
	UiManager& in_ui_manager,
	DscRender::DrawSystem& in_draw_system,
	DscDag::DagCollection& in_dag_collection,
	const std::shared_ptr<UiRenderTarget>& in_root_external_render_target_or_null,
	DscDag::NodeToken in_parent_ui_node_group_or_null
)
{
	DscDag::NodeToken result = in_dag_collection.CreateCalculate<std::shared_ptr<DscUi::IUiInstance>>(
		// typedef std::function<void(IN_TYPE&, std::set<NodeToken>&, std::vector<NodeToken>&)> TCalculateFunction;
		[this, &in_ui_manager, &in_draw_system, &in_dag_collection, in_root_external_render_target_or_null, in_data_source, in_parent_ui_node_group_or_null]
		(std::shared_ptr<DscUi::IUiInstance>& in_out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
			in_out_value.reset();

			const std::string& template_name = DscDag::GetValueType<std::string>(in_input_array[0]);

			auto found = this->_name_factory_map.find(template_name);
			if (found != this->_name_factory_map.end())
			{
				in_out_value = (found->second)(
					*this,
					in_ui_manager,
					in_draw_system,
					in_dag_collection,
					in_root_external_render_target_or_null,
					in_data_source,
					in_parent_ui_node_group_or_null
					);
			}

			return;
		}
		
		);
	DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result, "instance factory build instance"));

	DscDag::LinkIndexNodes(0, DscDag::DagNodeGroup::GetNodeTokenEnum(in_data_source, TUiNodeGroupDataSource::TTemplateName), result);

	return result;
}
