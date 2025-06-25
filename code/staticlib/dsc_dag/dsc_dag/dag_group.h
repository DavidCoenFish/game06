#pragma once
#include "dsc_dag.h"

namespace DscDag
{
	class IDagNode;
	typedef IDagNode* NodeToken;

	template <typename ENUM, std::size_t SIZE>
	class DagGroup
	{
	public:
		DagGroup(NodeToken const (&in_node_token_array)[SIZE])
		{
			static_assert(SIZE == ENUM::TCount);
			for (std::size_t index = 0; index < SIZE; ++index)
			{
				_node_token_array[index] = in_node_token_array[index];
			}
			return;
		}

		NodeToken GetNodeToken(const ENUM in_index)
		{
			return _node_token_array[in_index];
		}

	private:
		NodeToken _node_token_array[SIZE] = {};

	}; // DagGroup
} //DscDag
