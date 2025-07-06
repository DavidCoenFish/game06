#pragma once
#include "dsc_dag.h"

namespace DscDag
{
	class IDagNode;
	typedef IDagNode* NodeToken;

	class IDagGroup
	{
	public:
		virtual ~IDagGroup() {};

		virtual void AddOwnership(NodeToken in_node_token) = 0;

	}; // IDagGroup
} //DscDag
