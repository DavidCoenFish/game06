#include "dsc_dag_group.h"
#include <dsc_common/dsc_common.h>
#include <dsc_dag/dag_collection.h>
#include <dsc_dag/dag_node_group.h>
#include "test_util.h"

namespace
{
	enum class TTestGroup
	{
		TInt,
		TFloat,
		TCount
	};

}
#if 0
template <>
const DscDag::DagGroupNodeMetaData& DscDag::GetDagGroupMetaData(const TTestGroup in_value)
{
	switch (in_value)
	{
	default:
		DSC_ASSERT_ALWAYS("invalid switch");
		break;
	case TTestGroup::TInt:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { false, typeid(int)};
		return s_meta_data;
	}
	case TTestGroup::TFloat:
	{
		static DscDag::DagGroupNodeMetaData s_meta_data = { true, typeid(float) };
		return s_meta_data;
	}
	}
	static DscDag::DagGroupNodeMetaData s_dummy = { false, typeid(nullptr) };
	return s_dummy;
}

namespace
{
	const bool TestSanity()
	{
		bool ok = true;

		DscDag::DagCollection collection = {};

		DscDag::DagGroup<TTestGroup, static_cast<size_t>(TTestGroup::TCount)> group0;
		group0.SetNodeToken(TTestGroup::TInt, collection.CreateValue(7));
		group0.SetNodeToken(TTestGroup::TFloat, collection.CreateValue(3.4f));
		group0.Validate();

		DscDag::DagGroup<TTestGroup, static_cast<size_t>(TTestGroup::TCount)> group1;
		group1.SetNodeToken(TTestGroup::TInt, collection.CreateValue(8));
		group1.Validate();

		return ok;
	}
}//namespace
#endif
const bool DscDagGroup()
{
	bool ok = true;

	//ok &= TestSanity();

	return ok;
}