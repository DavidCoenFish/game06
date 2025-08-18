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

namespace DscData
{
    struct JsonValue;
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

class UiInstanceApp : public DscUi::IUiInstance
{
public:
    enum class TUiNodeGroupDataSource : uint8
    {
        TLocale = static_cast<uint8>(DscUi::TUiNodeGroupDataSource::TCount),
        TLocaleData,
        TFontPath,

        TKeepAppRunning,
        TMainScreenDataSourceNode, // a dag node <NodeToken> with the value of active Data Source for the main screen or null
        TDialogDataSourceNode,

        TMainMenuDataSource,

        TCount
    };

    UiInstanceApp() = delete;
    UiInstanceApp& operator=(const UiInstanceApp&) = delete;
    UiInstanceApp(const UiInstanceApp&) = delete;

    static const std::string GetTemplateName();

    static DscDag::NodeToken BuildDataSource(
        DscDag::DagCollection& in_dag_collection,
        DscData::JsonValue& in_data
    );

    static std::shared_ptr<DscUi::IUiInstance> Factory(
        const DscUi::UiInstanceFactory<UiInstanceContext>& in_ui_instance_factory,
        const UiInstanceContext& in_context
    );

    UiInstanceApp(
        const DscUi::UiInstanceFactory<UiInstanceContext>& in_ui_instance_factory,
        const UiInstanceContext& in_context
    );
    ~UiInstanceApp();

private:
    virtual void Update() override;
    virtual DscDag::NodeToken GetDagUiGroupNode() override;
    virtual DscDag::NodeToken GetDagUiDrawNode() override;
    virtual DscDag::NodeToken GetDagUiDrawBaseNode() override;
    virtual const bool HasContent() override;

private:
    DscUi::UiManager& _ui_manager;
    DscRender::DrawSystem& _draw_system;
    DscDag::DagCollection& _dag_collection;

    DscDag::NodeToken _root_node_group = {};
    DscDag::NodeToken _main_screen_cross_fade = {};
    DscDag::NodeToken _main_screen_factory_node = {};
};


