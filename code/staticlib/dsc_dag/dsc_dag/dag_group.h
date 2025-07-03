#pragma once
#include "dsc_dag.h"
#include "dag_collection.h"

namespace DscDag
{
	class IDagNode;
	typedef IDagNode* NodeToken;

	struct DagGroupNodeMetaData
	{
		DagGroupNodeMetaData() = delete;
		DagGroupNodeMetaData& operator=(const DagGroupNodeMetaData&) = delete;
		DagGroupNodeMetaData(const DagGroupNodeMetaData&) = delete;

		bool _optional = {};
		const std::type_info& _type_info;
	};

	template <typename IN_ENUM>
	const DagGroupNodeMetaData& GetDagGroupMetaData(const IN_ENUM);

	/// <summary>
	/// Totally not a duck typed class / object that can be handed around to other people that agree with the set of nodes to be provided / interface
	/// currently missing type safety of the node types however... 
	/// which could require another array of the type ids/ if empty allowed? and if allowed to set? and bring the GetValueType into method this class?
	/// </summary>
	template <typename IN_ENUM, std::size_t IN_SIZE>
	class DagGroup
	{
	public:
		DagGroup(DagCollection* const in_dag_collection = nullptr) : _dag_collection(in_dag_collection) {}
		DagGroup(DagCollection* const in_dag_collection, NodeToken const (&in_node_token_array)[IN_SIZE])
			: _dag_collection(in_dag_collection)
		{
			static_assert(IN_SIZE == static_cast<std::size_t>(IN_ENUM::TCount));
			for (std::size_t index = 0; index < IN_SIZE; ++index)
			{
				_node_token_array[index] = in_node_token_array[index];
			}
			return;
		}

		DagGroup& operator=(const DagGroup& in_rhs)
		{
			if (this != &in_rhs)
			{
				_dag_collection = in_rhs._dag_collection;
				for (std::size_t index = 0; index < IN_SIZE; ++index)
				{
					_node_token_array[index] = in_rhs._node_token_array[index];
				}
			}
			return *this;
		}

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
			DSC_ASSERT(nullptr != _node_token_array[static_cast<std::size_t>(in_index)], "invalid state");
			return _node_token_array[static_cast<std::size_t>(in_index)];
		}

		void Validate() const
		{
			if (nullptr == _dag_collection)
			{
				DSC_ASSERT_ALWAYS("invalid state");
				return;
			}
			for (std::size_t index = 0; index < IN_SIZE; ++index)
			{
				const DagGroupNodeMetaData& meta_data = GetDagGroupMetaData(static_cast<IN_ENUM>(index));

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

		void ResolveDirtyConditionNodes()
		{
			DSC_ASSERT(nullptr != _dag_collection, "invalid state");
			_dag_collection->ResolveDirtyConditionNodes();
		}

		DagCollection* GetDagCollection() const 
		{
			return _dag_collection;
		}

	private:
		DagCollection* _dag_collection = nullptr;
		NodeToken _node_token_array[IN_SIZE] = {};

	}; // DagGroup
} //DscDag
