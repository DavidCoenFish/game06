#include "ui_instance_app.h"
#include "ui_instance_context.h"
#include <dsc_common/enum_soft_bind.h>
#include <dsc_common/file_system.h>
#include <dsc_data/accessor.h>
#include <dsc_data/dsc_data.h>
#include <dsc_locale/dsc_locale.h>
#include <dsc_ui/component_construction_helper.h>
#include <dsc_ui/ui_manager.h>

namespace
{
    const float UpdateCrossFadeChildren(
        DscUi::UiManager& in_ui_manager,
        DscDag::DagCollection& in_dag_collection,
        DscDag::NodeToken in_cross_fade
    )
    {
        float cross_fade_sum = 0.0f;
        DscDag::NodeToken child_array_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_cross_fade, DscUi::TUiNodeGroup::TArrayChildUiNodeGroup);
        if (nullptr != child_array_node)
        {
            std::vector<DscDag::NodeToken> delete_node_array = {};
            const std::vector<DscDag::NodeToken>& child_array = DscDag::GetValueNodeArray(child_array_node);
            for (const auto& child : child_array)
            {
                DscDag::NodeToken child_resource_node_group = DscDag::DagNodeGroup::GetNodeTokenEnum(child, DscUi::TUiNodeGroup::TUiComponentResources);
                DscDag::NodeToken child_crossfade_amount_node = DscDag::DagNodeGroup::GetNodeTokenEnum(child_resource_node_group, DscUi::TUiComponentResourceNodeGroup::TCrossfadeChildAmount);
                const float cross_fade_amount = DscDag::GetValueType<float>(child_crossfade_amount_node);
                cross_fade_sum += cross_fade_amount;
                if (0.0f == cross_fade_amount)
                {
                    delete_node_array.push_back(child);
                }
            }

            for (const auto& delete_node : delete_node_array)
            {
                in_ui_manager.RemoveDestroyChild(
                    in_dag_collection,
                    in_cross_fade,
                    delete_node
                );
            }
        }
        return std::min(1.0f, cross_fade_sum);
    }
}


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
        // see DscCommon::EnumSoftBind<DscLocale::LocaleISO_639_1> for conversion to string
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(DscLocale::LocaleISO_639_1) };
        return s_meta_data;
    }
    case UiInstanceApp::TUiNodeGroupDataSource::TLocaleData:
    {
        // the DscData JsonValue of the locale parent, a child for each locale ["en", "bh", "es", ...]
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(DscData::JsonValue*) };
        return s_meta_data;
    }
    case UiInstanceApp::TUiNodeGroupDataSource::TFontPath:
    {
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(std::string) };
        return s_meta_data;
    }
    case UiInstanceApp::TUiNodeGroupDataSource::TMainScreenDataSourceNode: //dag node <NodeToken> of the active screen data source or null
    {
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(DscDag::NodeToken) };
        return s_meta_data;
    }
    case UiInstanceApp::TUiNodeGroupDataSource::TMainScreenDataSourceStack: // for back navigation, an array of the data sources we have navigated into
    {
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(std::vector<DscDag::NodeToken>) };
        return s_meta_data;
    }
    case UiInstanceApp::TUiNodeGroupDataSource::TDialogDataSourceStack:
    {
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(std::vector<DscDag::NodeToken>) };
        return s_meta_data;
    }
    case UiInstanceApp::TUiNodeGroupDataSource::TDialogDataSourceNode: // dag node <NodeToken> of the active dialog data source or null
    {
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(DscDag::NodeToken) };
        return s_meta_data;
    }
    case UiInstanceApp::TUiNodeGroupDataSource::TMainMenuDataSource: // NodeGroup of the main menu var
    {
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(DscDag::NodeToken) };
        return s_meta_data;
    }
    case UiInstanceApp::TUiNodeGroupDataSource::TCharacterDataSource:
    {
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(DscDag::NodeToken) };
        return s_meta_data;
    }
    case UiInstanceApp::TUiNodeGroupDataSource::TCombatDataSource:
    {
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(DscDag::NodeToken) };
        return s_meta_data;
    }
    case UiInstanceApp::TUiNodeGroupDataSource::TOptionsDataSource:
    {
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(DscDag::NodeToken) };
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
    DscDag::DagCollection& in_dag_collection,
    DscData::JsonValue& in_data
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
        //auto node = in_dag_collection.CreateValueOnValueChange(DscLocale::LocaleISO_639_1::Chinese_Simplified, data_source_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "locale"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            UiInstanceApp::TUiNodeGroupDataSource::TLocale,
            node
        );
    }

    //TLocaleData
    {
        const auto& data_locale = DscData::GetObjectChild(in_data, "locale");
        DSC_ASSERT(nullptr != data_locale, "invlaid state");
        DscData::JsonValue* const data_locale_ptr = data_locale.get();

        auto node = in_dag_collection.CreateCalculate<DscData::JsonValue*>([data_locale_ptr](DscData::JsonValue*& output, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscLocale::LocaleISO_639_1 locale = DscDag::GetValueType<DscLocale::LocaleISO_639_1>(in_input_array[0]);
            const std::string locale_key = DscCommon::EnumSoftBind<DscLocale::LocaleISO_639_1>::EnumToString(locale);

            const auto& data_locale_child = DscData::GetObjectChild(*data_locale_ptr, locale_key);
            output = data_locale_child.get();
            return;
        }, data_source_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "locale data"));
        DscDag::LinkIndexNodes(
            0,
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, UiInstanceApp::TUiNodeGroupDataSource::TLocale),
            node);

        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            UiInstanceApp::TUiNodeGroupDataSource::TLocaleData,
            node
        );
    }

    //TFontPath
    {
        auto node = in_dag_collection.CreateValueOnValueChange<std::string>(DscCommon::FileSystem::JoinPath("data", "font", "code2000.ttf"), data_source_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "font path"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            UiInstanceApp::TUiNodeGroupDataSource::TFontPath,
            node
        );
    }

    //TMainScreenDataSourceNode
    {
        auto node = in_dag_collection.CreateValueOnValueChange<DscDag::NodeToken>(nullptr, data_source_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "main screen data source node"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            UiInstanceApp::TUiNodeGroupDataSource::TMainScreenDataSourceNode,
            node
        );
    }

    //TMainScreenDataSourceStack
    {
        auto node = in_dag_collection.CreateValueNone<std::vector<DscDag::NodeToken>>(std::vector<DscDag::NodeToken>(), data_source_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "main screen data source stack"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            UiInstanceApp::TUiNodeGroupDataSource::TMainScreenDataSourceStack,
            node
        );
    }

    //TDialogDataSourceNode
    {
        auto node = in_dag_collection.CreateValueOnValueChange<DscDag::NodeToken>(nullptr, data_source_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "dialog data source node"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            UiInstanceApp::TUiNodeGroupDataSource::TDialogDataSourceNode,
            node
        );
    }

    //TMainScreenDataSourceStack
    {
        auto node = in_dag_collection.CreateValueNone<std::vector<DscDag::NodeToken>>(std::vector<DscDag::NodeToken>(), data_source_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "dialog data source stack"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            UiInstanceApp::TUiNodeGroupDataSource::TDialogDataSourceStack,
            node
        );
    }

    return result;
}


void UiInstanceApp::DataSourceMainScreenStackPush(DscDag::NodeToken in_data_source, DscDag::NodeToken in_main_screen)
{
    DSC_ASSERT(nullptr != in_data_source, "invalid param");
    DSC_ASSERT(nullptr != in_main_screen, "invalid param");
    std::vector<DscDag::NodeToken>& stack = DscDag::GetValueNonConstRef<std::vector<DscDag::NodeToken>>(DscDag::DagNodeGroup::GetNodeTokenEnum(in_data_source, TUiNodeGroupDataSource::TMainScreenDataSourceStack), true);
    stack.push_back(in_main_screen);

    DscDag::NodeToken main_screen_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_data_source, TUiNodeGroupDataSource::TMainScreenDataSourceNode);
    DscDag::SetValueType(main_screen_node, in_main_screen);
}

void UiInstanceApp::DataSourceMainScreenStackPop(DscDag::NodeToken in_data_source)
{
    DSC_ASSERT(nullptr != in_data_source, "invalid param");
    std::vector<DscDag::NodeToken>& stack = DscDag::GetValueNonConstRef<std::vector<DscDag::NodeToken>>(DscDag::DagNodeGroup::GetNodeTokenEnum(in_data_source, TUiNodeGroupDataSource::TMainScreenDataSourceStack), true);
    stack.pop_back();

    DscDag::NodeToken back = (0 < stack.size()) ? stack.back() : nullptr;

    DscDag::NodeToken main_screen_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_data_source, TUiNodeGroupDataSource::TMainScreenDataSourceNode);
    DscDag::SetValueType(main_screen_node, back);
}

void UiInstanceApp::DataSourceMainScreenStackClear(DscDag::NodeToken in_data_source)
{
    DSC_ASSERT(nullptr != in_data_source, "invalid param");
    std::vector<DscDag::NodeToken>& stack = DscDag::GetValueNonConstRef<std::vector<DscDag::NodeToken>>(DscDag::DagNodeGroup::GetNodeTokenEnum(in_data_source, TUiNodeGroupDataSource::TMainScreenDataSourceStack), true);
    stack.clear();

    DscDag::NodeToken main_screen_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_data_source, TUiNodeGroupDataSource::TMainScreenDataSourceNode);
    DscDag::SetValueType<DscDag::NodeToken>(main_screen_node, nullptr);
}

void UiInstanceApp::DataSourceDialogStackPush(DscDag::NodeToken in_data_source, DscDag::NodeToken in_dialog)
{
    DSC_ASSERT(nullptr != in_data_source, "invalid param");
    DSC_ASSERT(nullptr != in_dialog, "invalid param");
    std::vector<DscDag::NodeToken>& stack = DscDag::GetValueNonConstRef<std::vector<DscDag::NodeToken>>(DscDag::DagNodeGroup::GetNodeTokenEnum(in_data_source, TUiNodeGroupDataSource::TDialogDataSourceStack), true);
    stack.push_back(in_dialog);

    DscDag::NodeToken dialog_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_data_source, TUiNodeGroupDataSource::TDialogDataSourceNode);
    DscDag::SetValueType(dialog_node, in_dialog);
}

void UiInstanceApp::DataSourceDialogStackPop(DscDag::NodeToken in_data_source)
{
    DSC_ASSERT(nullptr != in_data_source, "invalid param");
    std::vector<DscDag::NodeToken>& stack = DscDag::GetValueNonConstRef<std::vector<DscDag::NodeToken>>(DscDag::DagNodeGroup::GetNodeTokenEnum(in_data_source, TUiNodeGroupDataSource::TDialogDataSourceStack), true);
    stack.pop_back();

    DscDag::NodeToken back = (0 < stack.size()) ? stack.back() : nullptr;

    DscDag::NodeToken dialog_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_data_source, TUiNodeGroupDataSource::TDialogDataSourceNode);
    DscDag::SetValueType<DscDag::NodeToken>(dialog_node, back);
}

void UiInstanceApp::DataSourceDialogStackClear(DscDag::NodeToken in_data_source)
{
    DSC_ASSERT(nullptr != in_data_source, "invalid param");
    std::vector<DscDag::NodeToken>& stack = DscDag::GetValueNonConstRef<std::vector<DscDag::NodeToken>>(DscDag::DagNodeGroup::GetNodeTokenEnum(in_data_source, TUiNodeGroupDataSource::TDialogDataSourceStack), true);
    stack.clear();

    DscDag::NodeToken dialog_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_data_source, TUiNodeGroupDataSource::TDialogDataSourceNode);
    DscDag::SetValueType(dialog_node, nullptr);
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
        _dag_collection
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
        ).SetHasEffectScale(
        ),
        _draw_system,
        _dag_collection,
        _root_node_group,
        _root_node_group,
        std::vector<DscUi::UiManager::TEffectConstructionHelper>({
            {DscUi::TUiEffectType::TEffectBlur}
            ,{DscUi::TUiEffectType::TEffectDesaturate}
            })
        DSC_DEBUG_ONLY(DSC_COMMA "app crossfade main")
    );

    _dialog_cross_fade = _ui_manager.AddChildNode(
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
        DSC_DEBUG_ONLY(DSC_COMMA "app crossfade dialog")
    );

    // make a node to build the main screen ui
    {
        DscDag::NodeToken main_screen_data_source_node = DscDag::DagNodeGroup::GetNodeTokenEnum(
            DscDag::GetValueType<DscDag::NodeToken>(in_context._data_source_node),
            TUiNodeGroupDataSource::TMainScreenDataSourceNode
        );

        UiInstanceContext context = in_context.MakeChild(
            main_screen_data_source_node,
            _main_screen_cross_fade
        );
        context._root_node_or_null = _root_node_group;

        _main_screen_factory_node = in_ui_instance_factory.BuildInstance(
            context
        );
    }

    // make a node to build the dialog
    {
        DscDag::NodeToken dialog_data_source_node = DscDag::DagNodeGroup::GetNodeTokenEnum(
            DscDag::GetValueType<DscDag::NodeToken>(in_context._data_source_node),
            TUiNodeGroupDataSource::TDialogDataSourceNode
        );

        UiInstanceContext context = in_context.MakeChild(
            dialog_data_source_node,
            _dialog_cross_fade
        );
        context._root_node_or_null = _root_node_group;

        _dialog_factory_node = in_ui_instance_factory.BuildInstance(
            context
        );
    }

    return;
}

UiInstanceApp::~UiInstanceApp()
{
    //DscDag::DebugPrintRecurseInputs(_root_node_group);
    //DscDag::DebugPrintRecurseOutputs(_root_node_group);

    _dag_collection.DeleteNode(_main_screen_factory_node);
    _main_screen_factory_node = nullptr;

    _dag_collection.DeleteNode(_dialog_factory_node);
    _dialog_factory_node = nullptr;

    _ui_manager.DestroyNode(
        _dag_collection,
        _root_node_group
    );
}

void UiInstanceApp::Update()
{
    // iterate over children of _main_screen_cross_fade, and remove the children that are at zero cross fade
    // for safety, do this BOTH before the factory node updates and in the child ui instances, create top level with non zero corss fade amount
    UpdateCrossFadeChildren(
        _ui_manager,
        _dag_collection,
        _main_screen_cross_fade
        );

    const float dialog_coverage = UpdateCrossFadeChildren(
        _ui_manager,
        _dag_collection,
        _dialog_cross_fade
    );

    if (nullptr != _main_screen_cross_fade)
    {
        DscDag::NodeToken component_resources = DscDag::DagNodeGroup::GetNodeTokenEnum(
            _main_screen_cross_fade, DscUi::TUiNodeGroup::TUiComponentResources);
        DscDag::NodeToken effect_strength_node = component_resources ? DscDag::DagNodeGroup::GetNodeTokenEnum(
            component_resources, DscUi::TUiComponentResourceNodeGroup::TEffectStrength) : nullptr;
        if (nullptr != effect_strength_node)
        {
            DscDag::SetValueType(effect_strength_node, dialog_coverage);
        }
    }

    if (nullptr != _main_screen_factory_node)
    {
        _main_screen_factory_node->Update();
    }

    if (nullptr != _dialog_factory_node)
    {
        _dialog_factory_node->Update();
    }

    return;
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

const bool UiInstanceApp::HasContent()
{
    //_main_screen_cross_fade
    {
        DscDag::NodeToken child_array_node = DscDag::DagNodeGroup::GetNodeTokenEnum(_main_screen_cross_fade, DscUi::TUiNodeGroup::TArrayChildUiNodeGroup);
        const std::vector<DscDag::NodeToken>& child_array = DscDag::GetValueNodeArray(child_array_node);
        if (0 != child_array.size())
        {
            return true;
        }
    }

    return false;
}

