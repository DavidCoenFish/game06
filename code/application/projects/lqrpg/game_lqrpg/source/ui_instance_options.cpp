#include "ui_instance_options.h"

#include "application.h"
#include "ui_instance.h"
#include "ui_instance_app.h"
#include "ui_instance_context.h"
#include <dsc_common/file_system.h>
#include <dsc_common/math.h>
#include <dsc_data/dsc_data.h>
#include <dsc_data/accessor.h>
#include <dsc_text/text_manager.h>
#include <dsc_text/text.h>
#include <dsc_text/i_text_run.h>
#include <dsc_ui/component_construction_helper.h>
#include <dsc_ui/ui_manager.h>

#if defined(_DEBUG)
template <>
const DscDag::DagNodeGroupMetaData& DscDag::GetDagNodeGroupMetaData(const UiInstanceOptions::TUiNodeGroupDataSource in_value)
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
    case UiInstanceOptions::TUiNodeGroupDataSource::TBack:
    {
        // do we spit out strings of Text
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(UiInstance::TButtonData) };
        return s_meta_data;
    }
    }
    static DscDag::DagNodeGroupMetaData s_dummy = { false, typeid(nullptr) };
    return s_dummy;
}
#endif //#if defined(_DEBUG)

const std::string UiInstanceOptions::GetTemplateName()
{
    return "options";
}

DscDag::NodeToken UiInstanceOptions::BuildDataSource(
    DscDag::DagCollection& in_dag_collection,
    DscDag::IDagOwner* const in_data_source_owner,
    DscDag::NodeToken in_root_data_source_node
)
{
    DSC_UNUSED(in_root_data_source_node);

    auto result = in_dag_collection.CreateGroupEnum<UiInstanceOptions::TUiNodeGroupDataSource, DscUi::TUiNodeGroupDataSource>(in_data_source_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result, "data source options"));
    // need to modify DagCollection::Delete to recurse IDagNode owner deletion?
    DscDag::IDagOwner* const dag_owner = dynamic_cast<DscDag::IDagOwner*>(result);

    // template name
    {
        auto node = in_dag_collection.CreateValueOnValueChange<std::string>(GetTemplateName(), dag_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, GetTemplateName()));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            DscUi::TUiNodeGroupDataSource::TTemplateName,
            node
        );
    }

    // back
    {
        auto text_node = UiInstance::MakeLocaleKey(in_dag_collection, dag_owner, in_root_data_source_node, "back");
        auto function = [in_root_data_source_node](DscDag::NodeToken, const DscCommon::VectorFloat2 &) {
            UiInstanceApp::DataSourceDialogStackPop(in_root_data_source_node);
        };
        UiInstance::TButtonData button_data({ text_node , function });

        auto node = in_dag_collection.CreateValueNone<UiInstance::TButtonData>({ text_node , function }, dag_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "back button"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            TUiNodeGroupDataSource::TBack,
            node
        );
    }

    return result;
}

std::shared_ptr<DscUi::IUiInstance> UiInstanceOptions::Factory(
    const DscUi::UiInstanceFactory<UiInstanceContext>& in_ui_instance_factory,
    const UiInstanceContext& in_context
)
{
    std::shared_ptr<DscUi::IUiInstance> result = std::make_shared<UiInstanceOptions>(
        in_ui_instance_factory,
        in_context
        );

    return result;
}


UiInstanceOptions::UiInstanceOptions(
    const DscUi::UiInstanceFactory<UiInstanceContext>& in_ui_instance_factory,
    const UiInstanceContext& in_context
)
    : _ui_manager(*in_context._ui_manager)
    , _draw_system(*in_context._draw_system)
    , _dag_collection(*in_context._dag_collection)
    , _root_node_group(in_context._root_node_or_null)
    , _parent_node_group(in_context._parent_node_or_null)
{
    DSC_UNUSED(in_ui_instance_factory);

    DscDag::NodeToken data_source = DscDag::GetValueType< DscDag::NodeToken>(in_context._data_source_node);

	//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_APPLICATION, "UiInstanceOptions ctor\n");
    _main_node_group = _ui_manager.AddChildNode(
        DscUi::MakeComponentCanvas(
        ).SetCrossfadeChildAmount(
            0.001f // just for safety, moving to only delete cross fade children when child amount is zero
        ).SetInputFlow(
            DscUi::TUiInputFlowBehaviour::TModal
        ).SetChildSlot(
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.75f), DscUi::UiCoord(0, 0.5f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f))
        ),
        _draw_system,
        _dag_collection,
        in_context._root_node_or_null,
        in_context._parent_node_or_null,
        std::vector<DscUi::UiManager::TEffectConstructionHelper>({
            {
                DscUi::TUiEffectType::TEffectDropShadow,
                DscCommon::VectorFloat4(2.0f, 8.0f, 6.0f, 0.0f),
                DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 1.0f)
            }
        })
        DSC_DEBUG_ONLY(DSC_COMMA "main menu node group")
    );

    // tell the parent node that we are the active crossfade node, null protected but also asuming parent might be crossfade
    {
        auto parent_resource_node = DscDag::DagNodeGroup::GetNodeTokenEnum(_parent_node_group, DscUi::TUiNodeGroup::TUiComponentResources);
        auto cross_fade_active_node = parent_resource_node ? DscDag::DagNodeGroup::GetNodeTokenEnum(parent_resource_node, DscUi::TUiComponentResourceNodeGroup::TCrossfadeActiveChild) : nullptr;
        if (nullptr != cross_fade_active_node)
        {
            DscDag::SetValueType(cross_fade_active_node, _main_node_group);
        }
    }

    _ui_manager.AddChildNode(
        DscUi::MakeComponentCanvas(
        ).SetChildSlot(
            DscUi::VectorUiCoord2(DscUi::UiCoord(-16, 1.0f), DscUi::UiCoord(-16, 1.0f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f))
        ).SetClearColour(
            DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 0.5f)
        ),
        _draw_system,
        _dag_collection,
        in_context._root_node_or_null,
        _main_node_group,
        std::vector<DscUi::UiManager::TEffectConstructionHelper>({
            {
                DscUi::TUiEffectType::TEffectCorner,
                DscCommon::VectorFloat4(32.0f, 32.0f, 32.0f, 32.0f)
            }
            })
        DSC_DEBUG_ONLY(DSC_COMMA "background")
    );

    _ui_manager.AddChildNode(
        DscUi::MakeComponentCelticKnot(
            32,
            DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 0.5f)
        ).SetChildSlot(
            DscUi::VectorUiCoord2(DscUi::UiCoord(-16, 1.0f), DscUi::UiCoord(-16, 1.0f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f))
        ),
        _draw_system,
        _dag_collection,
        in_context._root_node_or_null,
        _main_node_group,
        std::vector<DscUi::UiManager::TEffectConstructionHelper>({
            {
                DscUi::TUiEffectType::TEffectDropShadow,
                DscCommon::VectorFloat4(2.0f, 8.0f, 6.0f, 0.0f),
                DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 1.0f)
            }
            })
        DSC_DEBUG_ONLY(DSC_COMMA "celtic knot")
        );

    // stack node
    DscDag::NodeToken stack_node = nullptr;
    {
        stack_node = _ui_manager.AddChildNode(
            DscUi::MakeComponentStack(
                DscUi::TUiFlow::TVertical, DscUi::UiCoord(0, 0.0f)
            ).SetChildSlot(
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 0.5f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 1.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 1.0f))
            ),
            _draw_system,
            _dag_collection,
            in_context._root_node_or_null,
            _main_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "stack")
        );
    }

    // buttons
    {
        DscDag::NodeToken back_button_node = DscDag::DagNodeGroup::GetNodeTokenEnum(data_source, TUiNodeGroupDataSource::TBack);
        DSC_ASSERT(nullptr != back_button_node, "invalid state");
        const auto& button_data = DscDag::GetValueType<UiInstance::TButtonData>(back_button_node);
        UiInstance::AddButton(
            button_data,
            *in_context._ui_manager,
            *in_context._draw_system,
            *in_context._dag_collection,
            *in_context._file_system,
            *in_context._text_manager,
            in_context._root_node_or_null,
            stack_node,
            in_context._root_data_source_node_group
        );
    }
}

UiInstanceOptions::~UiInstanceOptions()
{
    // tell the parent node that we are no longer the active crossfade node, null protected but also asuming parent might be crossfade
    {
        auto parent_resource_node = DscDag::DagNodeGroup::GetNodeTokenEnum(_parent_node_group, DscUi::TUiNodeGroup::TUiComponentResources);
        auto cross_fade_active_node = parent_resource_node ? DscDag::DagNodeGroup::GetNodeTokenEnum(parent_resource_node, DscUi::TUiComponentResourceNodeGroup::TCrossfadeActiveChild) : nullptr;
        if (nullptr != cross_fade_active_node)
        {
            DscDag::SetValueType<DscDag::NodeToken>(cross_fade_active_node, nullptr);
        }
    }
}

void UiInstanceOptions::Update()
{
}

DscDag::NodeToken UiInstanceOptions::GetDagUiGroupNode()
{
    return _main_node_group;
}

DscDag::NodeToken UiInstanceOptions::GetDagUiDrawNode()
{
    return DscDag::DagNodeGroup::GetNodeTokenEnum(_main_node_group, DscUi::TUiNodeGroup::TDrawNode);
}

DscDag::NodeToken UiInstanceOptions::GetDagUiDrawBaseNode()
{
    return DscDag::DagNodeGroup::GetNodeTokenEnum(_main_node_group, DscUi::TUiNodeGroup::TDrawBaseNode);
}
