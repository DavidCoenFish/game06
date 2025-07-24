#pragma once
#include "dsc_dag.h"
#include "i_dag_node.h"
#include "i_dag_owner.h"
#include <dsc_common/log_system.h>

namespace DscDag
{
	class DagCollection;
	class IDagNode;
	typedef IDagNode* NodeToken;

	/// Used in accessor to check types in and out of DagNode
#if defined(_DEBUG)
	struct DagNodeGroupMetaData
	{
		DagNodeGroupMetaData() = delete;
		DagNodeGroupMetaData& operator=(const DagNodeGroupMetaData&) = delete;
		DagNodeGroupMetaData(const DagNodeGroupMetaData&) = delete;

		bool _optional = false;
		const std::type_info& _type_info;
	};
	template <typename IN_ENUM>
	const DagNodeGroupMetaData& GetDagNodeGroupMetaData(const IN_ENUM);
#endif //#if defined(_DEBUG)

	template <typename IN_ENUM>
	struct ValidateOneType {
		static const bool Function(const std::type_info& in_enum_info)
		{
			return (typeid(IN_ENUM) == in_enum_info);
		}
	};

	template <typename IN_ENUM_A, typename IN_ENUM_B>
	struct ValidateTwoType {
		static const bool Function(const std::type_info& in_enum_info)
		{
			return (typeid(IN_ENUM_A) == in_enum_info) ||
				(typeid(IN_ENUM_B) == in_enum_info);
		}
	};

	/// hold a fixed size array of Nodes of a controlled type
	class DagNodeGroup : public IDagNode, public IDagOwner
	{
	public:
		DagNodeGroup() = delete;
		DagNodeGroup& operator=(const DagNodeGroup&) = delete;
		DagNodeGroup(const DagNodeGroup&) = delete;

		typedef std::function<const bool(const std::type_info&)> TValidateFunction;
		// provide a callback for accepted enum types to construct index from
		DagNodeGroup(const int32 in_size, const TValidateFunction& in_validate_function);
		virtual ~DagNodeGroup();

		template <typename IN_ENUM>
		static NodeToken GetNodeTokenEnum(NodeToken in_node, const IN_ENUM in_index)
		{
			DagNodeGroup* const group = dynamic_cast<DagNodeGroup*>(in_node);
			DSC_ASSERT(nullptr != group, "why are you trying to access node as a DagNodeGroup");
			if (nullptr == group)
			{
				return nullptr;
			}
			DSC_ASSERT(true == group->ValidateIndexEnum(typeid(IN_ENUM)), "incompatable enum type");
			NodeToken result = group->GetNodeToken(static_cast<int32>(in_index));
#if defined(_DEBUG)
			const DagNodeGroupMetaData& meta_data = GetDagNodeGroupMetaData(in_index);
			if (nullptr == result)
			{
				DSC_ASSERT(meta_data._optional == true, "invalid state");
			}
			else
			{
				DSC_ASSERT(result->DebugGetTypeInfo() == meta_data._type_info, "unexpected type");
			}
#endif
			return result;
		}

		template <typename IN_ENUM>
		static void SetNodeTokenEnum(NodeToken in_node, const IN_ENUM in_index, NodeToken in_node_to_add_or_null)
		{
			DagNodeGroup* const group = dynamic_cast<DagNodeGroup*>(in_node);
			DSC_ASSERT(nullptr != group, "why are you trying to access node as a DagNodeGroup");
			if (nullptr == group)
			{
				return;
			}
			DSC_ASSERT(true == group->ValidateIndexEnum(typeid(IN_ENUM)), "incompatable enum type");
#if defined(_DEBUG)
			const DagNodeGroupMetaData& meta_data = GetDagNodeGroupMetaData(in_index);
			if (nullptr == in_node_to_add_or_null)
			{
				DSC_ASSERT(meta_data._optional == true, "invalid state");
			}
			else
			{
				DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "SetNodeTokenEnum: in_node_type:%s meta_data_type:%s\n", in_node_to_add_or_null->DebugGetTypeInfo().name(), meta_data._type_info.name());
				DSC_ASSERT(in_node_to_add_or_null->DebugGetTypeInfo() == meta_data._type_info, "unexpected type");
			}
#endif
			group->SetNodeToken(static_cast<int32>(in_index), in_node_to_add_or_null);
		}

		NodeToken GetNodeToken(const int32 in_index) const;
		void SetNodeToken(const int32 in_index, NodeToken in_node_token_or_null);

#if defined(_DEBUG)
		template <typename IN_ENUM>
		static void DebugValidate(NodeToken in_node)
		{
			DagNodeGroup* const group = dynamic_cast<DagNodeGroup*>(in_node);
			DSC_ASSERT(nullptr != group, "why are you trying to access node as a DagNodeGroup");
			if (nullptr == group)
			{
				return;
			}

			DSC_ASSERT(true == group->ValidateIndexEnum(typeid(IN_ENUM)), "incompatable enum type");
			// if there is a subset enum, then only check the subset
			const std::size_t size = std::min(group->_node_token_array.size(), static_cast<size_t>(IN_ENUM::TCount));
			for (std::size_t index = 0; index < size; ++index)
			{
				const DagNodeGroupMetaData& meta_data = GetDagNodeGroupMetaData(static_cast<IN_ENUM>(index));
				NodeToken item = group->_node_token_array[index];

				if (nullptr == item)
				{
					if (false == meta_data._optional)
					{
						DSC_ASSERT_ALWAYS("invalid state");
						return;
					}
				}
				else
				{
					if (meta_data._type_info != item->DebugGetTypeInfo())
					{
						DSC_ASSERT_ALWAYS("invalid type found");
						return;
					}
				}
			}
			return;
		}
#endif //#if defined(_DEBUG)

	private:
		const bool ValidateIndexEnum(const std::type_info& in_index_enum);

		//IDagOwner
		virtual void AddOwnership(NodeToken in_node_token) override;
		virtual void DestroyOwned(DagCollection& in_dag_collection) override;

		//IDagNode
		virtual void MarkDirty() override;
		virtual void Update() override;
		virtual void AddOutput(NodeToken in_node) override;
		virtual void RemoveOutput(NodeToken in_node) override;
		virtual const bool GetHasNoLinks() const override;
		virtual void UnlinkInputs() override;

#if defined(_DEBUG)
		virtual const std::type_info& DebugGetTypeInfo() const override;
		virtual const std::string DebugPrintRecurseInputs(const int32 in_depth = 0) const override;
		virtual const std::string DebugPrintRecurseOutputs(const int32 in_depth = 0) const override;
#endif //#if defined(_DEBUG)

	private:
		const TValidateFunction _validate_function = {};
		std::vector<NodeToken> _node_token_array = {};

		// trying to make it easier to latter collect all the nodes that are to be removed as a group
		std::set<NodeToken> _node_ownership_group = {};
		std::set<NodeToken> _output = {};
		bool _dirty = false;
	}; // DagNodeGroup
} //DscDag
