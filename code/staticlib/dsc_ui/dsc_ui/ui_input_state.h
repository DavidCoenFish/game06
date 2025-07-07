#pragma once
#include "dsc_ui.h"

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
		UiInputState();

	private:
		struct TouchState
		{
			int32 _id = 0;
			DscDag::NodeToken _node_under_touch_start = nullptr;
		};
		std::vector<TouchState> _touch_state = {};

	};
}



