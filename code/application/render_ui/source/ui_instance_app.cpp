#include "ui_instance_app.h"
#include "ui_instance_context.h"
#include <dsc_ui/component_construction_helper.h>
#include <dsc_ui/ui_manager.h>
#include <dsc_locale/dsc_locale.h>

#if defined(_DEBUG)
template <>
const DscDag::DagNodeGroupMetaData& DscDag::GetDagNodeGroupMetaData(const UiInstanceApp::TUiNodeGroupDataSource in_value)
{
    const int32 value_int = static_cast<int32>(in_value);
    if (value_int < static_cast<int32>(DscUi::TUiNodeGroupDataSource::TCount))
    {
        return GetDagNodeGroupMetaData(static_cast<DscUi::TUiNodeGroupDataSource>(in_value));
    }

    switch (in_value)
    {
    default:
        DSC_ASSERT_ALWAYS("invalid switch");
        break;
    case UiInstanceApp::TUiNodeGroupDataSource::TLocale:
    {
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(DscLocale::LocaleISO_639_1) };
        return s_meta_data;
    }
    case UiInstanceApp::TUiNodeGroupDataSource::TKeepAppRunning:
    {
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(bool) };
        return s_meta_data;
    }
    case UiInstanceApp::TUiNodeGroupDataSource::TMainScreenDataSource:
    {
        static DscDag::DagNodeGroupMetaData s_meta_data = { true, typeid(DscDag::NodeToken) };
        return s_meta_data;
    }
    case UiInstanceApp::TUiNodeGroupDataSource::TDialogDataSource:
    {
        static DscDag::DagNodeGroupMetaData s_meta_data = { true, typeid(DscDag::NodeToken) };
        return s_meta_data;
    }
    }
    static DscDag::DagNodeGroupMetaData s_dummy = { false, typeid(nullptr) };
    return s_dummy;
}

#endif //#if defined(_DEBUG)

const std::string UiInstanceApp::GetTemplateName()
{
    return "app";
}

DscDag::NodeToken UiInstanceApp::BuildDataSource(
    DscDag::DagCollection& in_dag_collection
)
{
    auto result = in_dag_collection.CreateGroupEnum<UiInstanceApp::TUiNodeGroupDataSource, DscUi::TUiNodeGroupDataSource>();
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result, "data source"));
    DscDag::IDagOwner* const data_source_owner = dynamic_cast<DscDag::IDagOwner*>(result);
    //TTemplateName
    {
        auto node = in_dag_collection.CreateValueOnValueChange<std::string>(GetTemplateName(), data_source_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "template name"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            DscUi::TUiNodeGroupDataSource::TTemplateName,
            node
        );
    }
    //TLocale
    {
        auto node = in_dag_collection.CreateValueOnValueChange(DscLocale::LocaleISO_639_1::English, data_source_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "locale"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            UiInstanceApp::TUiNodeGroupDataSource::TLocale,
            node
        );
    }
    //TKeepAppRunning
    {
        auto node = in_dag_collection.CreateValueOnValueChange(true, data_source_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "keep app running"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            UiInstanceApp::TUiNodeGroupDataSource::TKeepAppRunning,
            node
        );
    }

    return result;
}

std::shared_ptr<DscUi::IUiInstance> UiInstanceApp::Factory(
    const DscUi::UiInstanceFactory<UiInstanceContext>& in_ui_instance_factory,
    const UiInstanceContext& in_context
)
{
    std::shared_ptr<DscUi::IUiInstance> result = std::make_shared<UiInstanceApp>(
        in_ui_instance_factory,
        in_context
        );

    return result;
}

UiInstanceApp::UiInstanceApp(
    const DscUi::UiInstanceFactory<UiInstanceContext>& in_ui_instance_factory,
    const UiInstanceContext& in_context
)
    : _ui_manager(*in_context._ui_manager)
    , _draw_system(*in_context._draw_system)
    , _dag_collection(*in_context._dag_collection)
{
    _root_node_group = _ui_manager.MakeRootNode(
        DscUi::MakeComponentCanvas(
        ),
        _draw_system,
        _dag_collection,
        in_context._root_external_render_target_or_null
    );

#if 0
    _ui_manager.AddChildNode(
        DscUi::MakeComponentDebugGrid().SetChildSlot(
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f))
        ),
        _draw_system,
        _dag_collection,
        _root_node_group,
        _root_node_group,
        std::vector<DscUi::UiManager::TEffectConstructionHelper>()
        DSC_DEBUG_ONLY(DSC_COMMA "debug grid")
    );
#endif

    _main_screen_cross_fade = _ui_manager.AddChildNode(
        DscUi::MakeComponentCrossfade(
            nullptr
        ).SetChildSlot(
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f))
        ).SetClearColour(
            DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.0f)
        ),
        _draw_system,
        _dag_collection,
        _root_node_group,
        _root_node_group,
        std::vector<DscUi::UiManager::TEffectConstructionHelper>()
        DSC_DEBUG_ONLY(DSC_COMMA "app crossfade main")
    );

    {
        DscDag::NodeToken main_screen_data_source = DscDag::DagNodeGroup::GetNodeTokenEnum(
            in_context._data_source,
            TUiNodeGroupDataSource::TMainScreenDataSource
        );

        UiInstanceContext context = in_context.MakeChild(
            main_screen_data_source,
            _main_screen_cross_fade
            );
        context._root_node_or_null = _root_node_group;

        _main_screen_factory_node = in_ui_instance_factory.BuildInstance(
            context
        );
    }
}

UiInstanceApp::~UiInstanceApp()
{
    //DscDag::DebugPrintRecurseInputs(_root_node_group);
    //DscDag::DebugPrintRecurseOutputs(_root_node_group);

    _dag_collection.DeleteNode(_main_screen_factory_node);
    _main_screen_factory_node = nullptr;

    _ui_manager.DestroyNode(
        _dag_collection,
        _root_node_group
    );
}

void UiInstanceApp::Update()
{
    if (nullptr != _main_screen_factory_node)
    {
        _main_screen_factory_node->Update();
    }
}

DscDag::NodeToken UiInstanceApp::GetDagUiGroupNode()
{
    return _root_node_group;
}

DscDag::NodeToken UiInstanceApp::GetDagUiDrawNode()
{
    return DscDag::DagNodeGroup::GetNodeTokenEnum(_root_node_group, DscUi::TUiNodeGroup::TDrawNode);
}

DscDag::NodeToken UiInstanceApp::GetDagUiDrawBaseNode()
{
    return DscDag::DagNodeGroup::GetNodeTokenEnum(_root_node_group, DscUi::TUiNodeGroup::TDrawBaseNode);
}
