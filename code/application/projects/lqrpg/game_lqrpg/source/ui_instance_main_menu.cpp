#include "ui_instance_main_menu.h"

#include "application.h"
#include "ui_instance.h"
#include "ui_instance_app.h"
#include "ui_instance_context.h"
#include <dsc_common/file_system.h>
#include <dsc_common/math.h>
#include <dsc_data/dsc_data.h>
#include <dsc_data/accessor.h>
#include <dsc_render_resource_png/dsc_render_resource_png.h>
#include <dsc_text/text_manager.h>
#include <dsc_text/text_run.h>
#include <dsc_text/i_text_run.h>
#include <dsc_ui/component_construction_helper.h>
#include <dsc_ui/ui_manager.h>

#if defined(_DEBUG)
template <>
const DscDag::DagNodeGroupMetaData& DscDag::GetDagNodeGroupMetaData(const UiInstanceMainMenu::TUiNodeGroupDataSource in_value)
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
    case UiInstanceMainMenu::TUiNodeGroupDataSource::TTitle:
    {
        // do we spit out strings of TextRun
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(std::string) };
        return s_meta_data;
    }
    case UiInstanceMainMenu::TUiNodeGroupDataSource::TSubTitle:
    {
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(std::string) };
        return s_meta_data;
    }
    case UiInstanceMainMenu::TUiNodeGroupDataSource::TButtonArray:
    {
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(std::vector<UiInstance::TButtonData>) };
        return s_meta_data;
    }
    }
    static DscDag::DagNodeGroupMetaData s_dummy = { false, typeid(nullptr) };
    return s_dummy;
}
#endif //#if defined(_DEBUG)

namespace
{
}


const std::string UiInstanceMainMenu::GetTemplateName()
{
    return "main menu";
}

DscDag::NodeToken UiInstanceMainMenu::BuildDataSource(
    DscDag::DagCollection& in_dag_collection,
    DscDag::IDagOwner* const in_data_source_owner,
    DscDag::NodeToken in_root_data_source_node
)
{
    DSC_UNUSED(in_root_data_source_node);

    auto result = in_dag_collection.CreateGroupEnum<UiInstanceMainMenu::TUiNodeGroupDataSource, DscUi::TUiNodeGroupDataSource>(in_data_source_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result, "data source main screen"));
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

    // title
    {
        auto node = UiInstance::MakeLocaleKey(in_dag_collection, dag_owner, in_root_data_source_node, "main menu title");
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            UiInstanceMainMenu::TUiNodeGroupDataSource::TTitle,
            node
        );
    }

    // sub title
    {
        auto node = UiInstance::MakeLocaleKey(in_dag_collection, dag_owner, in_root_data_source_node, "main menu sub title");
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            UiInstanceMainMenu::TUiNodeGroupDataSource::TSubTitle,
            node
        );
    }

    // button data
    {
        std::vector<UiInstance::TButtonData> button_data_array = {};

        //Character
        {
            auto text_node = UiInstance::MakeLocaleKey(in_dag_collection, dag_owner, in_root_data_source_node, "character");
            auto function = [in_root_data_source_node](DscDag::NodeToken,const DscCommon::VectorFloat2 &) {
                UiInstanceApp::DataSourceMainScreenStackPush(in_root_data_source_node,
                    DscDag::DagNodeGroup::GetNodeTokenEnum(
                        in_root_data_source_node,
                        UiInstanceApp::TUiNodeGroupDataSource::TCharacterDataSource
                    )
                );
            };
            button_data_array.push_back({ text_node , function });
        }
        //Combat
        {
            auto text_node = UiInstance::MakeLocaleKey(in_dag_collection, dag_owner, in_root_data_source_node, "combat");
            auto function = [in_root_data_source_node](DscDag::NodeToken,const DscCommon::VectorFloat2 &) {
                UiInstanceApp::DataSourceMainScreenStackPush(in_root_data_source_node,
                    DscDag::DagNodeGroup::GetNodeTokenEnum(
                        in_root_data_source_node,
                        UiInstanceApp::TUiNodeGroupDataSource::TCombatDataSource
                    )
                );
            };
            button_data_array.push_back({ text_node , function });
        }
        //Options
        {
            auto text_node = UiInstance::MakeLocaleKey(in_dag_collection, dag_owner, in_root_data_source_node, "options");
            auto function = [in_root_data_source_node](DscDag::NodeToken,const DscCommon::VectorFloat2 &) {
                UiInstanceApp::DataSourceDialogStackPush(in_root_data_source_node,
                    DscDag::DagNodeGroup::GetNodeTokenEnum(
                        in_root_data_source_node,
                        UiInstanceApp::TUiNodeGroupDataSource::TOptionsDataSource
                        )
                    );
            };
            button_data_array.push_back({ text_node , function });
        }
        // Exit
        {
            auto text_node = UiInstance::MakeLocaleKey(in_dag_collection, dag_owner, in_root_data_source_node, "exit");
            auto function = [in_root_data_source_node](DscDag::NodeToken,const DscCommon::VectorFloat2 &) {

				DSC_LOG_DIAGNOSTIC(LOG_TOPIC_APPLICATION, "Exit\n");
                UiInstanceApp::DataSourceMainScreenStackClear(in_root_data_source_node);

            };
            button_data_array.push_back({ text_node , function });
        }
        auto node = in_dag_collection.CreateValueNone(button_data_array, dag_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "button array"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            UiInstanceMainMenu::TUiNodeGroupDataSource::TButtonArray,
            node
        );
    }

    return result;
}

std::shared_ptr<DscUi::IUiInstance> UiInstanceMainMenu::Factory(
    const DscUi::UiInstanceFactory<UiInstanceContext>& in_ui_instance_factory,
    const UiInstanceContext& in_context
)
{
    std::shared_ptr<DscUi::IUiInstance> result = std::make_shared<UiInstanceMainMenu>(
        in_ui_instance_factory,
        in_context
        );

    return result;
}

UiInstanceMainMenu::UiInstanceMainMenu(
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

    _main_node_group = _ui_manager.AddChildNode(
        DscUi::MakeComponentCanvas(
        ).SetCrossfadeChildAmount(
            0.001f // just for safety, moving to only delete cross fade children when child amount is zero
        ),
        _draw_system,
        _dag_collection,
        in_context._root_node_or_null,
        in_context._parent_node_or_null,
        std::vector<DscUi::UiManager::TEffectConstructionHelper>()
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

    // background image
    {
        auto background_texture = DscRenderResourcePng::MakeShaderResource(
            *in_context._file_system,
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
            in_context._root_node_or_null,
            _main_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "main menu texture")
        );
    }

    // stack node
    DscDag::NodeToken stack_node = nullptr;
    {
        stack_node = _ui_manager.AddChildNode(
            DscUi::MakeComponentStack(
                DscUi::TUiFlow::TVertical, DscUi::UiCoord(0, 0.0f)
            ).SetChildSlot(
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(-16, 1.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f))
            ),
            _draw_system,
            _dag_collection,
            in_context._root_node_or_null,
            _main_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "main menu stack")
        );
    }

    // title
    {
        auto& text_manager = *in_context._text_manager;
        auto& file_system = *in_context._file_system;
        auto text = in_context._dag_collection->CreateCalculate<std::shared_ptr<DscText::TextRun>>(
                [&text_manager, &file_system]
                (std::shared_ptr<DscText::TextRun>& output, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                    const DscLocale::LocaleISO_639_1 locale = DscDag::GetValueType<DscLocale::LocaleISO_639_1>(in_input_array[0]);
                    const std::string& font_path = DscDag::GetValueType<std::string>(in_input_array[1]);
                    const std::string& message = DscDag::GetValueType<std::string>(in_input_array[2]);

                    const DscText::TextLocale* const pLocale = text_manager.GetLocaleToken(locale);
                    DscText::GlyphCollectionText* font = text_manager.LoadFont(file_system, font_path);

                    std::vector<std::unique_ptr<DscText::ITextRun>> text_run_array;
                    text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
                        message,
                        pLocale,
                        font,
                        80,
                        DscCommon::Math::ConvertColourToInt(255, 255, 255, 255)//,
                        //60,
                        //30
                    ));

                    text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
                        "\xE2" "\x84" "\xA2",
                        pLocale,
                        font,
                        32,
                        DscCommon::Math::ConvertColourToInt(255, 255, 255, 255),
                        0,
                        0,
                        24
                    ));

                    DscCommon::VectorInt2 container_size = {};
                    const int32 current_width = 0;
                    output = std::make_shared<DscText::TextRun>(
                        std::move(text_run_array),
                        container_size,
                        true,
                        current_width,
                        DscText::THorizontalAlignment::TMiddle,
                        DscText::TVerticalAlignment::TMiddle
                        );
            }, dynamic_cast<DscDag::IDagOwner*>(stack_node)
            );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(text, "title node"));
        DscDag::LinkIndexNodes(0, DscDag::DagNodeGroup::GetNodeTokenEnum(in_context._root_data_source_node_group, UiInstanceApp::TUiNodeGroupDataSource::TLocale), text);
        DscDag::LinkIndexNodes(1, DscDag::DagNodeGroup::GetNodeTokenEnum(in_context._root_data_source_node_group, UiInstanceApp::TUiNodeGroupDataSource::TFontPath), text);
        DscDag::LinkIndexNodes(2, DscDag::DagNodeGroup::GetNodeTokenEnum(data_source, TUiNodeGroupDataSource::TTitle), text);

        std::vector<DscUi::UiManager::TEffectConstructionHelper> effect_array = {};
        effect_array.push_back({
            DscUi::TUiEffectType::TEffectStroke, 
            DscCommon::VectorFloat4(4.0f, 0.0f, 0.0f, 0.0f),
            DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 1.0f)
            });
        _ui_manager.AddChildNode(
            DscUi::MakeComponentTextNode(
                text,
                in_context._text_manager,
                false
            ).SetUiScaleByWidth(
                800,
                0.00125f
            ).SetPadding(
                DscUi::UiCoord(0, 0.0f),
                DscUi::UiCoord(4, 0.0f),
                DscUi::UiCoord(0, 0.0f),
                DscUi::UiCoord(4, 0.0f)
            //).SetClearColour(
            //    DscCommon::VectorFloat4(0.0f, 0.5f, 0.0f, 1.0f)
            ),
            _draw_system,
            _dag_collection,
            in_context._root_node_or_null,
            stack_node,
            effect_array
            DSC_DEBUG_ONLY(DSC_COMMA "title node")
        );
    }

    // sub title
    {
        auto& text_manager = *in_context._text_manager;
        auto& file_system = *in_context._file_system;
        auto text = in_context._dag_collection->CreateCalculate<std::shared_ptr<DscText::TextRun>>(
            [&text_manager, &file_system]
        (std::shared_ptr<DscText::TextRun>& output, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscLocale::LocaleISO_639_1 locale = DscDag::GetValueType<DscLocale::LocaleISO_639_1>(in_input_array[0]);
            const std::string& font_path = DscDag::GetValueType<std::string>(in_input_array[1]);
            const std::string& message = DscDag::GetValueType<std::string>(in_input_array[2]);

            const DscText::TextLocale* const pLocale = text_manager.GetLocaleToken(locale);
            DscText::GlyphCollectionText* font = text_manager.LoadFont(file_system, font_path);

            std::vector<std::unique_ptr<DscText::ITextRun>> text_run_array;
            text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
                message,
                pLocale,
                font,
                20,
                DscCommon::Math::ConvertColourToInt(0, 0, 0, 255)
            ));

            DscCommon::VectorInt2 container_size = {};
            const int32 current_width = 0;
            output = std::make_shared<DscText::TextRun>(
                std::move(text_run_array),
                container_size,
                true,
                current_width,
                DscText::THorizontalAlignment::TMiddle,
                DscText::TVerticalAlignment::TMiddle
                );
        }, dynamic_cast<DscDag::IDagOwner*>(stack_node)
            );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(text, "sub title node"));
        DscDag::LinkIndexNodes(0, DscDag::DagNodeGroup::GetNodeTokenEnum(in_context._root_data_source_node_group, UiInstanceApp::TUiNodeGroupDataSource::TLocale), text);
        DscDag::LinkIndexNodes(1, DscDag::DagNodeGroup::GetNodeTokenEnum(in_context._root_data_source_node_group, UiInstanceApp::TUiNodeGroupDataSource::TFontPath), text);
        DscDag::LinkIndexNodes(2, DscDag::DagNodeGroup::GetNodeTokenEnum(data_source, TUiNodeGroupDataSource::TSubTitle), text);

        std::vector<DscUi::UiManager::TEffectConstructionHelper> effect_array = {};
        _ui_manager.AddChildNode(
            DscUi::MakeComponentTextNode(
                text,
                in_context._text_manager,
                false
            ).SetPadding(
                DscUi::UiCoord(0, 0.0f),
                DscUi::UiCoord(8, 0.0f),
                DscUi::UiCoord(0, 0.0f),
                DscUi::UiCoord(8, 0.0f)
            ),
            _draw_system,
            _dag_collection,
            in_context._root_node_or_null,
            stack_node,
            effect_array
            DSC_DEBUG_ONLY(DSC_COMMA "sub title node")
        );
    }

    // buttons
    {
        DscDag::NodeToken button_array_data_node = DscDag::DagNodeGroup::GetNodeTokenEnum(data_source, TUiNodeGroupDataSource::TButtonArray);
        DSC_ASSERT(nullptr != button_array_data_node, "invalid state");
        const auto& button_array_data = DscDag::GetValueType<std::vector<UiInstance::TButtonData>>(button_array_data_node);
        for (const auto& data : button_array_data)
        {
            UiInstance::AddButton(
                data,
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
}

UiInstanceMainMenu::~UiInstanceMainMenu()
{
	DSC_LOG_DIAGNOSTIC(LOG_TOPIC_APPLICATION, "UiInstanceMainMenu dtor\n");
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

void UiInstanceMainMenu::Update()
{
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
