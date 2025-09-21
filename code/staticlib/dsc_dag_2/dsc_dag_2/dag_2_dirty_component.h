#pragma once
#include "dsc_dag_2.h"
#include <dsc_common\dsc_common.h>

/*
don't have the concept of condtional sets in the dirty component
alt. have a set of the conditional nodes that need to be checked each step

other than adding debug name, pretty sure nothing else should be added to this class
*/
namespace DscDag2
{
	class Dag2calculateComponent;

	class Dag2DirtyComponent
	{
	public:
		// disable copy
		Dag2DirtyComponent(const Dag2DirtyComponent&) = delete;
		Dag2DirtyComponent& operator=(const Dag2DirtyComponent&) = delete;

			//std::set<Dag2DirtyComponent*>* in_conditional_set_or_nullptr = nullptr,
			//Dag2calculateComponent* in_calculate_component_or_nullptr = nullptr
		Dag2DirtyComponent();
		// on dtor, we unlink all our outputs
		~Dag2DirtyComponent();

		static void Link(Dag2DirtyComponent& in_input, Dag2DirtyComponent& in_output);
		static void Unlink(Dag2DirtyComponent& in_input, Dag2DirtyComponent& in_output);

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

		// Multiple double linked
		// the nodes that we use as input to our dirty state
		std::set<Dag2DirtyComponent*> _input_set = {};
		// the nodes that use us as input, our output 
		std::set<Dag2DirtyComponent*> _output_set = {};

		// where should this live, on dirty, want conditional components to be added to a set to be calculated
		// does clearing the dirty invoke the calculate components update? 
		// just keep track of the conditional nodes and update them each tick
		//std::set<Dag2DirtyComponent*>* _condition_set = {};
		//Dag2calculateComponent* _calculate_component = nullptr;
	};
}