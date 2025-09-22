#include "dag_2_dirty_component.h"
#include <dsc_common\log_system.h>

DscDag2::Dag2DirtyComponent::Dag2DirtyComponent()
{
	//NOP
}

// on dtor, we unlink all our outputs
DscDag2::Dag2DirtyComponent::~Dag2DirtyComponent()
{
	for (auto& iter : _output_set)
	{
		DSC_ASSERT(nullptr != iter, "invalid state");
		Unlink(*this, *iter);
	}
	return;
}

void DscDag2::Dag2DirtyComponent::Link(Dag2DirtyComponent& in_input, Dag2DirtyComponent& in_output)
{
	auto found_input = in_input._output_set.find(&in_output);
	auto found_output = in_output._input_set.find(&in_input);
	if ((found_input != in_input._output_set.end()) ||
		(found_output != in_output._input_set.end()))
	{
		DSC_LOG_WARNING(LOG_TOPIC_DSC_DAG_2, "attempt to multiplically link Dag2DirtyComponent");
		return;
	}

	in_input._output_set.insert(&in_output);
	in_output._input_set.insert(&in_input);
	return;
}

void DscDag2::Dag2DirtyComponent::Unlink(Dag2DirtyComponent& in_input, Dag2DirtyComponent& in_output)
{
	auto found_input = in_input._output_set.find(&in_output);
	if (found_input != in_input._output_set.end())
	{
		in_input._output_set.erase(found_input);
	}

	auto found_output = in_output._input_set.find(&in_input);
	if (found_output != in_output._input_set.end())
	{
		in_output._input_set.erase(found_output);
	}

	return;
}

// if we are not dirty, then set ourselves as dirty and tell output to mark themselves as dirty
void DscDag2::Dag2DirtyComponent::MarkDirtyFlag()
{
	if (true == _dirty_flag)
	{
		return;
	}

	_dirty_flag = true;
	for (auto& iter : _output_set)
	{
		iter->MarkDirtyFlag();
	}

	return;
}

// if we are dirty, remove the 
void DscDag2::Dag2DirtyComponent::ClearDirtyFlag()
{
	if (true == _dirty_flag)
	{
		_dirty_flag = false;
	}
	return;
}
