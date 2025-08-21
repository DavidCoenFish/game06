#include "ui_instance.h"
#include "ui_instance_app.h"
#include "ui_instance_context.h"
#include <dsc_common/enum_soft_bind.h>
#include <dsc_common/file_system.h>
#include <dsc_common/math.h>
#include <dsc_data/accessor.h>
#include <dsc_data/dsc_data.h>
#include <dsc_locale/dsc_locale.h>
#include <dsc_ui/component_construction_helper.h>
#include <dsc_ui/ui_manager.h>
#include <dsc_text/text_manager.h>
#include <dsc_text/text_run.h>
#include <dsc_text/i_text_run.h>

DscDag::NodeToken UiInstance::MakeLocaleKey(
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

void UiInstance::AddButton(
    const TButtonData& in_data,
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
            , 0, 0, 15
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
