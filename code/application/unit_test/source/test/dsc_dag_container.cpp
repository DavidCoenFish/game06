#include "dsc_dag_container.h"
#include <dsc_common/dsc_common.h>
#include <dsc_dag/dag_collection.h>
#include "test_util.h"

namespace
{
const bool TestSanity()
{
	bool ok = true;

	DscDag::DagCollection collection = {};
	auto n0 = collection.CreateValue(1);
	auto n1 = collection.CreateValue(2);
	auto n2 = collection.CreateCalculate<int>([](int& output, std::set<DscDag::NodeToken>& inputSet, std::vector<DscDag::NodeToken>) {
		int32 sum = 0;
		for (auto& item : inputSet)
		{
			sum += DscDag::DagCollection::GetValueType<int32>(item);
		}
		output = sum;
	});

	DscDag::DagCollection::LinkNodes(n0, n2);
	DscDag::DagCollection::LinkNodes(n1, n2);

	ok = TEST_UTIL_EQUAL(ok, 3, DscDag::DagCollection::GetValueType<int32>(n2));
	DscDag::DagCollection::SetValueType(n0, 3);
	ok = TEST_UTIL_EQUAL(ok, 5, DscDag::DagCollection::GetValueType<int32>(n2));

	return ok;
}

const bool TestDirty()
{
	bool ok = true;

	DscDag::DagCollection collection = {};
	auto n0 = collection.CreateValue(1);
	auto n1 = collection.CreateValue(2);
	int calculateCount = 0;
	auto n2 = collection.CreateCalculate<int>([&calculateCount](int& output, std::set<DscDag::NodeToken>& inputSet, std::vector<DscDag::NodeToken>) {
		calculateCount += 1;
		int32 sum = 0;
		for (auto& item : inputSet)
		{
			sum += DscDag::DagCollection::GetValueType<int32>(item);
		}
		output = sum;
	});

	DscDag::DagCollection::LinkNodes(n0, n2);
	DscDag::DagCollection::LinkNodes(n1, n2);

	ok = TEST_UTIL_EQUAL(ok, 0, calculateCount);
	ok = TEST_UTIL_EQUAL(ok, 3, DscDag::DagCollection::GetValueType<int32>(n2));
	ok = TEST_UTIL_EQUAL(ok, 1, calculateCount);

	DscDag::DagCollection::SetValueType(n0, 3);
	ok = TEST_UTIL_EQUAL(ok, 1, calculateCount);
	ok = TEST_UTIL_EQUAL(ok, 5, DscDag::DagCollection::GetValueType<int32>(n2));
	ok = TEST_UTIL_EQUAL(ok, 2, calculateCount);

	DscDag::DagCollection::SetValueType(n0, 3);
	ok = TEST_UTIL_EQUAL(ok, 2, calculateCount);
	ok = TEST_UTIL_EQUAL(ok, 5, DscDag::DagCollection::GetValueType<int32>(n2));
	ok = TEST_UTIL_EQUAL(ok, 2, calculateCount);

	return ok;
}

}//namespace

const bool DscDagContainer()
{
	bool ok = true;

	ok &= TestSanity();
	ok &= TestDirty();

	return ok;
}