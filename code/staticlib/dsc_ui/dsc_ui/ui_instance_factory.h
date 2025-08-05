#pragma once
#include "dsc_ui.h"
#include <dsc_dag/dag_node_group.h>
#include <dsc_dag/dag_collection.h>
#include <dsc_dag/debug_print.h>

namespace DscDag
{
	class DagCollection;
	class IDagNode;
	typedef IDagNode* NodeToken;
}

namespace DscRender
{
	class DrawSystem;
}

namespace DscDag
{
	class DagCollection;

#if defined(_DEBUG)
	struct DagNodeGroupMetaData;
	template <typename IN_ENUM>
	const DagNodeGroupMetaData& GetDagNodeGroupMetaData(const IN_ENUM);
#endif //#if defined(_DEBUG)
}

namespace DscUi
{
	class IUiInstance;
	class UiManager;
	class UiRenderTarget;

	enum class TUiNodeGroupDataSource : uint8
	{
		TTemplateName,
		TCount
	};

	template <typename CONTEXT>
	class UiInstanceFactory
	{
	public:
		typedef std::function<std::shared_ptr<IUiInstance>(const UiInstanceFactory<CONTEXT>&, const CONTEXT&)> TUiInstanceFactory;
		void AddFactory(
			const std::string& in_template_name,
			const TUiInstanceFactory& in_factory
			)
		{
			DSC_ASSERT(false == in_template_name.empty(), "invalid param");
			DSC_ASSERT(nullptr != in_factory, "invalid param");
			_name_factory_map[in_template_name] = in_factory;
			return;
		}

		/*
			CONTEXT._dag_collection
			CONTEXT._data_source_node (a dag node <DscDag::NodeToken>) which has a value of a DagNodeGroup<TUiNodeGroupDataSource>
		*/
		DscDag::NodeToken BuildInstance(
			CONTEXT& in_context
		) const
		{
			DscDag::NodeToken result = in_context._dag_collection->CreateCalculate<std::shared_ptr<DscUi::IUiInstance>>(
				[this, in_context]
			(std::shared_ptr<DscUi::IUiInstance>& in_out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
				in_out_value.reset();

				DscDag::NodeToken data_source = DscDag::GetValueType<DscDag::NodeToken>(in_input_array[0]);
				if (nullptr == data_source)
				{
					return;
				}

				DscDag::NodeToken template_node = DscDag::DagNodeGroup::GetNodeTokenEnum(data_source, TUiNodeGroupDataSource::TTemplateName);
				const std::string& template_name = DscDag::GetValueType<std::string>(template_node);

				auto found = this->_name_factory_map.find(template_name);
				if (found != this->_name_factory_map.end())
				{
					in_out_value = (found->second)(
						*this,
						in_context
						);
				}

				return;
			}

			);
			DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result, "instance factory build instance"));

			DscDag::LinkIndexNodes(
				0, 
				//DscDag::DagNodeGroup::GetNodeTokenEnum(in_context._data_source, TUiNodeGroupDataSource::TTemplateName),
				in_context._data_source_node,
				result
				);

			return result;
		}

	private:
		std::map<std::string, TUiInstanceFactory> _name_factory_map = {};

	};
}

#if defined(_DEBUG)
template <>
const DscDag::DagNodeGroupMetaData& DscDag::GetDagNodeGroupMetaData(const DscUi::TUiNodeGroupDataSource in_value);
#endif //#if defined(_DEBUG)




