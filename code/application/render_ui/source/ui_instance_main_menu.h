#pragma once
#include "render_ui.h"
#include <dsc_ui\i_ui_instance.h>
#include <dsc_ui\ui_instance_factory.h>

namespace DscCommon
{
    class FileSystem;
}

namespace DscDag
{
    class DagCollection;
    class IDagNode;
    typedef IDagNode* NodeToken;
}

namespace DscRender
{
    class DagCollection;
}

namespace DscRender
{
    class DrawSystem;
}

namespace DscUi
{
    template <typename CONTEXT>
    class UiInstanceFactory;
    class UiRenderTarget;
    class UiManager;
}

struct UiInstanceContext;

class UiInstanceMainMenu : public DscUi::IUiInstance
{
public:
    enum class TUiNodeGroupDataSource : uint8
    {
        TTitle = static_cast<uint8>(DscUi::TUiNodeGroupDataSource::TCount),
        TCount
    };

    UiInstanceMainMenu() = delete;
    UiInstanceMainMenu& operator=(const UiInstanceMainMenu&) = delete;
    UiInstanceMainMenu(const UiInstanceMainMenu&) = delete;

    static std::shared_ptr<DscUi::IUiInstance> Factory(
        const DscUi::UiInstanceFactory<UiInstanceContext>& in_ui_instance_factory,
        const UiInstanceContext& in_context
    );

    UiInstanceMainMenu(
        DscUi::UiManager& in_ui_manager,
        DscRender::DrawSystem& in_draw_system,
        DscDag::DagCollection& in_dag_collection,
        DscCommon::FileSystem& in_file_system,
        DscDag::NodeToken in_data_source,
        DscDag::NodeToken in_root_node_or_null,
        DscDag::NodeToken in_parent_node_or_null
    );
    ~UiInstanceMainMenu();

private:
    virtual void Update() override;
    virtual DscDag::NodeToken GetDagUiGroupNode() override;
    virtual DscDag::NodeToken GetDagUiDrawNode() override;
    virtual DscDag::NodeToken GetDagUiDrawBaseNode() override;

private:
    DscUi::UiManager& _ui_manager;
    DscRender::DrawSystem& _draw_system;
    DscDag::DagCollection& _dag_collection;

    DscDag::NodeToken _parent_node_group = {};
    DscDag::NodeToken _main_node_group = {};
};


