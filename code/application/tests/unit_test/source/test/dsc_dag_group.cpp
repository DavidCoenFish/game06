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

#if defined(_DEBUG)
template <>
const DscDag::DagNodeGroupMetaData& DscDag::GetDagNodeGroupMetaData(const TTestGroup in_value)
{
	switch (in_value)
	{
	default:
		DSC_ASSERT_ALWAYS("invalid switch");
		break;
	case TTestGroup::TInt:
	{
		static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(int)};
		return s_meta_data;
	}
	case TTestGroup::TFloat:
	{
		static DscDag::DagNodeGroupMetaData s_meta_data = { true, typeid(float) };
		return s_meta_data;
	}
	}
	static DscDag::DagNodeGroupMetaData s_dummy = { false, typeid(nullptr) };
	return s_dummy;
}
#endif //#if defined(_DEBUG)

namespace
{
	const bool TestSanity()
	{
		bool ok = true;

		DscDag::DagCollection collection = {};
		const int32 count_before = collection.GetNodeCount();

		DscDag::NodeToken group0 = collection.CreateGroupEnum<TTestGroup>();
		DscDag::IDagOwner* owner0 = dynamic_cast<DscDag::IDagOwner*>(group0);
		DscDag::DagNodeGroup::SetNodeTokenEnum(
			group0, 
			TTestGroup::TInt, 
			collection.CreateValue(
				7, 
				DscDag::CallbackOnValueChange<int32>::Function, 
				owner0
			));
		DscDag::DagNodeGroup::SetNodeTokenEnum(
			group0, 
			TTestGroup::TFloat, 
			collection.CreateValue(
				3.4f, 
				DscDag::CallbackOnValueChange<float>::Function, 
				owner0
			));
		const int32 count_during = collection.GetNodeCount();
#if defined(_DEBUG)
		DscDag::DagNodeGroup::DebugValidate<TTestGroup>(group0);
#endif //defined(_DEBUG)

		collection.DeleteNode(group0);
		const int32 count_after = collection.GetNodeCount();


		ok = TEST_UTIL_EQUAL(ok, count_before, count_after);
		ok = TEST_UTIL_NOT_EQUAL(ok, count_before, count_during);

		DscDag::NodeToken group1 = collection.CreateGroupEnum<TTestGroup>();
		DscDag::IDagOwner* owner1 = dynamic_cast<DscDag::IDagOwner*>(group1);
		DscDag::DagNodeGroup::SetNodeTokenEnum(
			group0,
			TTestGroup::TInt,
			collection.CreateValue(
				8,
				DscDag::CallbackOnValueChange<int32>::Function,
				owner1
			));
#if defined(_DEBUG)
		DscDag::DagNodeGroup::DebugValidate<TTestGroup>(group1);
#endif //defined(_DEBUG)
		collection.DeleteNode(group1);

		return ok;
	}
}//namespace

const bool DscDagGroup()
{
	bool ok = true;

	ok &= TestSanity();

	return ok;
}