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
    class IDagOwner;
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
        TSubTitle,
        TButtonArray,
        TCount
    };

    struct TButtonData
    {
        DscDag::NodeToken _button_text = {};
        std::function<void(DscDag::NodeToken)> _on_click = {};
    };

    UiInstanceMainMenu() = delete;
    UiInstanceMainMenu& operator=(const UiInstanceMainMenu&) = delete;
    UiInstanceMainMenu(const UiInstanceMainMenu&) = delete;

    static const std::string GetTemplateName();

    static DscDag::NodeToken BuildDataSource(
        DscDag::DagCollection& in_dag_collection,
        DscDag::IDagOwner* const in_data_source_owner,
        DscDag::NodeToken in_root_data_source_node
        );

    static std::shared_ptr<DscUi::IUiInstance> Factory(
        const DscUi::UiInstanceFactory<UiInstanceContext>& in_ui_instance_factory,
        const UiInstanceContext& in_context
    );

    UiInstanceMainMenu(
        const DscUi::UiInstanceFactory<UiInstanceContext>& in_ui_instance_factory,
        const UiInstanceContext& in_context
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

    DscDag::NodeToken _root_node_group = {};
    DscDag::NodeToken _parent_node_group = {};
    DscDag::NodeToken _main_node_group = {};
};


