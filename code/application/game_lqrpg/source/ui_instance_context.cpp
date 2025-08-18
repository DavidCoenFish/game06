#include "ui_instance_context.h"

UiInstanceContext UiInstanceContext::MakeChild(DscDag::NodeToken in_data_source_node, DscDag::NodeToken in_parent) const
{
	UiInstanceContext result(*this);

	result._root_external_render_target_or_null = nullptr;
	result._data_source_node = in_data_source_node;
	result._parent_node_or_null = in_parent;

	return result;
}

