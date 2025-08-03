#include "ui_instance_main_menu.h"
#include "ui_instance_context.h"
#include <dsc_common/file_system.h>
#include <dsc_ui/component_construction_helper.h>
#include <dsc_ui/ui_manager.h>
#include <dsc_render_resource_png/dsc_render_resource_png.h>

std::shared_ptr<DscUi::IUiInstance> UiInstanceMainMenu::Factory(
    const DscUi::UiInstanceFactory<UiInstanceContext>& in_ui_instance_factory,
    const UiInstanceContext& in_context
)
{
    DSC_UNUSED(in_ui_instance_factory);

    std::shared_ptr<DscUi::IUiInstance> result = std::make_shared<UiInstanceMainMenu>(
        *in_context._ui_manager,
        *in_context._draw_system,
        *in_context._dag_collection,
        *in_context._file_system,
        in_context._data_source,
        in_context._root_node_or_null,
        in_context._parent_node_or_null
        );

    return result;

}

UiInstanceMainMenu::UiInstanceMainMenu(
    DscUi::UiManager& in_ui_manager,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    DscCommon::FileSystem& in_file_system,
    DscDag::NodeToken in_data_source,
    DscDag::NodeToken in_root_node_or_null,
    DscDag::NodeToken in_parent_node_or_null
)
    : _ui_manager(in_ui_manager)
    , _draw_system(in_draw_system)
    , _dag_collection(in_dag_collection)
    , _parent_node_group(in_parent_node_or_null)
{
    DSC_UNUSED(in_data_source);

    _main_node_group = _ui_manager.AddChildNode(
        DscUi::MakeComponentCanvas(
        ).SetCrossfadeChildAmount(
            0.0f
        ),
        _draw_system,
        _dag_collection,
        in_root_node_or_null,
        in_parent_node_or_null,
        std::vector<DscUi::UiManager::TEffectConstructionHelper>()
        DSC_DEBUG_ONLY(DSC_COMMA "main menu node group")
    );

    {
        auto parent_resource_node = DscDag::DagNodeGroup::GetNodeTokenEnum(_parent_node_group, DscUi::TUiNodeGroup::TUiComponentResources);
        auto cross_fade_active_node = parent_resource_node ? DscDag::DagNodeGroup::GetNodeTokenEnum(parent_resource_node, DscUi::TUiComponentResourceNodeGroup::TCrossfadeActiveChild) : nullptr;
        if (nullptr != cross_fade_active_node)
        {
            DscDag::SetValueType(cross_fade_active_node, _main_node_group);
        }
    }

    {
        //auto background_texture = MakeShaderResource(*_file_system, *_draw_system, DscCommon::FileSystem::JoinPath("data", "background", "background_00.png"));
        auto background_texture = DscRenderResourcePng::MakeShaderResource(
            in_file_system,
            _draw_system,
            DscCommon::FileSystem::JoinPath("data", "background", "background_00.png")
            );
        _ui_manager.AddChildNode(
            DscUi::MakeComponentImage(
                background_texture
            ).SetChildSlot(
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 2.2222f, DscUi::UiCoord::TMethod::TSecondaryPoroportinal), DscUi::UiCoord(0, 1.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f))
            ),
            _draw_system,
            _dag_collection,
            in_root_node_or_null,
            _main_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "main menu texture")
        );
    }


}

UiInstanceMainMenu::~UiInstanceMainMenu()
{
    _ui_manager.RemoveDestroyChild(
        _dag_collection,
        _parent_node_group,
        _main_node_group
    );
}

void UiInstanceMainMenu::Update()
{
    //nop
}

DscDag::NodeToken UiInstanceMainMenu::GetDagUiGroupNode()
{
    return _main_node_group;
}

DscDag::NodeToken UiInstanceMainMenu::GetDagUiDrawNode()
{
    return DscDag::DagNodeGroup::GetNodeTokenEnum(_main_node_group, DscUi::TUiNodeGroup::TDrawNode);
}

DscDag::NodeToken UiInstanceMainMenu::GetDagUiDrawBaseNode()
{
    return DscDag::DagNodeGroup::GetNodeTokenEnum(_main_node_group, DscUi::TUiNodeGroup::TDrawBaseNode);
}
