#include "ui_instance_main_menu.h"
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
        static DscDag::DagNodeGroupMetaData s_meta_data = { false, typeid(std::vector<UiInstanceMainMenu::TButtonData>) };
        return s_meta_data;
    }
    }
    static DscDag::DagNodeGroupMetaData s_dummy = { false, typeid(nullptr) };
    return s_dummy;
}
#endif //#if defined(_DEBUG)

namespace
{
    DscDag::NodeToken MakeLocaleKey(
        DscDag::DagCollection& in_dag_collection,
        DscDag::IDagOwner* const in_data_source_owner,
        DscDag::NodeToken in_root_data_source_node,
        const std::string& in_locale_key
    )
    {
        auto node = in_dag_collection.CreateCalculate<std::string>([in_locale_key](std::string& output, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const DscData::JsonValue* locale_data = DscDag::GetValueType<DscData::JsonValue*>(in_input_array[0]);
                DSC_ASSERT(nullptr != locale_data, "invalid state");
                output = DscData::GetString(DscData::GetObjectChild(*locale_data, in_locale_key));
                return;
            },
            in_data_source_owner
            );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, in_locale_key));
        DscDag::LinkIndexNodes(
            0,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_data_source_node, UiInstanceApp::TUiNodeGroupDataSource::TLocaleData),
            node);

        return node;
    }

    void AddButton(
        const UiInstanceMainMenu::TButtonData& in_data,
        DscUi::UiManager& in_ui_manager,
        DscRender::DrawSystem& in_draw_system,
        DscDag::DagCollection& in_dag_collection,
        DscCommon::FileSystem& in_file_system,
        DscText::TextManager& in_text_manager,
        DscDag::NodeToken in_root_node,
        DscDag::NodeToken in_parent_node,
        DscDag::NodeToken in_root_data_source_node_group
    )
    {
        std::vector<DscUi::UiManager::TEffectConstructionHelper> array_button_effect = {};
        array_button_effect.push_back({
            DscUi::TUiEffectType::TEffectDropShadow,
            DscCommon::VectorFloat4(2.0f, 8.0f, 6.0f, 0.0f),
            DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 1.0f),
            true,
            DscCommon::VectorFloat4(1.0f, 4.0f, 0.0f, 0.0f),
            DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.8f)
            });

        auto button_node_group = in_ui_manager.AddChildNode(
            DscUi::MakeComponentCanvas(
            ).SetInputData(
                in_data._on_click,
                true,
                true
            ).SetChildStackData(
                DscUi::UiCoord(300, 0.0f),
                DscUi::UiCoord(0, 0.5f),
                DscUi::UiCoord(0, 0.5f)
            ).SetDesiredSize(
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(48, 0.0f))
            //).SetClearColour(
            //    DscCommon::VectorFloat4(1.0f, 0.0f, 0.0f, 1.0f)
            ),
            in_draw_system,
            in_dag_collection,
            in_root_node,
            in_parent_node,
            array_button_effect
            DSC_DEBUG_ONLY(DSC_COMMA "button")
        );

        std::vector<DscUi::UiManager::TEffectConstructionHelper> array_fill_effect = {};
        array_fill_effect.push_back({
            DscUi::TUiEffectType::TEffectCorner,
            DscCommon::VectorFloat4(8.0f, 8.0f, 8.0f, 8.0f)
            });
        array_fill_effect.push_back({
            DscUi::TUiEffectType::TEffectInnerShadow,
            DscCommon::VectorFloat4(0.0f, 0.0f, 8.0f, 0.0f),
            DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.5f)
            });

        DscDag::NodeToken multi_gradient = in_dag_collection.FetchNodeName("multi_gradient");
        if (nullptr == multi_gradient)
        {
            std::vector<DscUi::TGradientFillConstantBuffer> multi_gradient_data = {};
            multi_gradient_data.push_back(
                //in_gradient_fill_constant_buffer_none,
                {
                    {0.0f, 0.1f, 0.5f, 1.0f},
                    {0.557f, 0.314f, 0.208f, 1.0f},
                    {0.733f, 0.439f, 0.286f, 1.0f},
                    {0.941f, 0.627f, 0.396f, 1.0f},
                    {0.996f, 0.757f, 0.525f, 1.0f}
                }
            );
            multi_gradient_data.push_back(
                //in_gradient_fill_constant_buffer_rollover,
                {
                    {0.0f, 0.5f, 0.9f, 1.0f},
                    {0.557f, 0.314f, 0.208f, 1.0f},
                    {0.733f, 0.439f, 0.286f, 1.0f},
                    {0.941f, 0.627f, 0.396f, 1.0f},
                    {0.996f, 0.757f, 0.525f, 1.0f}
                }
            );
            multi_gradient_data.push_back(
                //in_gradient_fill_constant_buffer_click,
                {
                    {0.0f, 0.75f, 1.0f, 1.1f},
                    {0.839f, 0.529f, 0.361f, 1.0f},
                    {0.525f, 0.306f, 0.196f, 1.0f},
                    {0.255f, 0.098f, 0.051f, 1.0f},
                    {0.341f, 0.063f, 0.055f, 1.0f}
                }
            );
            multi_gradient_data.push_back(
                //in_gradient_fill_constant_buffer_rollover_click,
                {
                    {0.0f, 0.9f, 1.5f, 1.6f},
                    {0.839f, 0.529f, 0.361f, 1.0f},
                    {0.525f, 0.306f, 0.196f, 1.0f},
                    {0.255f, 0.098f, 0.051f, 1.0f},
                    {0.341f, 0.063f, 0.055f, 1.0f}
                }
            );
            multi_gradient_data.push_back(
                //in_gradient_fill_constant_buffer_selection,
                {}
            );
            multi_gradient_data.push_back(
                //in_gradient_fill_constant_buffer_rollover_selection,
                {}
            );
            multi_gradient_data.push_back(
                //in_gradient_fill_constant_buffer_click_selection,
                {});
            multi_gradient_data.push_back(
                //in_gradient_fill_constant_buffer_rollover_click_selection
                {}
            );

            multi_gradient = in_dag_collection.CreateValueNone(multi_gradient_data);
            DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(multi_gradient, "multi gadient data"));
            in_dag_collection.AddNodeName(multi_gradient, "multi_gradient");
        }

        const DscUi::UiCoord padding_ammount_top(0, 0.0f);
        const DscUi::UiCoord padding_ammount(16, 0.0f);

        in_ui_manager.AddChildNode(
            DscUi::MakeComponentMultiGradientFill(
                multi_gradient
            ).SetPadding(
                padding_ammount,
                padding_ammount_top,
                padding_ammount,
                padding_ammount
                //).SetClearColour(
                //    DscCommon::VectorFloat4(0.0f, 1.0f, 0.0f, 1.0f)
            ),
            in_draw_system,
            in_dag_collection,
            in_root_node,
            button_node_group,
            array_fill_effect
            DSC_DEBUG_ONLY(DSC_COMMA "button multi fill")
        );

        std::vector<DscUi::UiManager::TEffectConstructionHelper> array_text_effect = {};
        array_text_effect.push_back({
            DscUi::TUiEffectType::TEffectBurnBlot
            });
        array_text_effect.push_back({
            DscUi::TUiEffectType::TEffectBurnPresent
            });
        array_text_effect.push_back({
            DscUi::TUiEffectType::TEffectDropShadow,
            DscCommon::VectorFloat4(0.0f, 0.0f, 2.0f, 0.0f),
            DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 1.25f)
            });

        auto text = in_dag_collection.CreateCalculate<std::shared_ptr<DscText::TextRun>>(
            [&in_text_manager, &in_file_system]
            (std::shared_ptr<DscText::TextRun>& output, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const DscLocale::LocaleISO_639_1 locale = DscDag::GetValueType<DscLocale::LocaleISO_639_1>(in_input_array[0]);
                const std::string& font_path = DscDag::GetValueType<std::string>(in_input_array[1]);
                const std::string& message = DscDag::GetValueType<std::string>(in_input_array[2]);

                const DscText::TextLocale* const pLocale = in_text_manager.GetLocaleToken(locale);
                DscText::GlyphCollectionText* font = in_text_manager.LoadFont(in_file_system, font_path);

                std::vector<std::unique_ptr<DscText::ITextRun>> text_run_array;
                DscCommon::VectorInt2 container_size = {};

                text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
                    message,
                    pLocale,
                    font,
                    20,
                    DscCommon::Math::ConvertColourToInt(111, 28, 11, 255)//,
                    //20,
                    //10
                    ,0,0,15
                ));

                const int32 current_width = 0;
                output = std::make_shared<DscText::TextRun>(
                    std::move(text_run_array),
                    container_size,
                    true,
                    current_width,
                    DscText::THorizontalAlignment::TMiddle,
                    DscText::TVerticalAlignment::TMiddle
                    );

            }, dynamic_cast<DscDag::IDagOwner*>(button_node_group)
            );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(text, "text node"));

        DscDag::LinkIndexNodes(0, DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_data_source_node_group, UiInstanceApp::TUiNodeGroupDataSource::TLocale), text);
        DscDag::LinkIndexNodes(1, DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_data_source_node_group, UiInstanceApp::TUiNodeGroupDataSource::TFontPath), text);
        DscDag::LinkIndexNodes(2, in_data._button_text, text);

        in_ui_manager.AddChildNode(
            DscUi::MakeComponentTextNode(
                text,
                &in_text_manager
            ).SetChildSlot(
            //).SetClearColour(
            //    DscCommon::VectorFloat4(0.0f, 0.0f, 1.0f, 1.0f)
            ),
            in_draw_system,
            in_dag_collection,
            in_root_node,
            button_node_group,
            array_text_effect
            DSC_DEBUG_ONLY(DSC_COMMA "button text")
        );
    }
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
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "main menu"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            DscUi::TUiNodeGroupDataSource::TTemplateName,
            node
        );
    }

    // title
    {
        auto node = MakeLocaleKey(in_dag_collection, dag_owner, in_root_data_source_node, "main menu title");
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            UiInstanceMainMenu::TUiNodeGroupDataSource::TTitle,
            node
        );
    }

    // sub title
    {
        auto node = MakeLocaleKey(in_dag_collection, dag_owner, in_root_data_source_node, "main menu sub title");
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            result,
            UiInstanceMainMenu::TUiNodeGroupDataSource::TSubTitle,
            node
        );
    }

    // button data
    {
        std::vector<TButtonData> button_data_array = {};

        //Character
        {
            auto text_node = MakeLocaleKey(in_dag_collection, dag_owner, in_root_data_source_node, "character");
            auto function = [in_root_data_source_node](DscDag::NodeToken) {
            };
            button_data_array.push_back({ text_node , function });
        }
        //Combat
        {
            auto text_node = MakeLocaleKey(in_dag_collection, dag_owner, in_root_data_source_node, "combat");
            auto function = [in_root_data_source_node](DscDag::NodeToken) {
            };
            button_data_array.push_back({ text_node , function });
        }
        // Exit
        {
            auto text_node = MakeLocaleKey(in_dag_collection, dag_owner, in_root_data_source_node, "exit");
            auto function = [in_root_data_source_node](DscDag::NodeToken) {
                auto keep_going_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_data_source_node, UiInstanceApp::TUiNodeGroupDataSource::TKeepAppRunning);
                DscDag::SetValueType(keep_going_node, false);

                // clear the active screen, (which will cause the main screen ui to delete)
                // todo: make a copy node to hold the last UiRenderTarget?
                DscDag::SetValueType<DscDag::NodeToken>(
                    DscDag::DagNodeGroup::GetNodeTokenEnum(
                        in_root_data_source_node,
                        UiInstanceApp::TUiNodeGroupDataSource::TMainScreenDataSourceNode),
                    nullptr
                    );

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
    DSC_UNUSED(in_ui_instance_factory);

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
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
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
                        DscCommon::Math::ConvertColourToInt(255, 255, 255, 255),
                        50,
                        30
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
                20
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
        const auto& button_array_data = DscDag::GetValueType<std::vector<TButtonData>>(button_array_data_node);
        for (const auto& data : button_array_data)
        {
            AddButton(
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
    //todo: if our data source is not the main screen data source, if we are the active cross fade, clear the active cross fade
    // this code is here as in our ctor, we set the active cross fade on the parent, but we 

    // tell the parent node that we are no longer the active crossfade node, null protected but also asuming parent might be crossfade
    {
        auto parent_resource_node = DscDag::DagNodeGroup::GetNodeTokenEnum(_parent_node_group, DscUi::TUiNodeGroup::TUiComponentResources);
        auto cross_fade_active_node = parent_resource_node ? DscDag::DagNodeGroup::GetNodeTokenEnum(parent_resource_node, DscUi::TUiComponentResourceNodeGroup::TCrossfadeActiveChild) : nullptr;
        if (nullptr != cross_fade_active_node)
        {
            DscDag::SetValueType<DscDag::NodeToken>(cross_fade_active_node, nullptr);
        }
    }

#if 0
    _ui_manager.RemoveDestroyChild(
        _dag_collection,
        _parent_node_group,
        _main_node_group
    );
#endif
    //_ui_manager.AddChildNode(
    //    DscUi::MakeComponentFill(
    //        DscCommon::VectorFloat4(1.0f, 0.0f, 0.0f, 1.0f)
    //    ).SetCrossfadeChildAmount(
    //        1.0f
    //    ),
    //    _draw_system,
    //    _dag_collection,
    //    _root_node_group,
    //    _parent_node_group,
    //    std::vector<DscUi::UiManager::TEffectConstructionHelper>()
    //    DSC_DEBUG_ONLY(DSC_COMMA "main menu fade out place holder")
    //    );
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
