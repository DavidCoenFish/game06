#include "ui_enum.h"
#include <dsc_dag/dag_group.h>

template class DscDag::DagGroup<DscUi::TUiRootNodeGroup, static_cast<std::size_t>(DscUi::TUiRootNodeGroup::TCount)>;

template class DscDag::DagGroup<DscUi::TUiParentNodeGroup, static_cast<std::size_t>(DscUi::TUiParentNodeGroup::TCount)>;


