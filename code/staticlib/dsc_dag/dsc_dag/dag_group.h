#pragma once
#include "dsc_dag.h"
#include "dag_collection.h"

namespace DscDag
{
	class IDagNode;
	typedef IDagNode* NodeToken;

	/// <summary>
	/// Totally not a duck typed class / object that can be handed around to other people that agree with the set of nodes to be provided / interface
	/// currently missing type safety of the node types however... 
	/// which could require another array of the type ids/ if empty allowed? and if allowed to set? and bring the GetValueType into method this class?
	/// </summary>
	template <typename ENUM, std::size_t SIZE>
	class DagGroup
	{
	public:
		DagGroup(DagCollection* const in_dag_collection = nullptr) : _dag_collection(in_dag_collection) {}
		DagGroup(DagCollection* const in_dag_collection, NodeToken const (&in_node_token_array)[SIZE])
			: _dag_collection(in_dag_collection)
		{
			static_assert(SIZE == static_cast<std::size_t>(ENUM::TCount));
			for (std::size_t index = 0; index < SIZE; ++index)
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
				for (std::size_t index = 0; index < SIZE; ++index)
				{
					_node_token_array[index] = in_rhs._node_token_array[index];
				}
			}
			return *this;
		}

		void SetNodeToken(const ENUM in_index, NodeToken in_node_token)
		{
			DSC_ASSERT((0 <= static_cast<std::size_t>(in_index)) && (static_cast<std::size_t>(in_index) < SIZE), "invalid param");
			_node_token_array[static_cast<std::size_t>(in_index)] = in_node_token;
			return;
		}

		NodeToken GetNodeToken(const ENUM in_index) const
		{
			DSC_ASSERT((0 <= static_cast<std::size_t>(in_index)) && (static_cast<std::size_t>(in_index) < SIZE), "invalid param");
			return _node_token_array[static_cast<std::size_t>(in_index)];
		}

		const bool IsValid() const
		{
			if (nullptr == _dag_collection)
			{
				return false;
			}
			for (std::size_t index = 0; index < SIZE; ++index)
			{
				if (nullptr == _node_token_array[index])
				{
					return false;
				}
			}
			return true;
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
		NodeToken _node_token_array[SIZE] = {};

	}; // DagGroup
} //DscDag
