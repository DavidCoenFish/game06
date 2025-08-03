#include "ui_instance_app.h"
#include "ui_instance_context.h"
#include <dsc_ui/component_construction_helper.h>
#include <dsc_ui/ui_manager.h>

#if defined(_DEBUG)
template <>
const DscDag::DagNodeGroupMetaData& DscDag::GetDagNodeGroupMetaData(const UiInstanceApp::TUiNodeGroupDataSource in_value)
{
    switch (in_value)
    {
    default:
        DSC_ASSERT_ALWAYS("invalid switch");
        break;
    case UiInstanceApp::TUiNodeGroupDataSource::TMainScreenDataSource:
    {
        static DscDag::DagNodeGroupMetaData s_meta_data = { true, typeid(DscDag::NodeToken) };
        return s_meta_data;
    }
    }
    static DscDag::DagNodeGroupMetaData s_dummy = { false, typeid(nullptr) };
    return s_dummy;
}

#endif //#if defined(_DEBUG)



std::shared_ptr<DscUi::IUiInstance> UiInstanceApp::Factory(
    const DscUi::UiInstanceFactory<UiInstanceContext>& in_ui_instance_factory,
    const UiInstanceContext& in_context
)
{
    DscDag::NodeToken main_screen_data_source = DscDag::DagNodeGroup::GetNodeTokenEnum(
        in_context._data_source, 
        TUiNodeGroupDataSource::TMainScreenDataSource
    );

    std::shared_ptr<DscUi::IUiInstance> result = std::make_shared<UiInstanceApp>(
        in_context._root_external_render_target_or_null,
        in_ui_instance_factory,
        *in_context._ui_manager,
        *in_context._draw_system,
        *in_context._dag_collection,
        *in_context._file_system,
        main_screen_data_source
        );

    return result;
}

UiInstanceApp::UiInstanceApp(
    const std::shared_ptr<DscUi::UiRenderTarget>& in_root_external_render_target_or_null,
    const DscUi::UiInstanceFactory<UiInstanceContext>& in_ui_instance_factory,
    DscUi::UiManager& in_ui_manager,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    DscCommon::FileSystem& in_file_system,
    DscDag::NodeToken in_main_screen_data_source
)
    : _ui_manager(in_ui_manager)
    , _draw_system(in_draw_system)
    , _dag_collection(in_dag_collection)
{
    _root_node_group = _ui_manager.MakeRootNode(
        DscUi::MakeComponentCanvas(
        ),
        _draw_system,
        _dag_collection,
        in_root_external_render_target_or_null
    );

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
        UiInstanceContext context = {};
        context._dag_collection = &_dag_collection;
        context._data_source = in_main_screen_data_source;
        context._draw_system = &_draw_system;
        context._file_system = &in_file_system;
        context._parent_node_or_null = _main_screen_cross_fade;
        context._root_node_or_null = _root_node_group;
        context._ui_manager = &_ui_manager;
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
