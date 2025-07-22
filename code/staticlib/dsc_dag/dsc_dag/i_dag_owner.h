#pragma once
#include "dsc_dag.h"

namespace DscDag
{
	class DagCollection;
	class IDagNode;
	typedef IDagNode* NodeToken;

	/// Ownership refers to a set of nodes that should be delete together
	class IDagOwner
	{
	public:
		virtual ~IDagOwner() {};

		virtual void AddOwnership(NodeToken in_node_token) = 0;
		virtual void DestroyOwned(DagCollection& in_dag_collection) = 0;

	}; // IDagOwner
} //DscDag
