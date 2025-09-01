#pragma once
#include "dsc_ui.h"
#include "ui_input_param.h"

namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
}

namespace DscDag
{
	class DagCollection;
	class IDagNode;
	typedef IDagNode* NodeToken;
}

namespace DscUi
{
	class UiInputState
	{
	public:
		struct TouchState
		{
			int32 _id = 0;
			DscDag::NodeToken _node_under_click_start = nullptr;
			DscCommon::VectorFloat2 _node_relative_click_start = {};
			bool _click_start = false;
			bool _click_end = false;
			bool _lask_known_touch_active = false;

		};

		UiInputState();

		// if a node is deleted, clear our references to nodes, ref to raw pointer...
		void ClearNodeReferences();

		TouchState& GetTouchState(const UiInputParam::TouchData& in_touch_data);
	private:
		TouchState& GetTouchStateForId(const int32 in_id);

	private:
		std::vector<TouchState> _touch_state = {};

	};
}



