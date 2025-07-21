#pragma once
#include "dsc_dag.h"

namespace DscDag
{
	class IDagNode;
	typedef IDagNode* NodeToken;

	void LinkNodes(NodeToken in_input, NodeToken in_output);
	void LinkIndexNodes(int32 in_index, NodeToken in_input, NodeToken in_output);

	void UnlinkNodes(NodeToken in_input, NodeToken in_output);
	void UnlinkIndexNodes(int32 in_index, NodeToken in_input, NodeToken in_output);

} //DscDag
