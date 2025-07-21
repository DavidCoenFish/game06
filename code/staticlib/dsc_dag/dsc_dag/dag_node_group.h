#pragma once
#include "dsc_dag.h"
#include "i_dag_node.h"
#include "i_dag_owner.h"
#include "dag_collection.h"

namespace DscDag
{
	class IDagNode;
	typedef IDagNode* NodeToken;

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

	/// <summary>
	/// Duck typed class / object that can be handed around to other people that agree with the set of nodes to be provided / interface
	/// currently missing type safety of the node types however... 
	/// which could require another array of the type ids/ if empty allowed? and if allowed to set? and bring the GetValueType into method this class?
	/// </summary>
	template <typename IN_ENUM, std::size_t IN_SIZE>
	class DagNodeGroup : public IDagNode, public IDagOwner
	{
	public:
		DagNodeGroup() {}

		void SetNodeToken(const IN_ENUM in_index, NodeToken in_node_token)
		{
			DSC_ASSERT((0 <= static_cast<std::size_t>(in_index)) && (static_cast<std::size_t>(in_index) < IN_SIZE), "invalid param");
#if defined(_DEBUG)
			const DagGroupNodeMetaData& meta_data = GetDagGroupMetaData(in_index);
			if (nullptr == in_node_token)
			{
				DSC_ASSERT(meta_data._optional == true, "invalid param");
			}
			else
			{
				DSC_ASSERT(meta_data._type_info == in_node_token->GetTypeInfo(), "invalid param");
			}
#endif
			_node_token_array[static_cast<std::size_t>(in_index)] = in_node_token;

			return;
		}

		NodeToken GetNodeToken(const IN_ENUM in_index) const
		{
			DSC_ASSERT((0 <= static_cast<std::size_t>(in_index)) && (static_cast<std::size_t>(in_index) < IN_SIZE), "invalid param");
#if defined(_DEBUG)
			const DagGroupNodeMetaData& meta_data = GetDagGroupMetaData(in_index);
			if (nullptr == _node_token_array[static_cast<std::size_t>(in_index)])
			{
				DSC_ASSERT(meta_data._optional == true, "invalid state");
			}
#endif
			return _node_token_array[static_cast<std::size_t>(in_index)];
		}

		void Validate() const
		{
			for (std::size_t index = 0; index < IN_SIZE; ++index)
			{
				const DagNodeGroupMetaData& meta_data = GetDagNodeGroupMetaData(static_cast<IN_ENUM>(index));

				if (nullptr == _node_token_array[index])
				{
					if (false == meta_data._optional)
					{
						DSC_ASSERT_ALWAYS("invalid state");
						return;
					}
				}
				else
				{
					if (meta_data._type_info != _node_token_array[index]->GetTypeInfo())
					{
						DSC_ASSERT_ALWAYS("invalid state");
						return;
					}
				}
			}
			return;
		}

		void UnlinkOwned()
		{
			for (const auto& item : _node_ownership_group)
			{
				item->UnlinkInputs();
			}
		}

		void DeleteOwned(DagCollection& in_dag_node)
		{
			for (const auto& item : _node_ownership_group)
			{
				in_dag_node.DeleteNode(item);
			}
			_node_ownership_group.clear();
		}

	private:
		virtual void AddOwnership(NodeToken in_node_token) override
		{
			if (nullptr != in_node_token)
			{
				_node_ownership_group.push_back(in_node_token);
			}
		}

		virtual void AddOutput(NodeToken in_nodeID) override
		{
			DSC_ASSERT(nullptr != in_nodeID, "invalid param");
			if (nullptr != in_nodeID)
			{
				in_nodeID->MarkDirty();
			}
			_output.insert(in_nodeID);
		}

		virtual void RemoveOutput(NodeToken in_nodeID) override
		{
			DSC_ASSERT(nullptr != in_nodeID, "invalid param");
			if (nullptr != in_nodeID)
			{
				in_nodeID->MarkDirty();
			}
			_output.erase(in_nodeID);
		}

	private:
		NodeToken _node_token_array[IN_SIZE] = {};

		// trying to make it easier to latter collect all the nodes that are to be removed as a group
		std::vector<NodeToken> _node_ownership_group = {};
		std::set<NodeToken> _output = {};

	}; // DagNodeGroup
} //DscDag
