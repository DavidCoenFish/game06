#pragma once
#include "dsc_dag_2.h"
#include <dsc_common\dsc_common.h>
#include <dsc_common\log_system.h>

/*
don't have the concept of condtional sets in the dirty component
alt. have a set of the conditional nodes that need to be checked each step

other than adding debug name, pretty sure nothing else should be added to this class
*/
namespace DscDag2
{
	class DirtyComponent
	{
	public:
		// disable copy
		DirtyComponent(const DirtyComponent&) = delete;
		DirtyComponent& operator=(const DirtyComponent&) = delete;

			//std::set<Dag2DirtyComponent*>* in_conditional_set_or_nullptr = nullptr,
			//Dag2calculateComponent* in_calculate_component_or_nullptr = nullptr
		DirtyComponent();
		// on dtor, we unlink all our outputs
		~DirtyComponent();

		static void Link(DirtyComponent& in_input, DirtyComponent& in_output);
		static void Unlink(DirtyComponent& in_input, DirtyComponent& in_output);

		// if we are not dirty, then set ourselves as dirty and tell output to mark themselves as dirty
		void MarkDirtyFlag();
		// if we are dirty, remove the 
		void ClearDirtyFlag();

		const bool GetDirtyFlag() const 
		{
			return _dirty_flag;
		}

	private:
		bool _dirty_flag = false;

		// Multiple double linked, makes it easier to delete node without needing to presuerve order of singlely linked
		// the nodes that we use as input to our dirty state
		std::set<DirtyComponent*> _input_set = {};
		// the nodes that use us as input, our output 
		std::set<DirtyComponent*> _output_set = {};

		// where should this live, on dirty, want conditional components to be added to a set to be calculated
		// does clearing the dirty invoke the calculate components update? 
		// just keep track of the conditional nodes and update them each tick
		//std::set<Dag2DirtyComponent*>* _condition_set = {};
		//Dag2calculateComponent* _calculate_component = nullptr;
	};
}