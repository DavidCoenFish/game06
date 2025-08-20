#include "component_construction_helper.h"
#include <dsc_common\math.h>
#include <dsc_dag\dag_collection.h>
#include <dsc_dag\debug_print.h>

DscDag::NodeToken DscUi::MakeComponentResourceGroup(
    DscDag::DagCollection& in_dag_collection,
    const DscUi::ComponentConstructionHelper& in_construction_helper,
    DscDag::NodeToken in_time_delta,
    DscDag::NodeToken in_ui_scale,
    DscDag::NodeToken in_parent,
    DscDag::NodeToken in_owner
)
{
    DscDag::IDagOwner* const top_owner = dynamic_cast<DscDag::IDagOwner*>(in_owner);
    DSC_ASSERT(nullptr != top_owner, "invalid state");

    DscDag::NodeToken component_resource_group = in_dag_collection.CreateGroupEnum<DscUi::TUiComponentResourceNodeGroup>(top_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(component_resource_group, "component_resource_group"));
    DscDag::IDagOwner* const owner = dynamic_cast<DscDag::IDagOwner*>(component_resource_group);
    DSC_ASSERT(nullptr != owner, "invalid state");

    {
        auto clear_colour = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._clear_colour,
            owner
            );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(clear_colour, "clear_colour"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group, 
            DscUi::TUiComponentResourceNodeGroup::TClearColour, 
            clear_colour
            );
    }

    if (true == in_construction_helper._has_scroll)
    {
        auto has_manual_scroll_x = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._has_manual_scroll_x,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(has_manual_scroll_x, "has_manual_scroll_x"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::THasManualScrollX,
            has_manual_scroll_x
        );

        auto manual_scroll_x = in_dag_collection.CreateValueOnValueChange(
            0.0f,
            owner
        );
        static int32 s_index = 0;
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(manual_scroll_x, std::string("manual_scroll_x") + std::to_string(s_index)));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TManualScrollX,
            manual_scroll_x
        );

        auto has_manual_scroll_y = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._has_manual_scroll_y,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(has_manual_scroll_y, "has_manual_scroll_y"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::THasManualScrollY,
            has_manual_scroll_y
        );

        auto manual_scroll_y = in_dag_collection.CreateValueOnValueChange(
            0.0f,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(manual_scroll_y, std::string("manual_scroll_y") + std::to_string(s_index)));
        s_index += 1;
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TManualScrollY,
            manual_scroll_y
        );
    }

    if (true == in_construction_helper._has_fill)
    {
        auto fill_colour = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._fill,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(fill_colour, "fill_colour"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TFillColour,
            fill_colour
        );
    }

    if (nullptr != in_construction_helper._texture)
    {
        auto texture = in_dag_collection.CreateValueOnSet(
            in_construction_helper._texture,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(texture, "texture"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TTexture,
            texture
        );
    }

    if (nullptr != in_construction_helper._text_run)
    {
        DSC_ASSERT(nullptr != in_construction_helper._text_manager, "invalid state");
        auto text = in_dag_collection.CreateValueOnSet(
            DscUi::TUiComponentTextData({ in_construction_helper._text_run , in_construction_helper._text_manager }),
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(text, "text"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TText,
            text
        );
    }
    else if (nullptr != in_construction_helper._text_run_node)
    {
        DSC_ASSERT(nullptr != in_construction_helper._text_manager, "invalid state");
        DscText::TextManager* text_manager = in_construction_helper._text_manager;
        auto text = in_dag_collection.CreateCalculate<DscUi::TUiComponentTextData>([text_manager](DscUi::TUiComponentTextData& output, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            output._text_manager = text_manager;
            output._text_run = DscDag::GetValueType<std::shared_ptr<DscText::TextRun>>(in_input_array[0]);
        }, owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(text, "text node"));

        DscDag::LinkIndexNodes(0, in_construction_helper._text_run_node, text);
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TText,
            text
        );
    }

    if (true == in_construction_helper._has_ui_scale_by_avaliable_width)
    {
        const int32 scale_width_low_threashhold = in_construction_helper._scale_width_low_threashhold;
        const float scale_factor = in_construction_helper._scale_factor;
        auto ui_scale_from_width_node = in_dag_collection.CreateCalculate<float>([scale_width_low_threashhold, scale_factor](float& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            float ui_scale = DscDag::GetValueType<float>(in_input_array[0]);
            const DscCommon::VectorInt2& avaliable_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            if (scale_width_low_threashhold < avaliable_size.GetX())
            {
                ui_scale = (1.0f + (static_cast<float>(avaliable_size.GetX() - scale_width_low_threashhold) * scale_factor));
            }

            value = ui_scale;
        }, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(ui_scale_from_width_node, "ui_scale_from_width"));
        DscDag::LinkIndexNodes(0, in_ui_scale, ui_scale_from_width_node);
        DSC_ASSERT(nullptr != in_parent, "invalid param");
        DscDag::NodeToken avaliable_size = DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TAvaliableSize);
        DscDag::LinkIndexNodes(1, avaliable_size, ui_scale_from_width_node);

        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TUiScale,
            ui_scale_from_width_node
        );
    }
    else
    {
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TUiScale,
            in_ui_scale
        );

    }

    if (true == in_construction_helper._has_child_slot_data)
    {
        auto child_slot_size = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._child_size,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(child_slot_size, "child_slot_size"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TChildSlotSize,
            child_slot_size
        );

        auto child_slot_pivot = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._child_pivot,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(child_slot_pivot, "child_slot_pivot"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TChildSlotPivot,
            child_slot_pivot
        );

        auto child_slot_parent_attach = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._attach_point,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(child_slot_parent_attach, "child_slot_parent_attach"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TChildSlotParentAttach,
            child_slot_parent_attach
        );
    }

    if (true == in_construction_helper._has_desired_size)
    {
        auto desired_size = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._desired_size,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(desired_size, "desired_size"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TDesiredSize,
            desired_size
        );
    }

    if (true == in_construction_helper._has_padding)
    {
        auto padding_left = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._padding_left,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(padding_left, "padding_left"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TPaddingLeft,
            padding_left
        );

        auto padding_right = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._padding_right,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(padding_right, "padding_right"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TPaddingRight,
            padding_right
        );

        auto padding_top = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._padding_top,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(padding_top, "padding_top"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TPaddingTop,
            padding_top
        );

        auto padding_bottom = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._padding_bottom,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(padding_bottom, "padding_bottom"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TPaddingBottom,
            padding_bottom
        );
    }

    if (true == in_construction_helper._has_gap)
    {
        auto gap = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._gap,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(gap, "gap"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TGap,
            gap
        );
    }

    if (true == in_construction_helper._has_gradient_fill)
    {
        auto gradient_fill = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._gradient_fill_constant_buffer,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(gradient_fill, "gradient fill"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TGradientFill,
            gradient_fill
        );
    }

    if (true == in_construction_helper._has_multi_gradient_fill)
    {
        auto gradient_fill = in_dag_collection.CreateCalculate<DscUi::TGradientFillConstantBuffer>([](DscUi::TGradientFillConstantBuffer& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                if (nullptr == in_input_array[0])
                {
                    // do we need a warning
                    DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "multi gradent without a parent input state");
                    return;
                }
                const TUiInputStateFlag input_state = DscDag::GetValueType<TUiInputStateFlag>(in_input_array[0]);
                const std::vector<DscUi::TGradientFillConstantBuffer>& array_gradient = DscDag::GetValueType<std::vector<DscUi::TGradientFillConstantBuffer>>(in_input_array[1]);

                const int32 index = static_cast<int32>(input_state);
                DSC_ASSERT((0 <= index) && (index < static_cast<int32>(array_gradient.size())), "invalid state");
                value = array_gradient[index];
            }, 
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(gradient_fill, "gradient fill calculate"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TGradientFill,
            gradient_fill
        );

        DSC_ASSERT(nullptr != in_parent, "invalid param");
        DscDag::NodeToken parent_resource_group = DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TUiComponentResources);
        DscDag::NodeToken parent_input_state_flag = DscDag::DagNodeGroup::GetNodeTokenEnum(parent_resource_group, TUiComponentResourceNodeGroup::TInputStateFlag);
        DscDag::LinkIndexNodes(0, parent_input_state_flag, gradient_fill); // set with the parent TUiInputStateFlag

        DSC_ASSERT(nullptr != in_construction_helper._multi_gradient_fill_node, "invalid param");
        // is there a better way to check type?
        DscDag::GetValueType<std::vector<DscUi::TGradientFillConstantBuffer>>(in_construction_helper._multi_gradient_fill_node);
        DscDag::LinkIndexNodes(1, in_construction_helper._multi_gradient_fill_node, gradient_fill);
    }

    if (DscUi::TUiFlow::TCount != in_construction_helper._flow_direction)
    {
        auto flow_direction = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._flow_direction,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(flow_direction, "flow_direction"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TFlow,
            flow_direction
        );
    }

    if (true == in_construction_helper._has_child_stack_data)
    {
        auto stack_size = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._stack_size,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(stack_size, "child stack size"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TChildStackSize,
            stack_size
        );

        auto stack_pivot = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._stack_pivot,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(stack_pivot, "child stack pivot"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TChildStackPivot,
            stack_pivot
        );

        auto stack_parent_attach = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._stack_parent_attach_point,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(stack_parent_attach, "child stack parent attach"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TChildStackParentAttach,
            stack_parent_attach
        );
    }

    if (true == in_construction_helper._has_input)
    {
        auto input_state_flag = in_dag_collection.CreateValueOnValueChange(
            DscUi::TUiInputStateFlag::TNone,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(input_state_flag, "input state flag"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TInputStateFlag,
            input_state_flag
        );

        if (nullptr != in_construction_helper._input_click_callback)
        {
            auto input_data = in_dag_collection.CreateValueNone(
                DscUi::TUiComponentInputData({ in_construction_helper._input_click_callback }),
                owner
            );
            DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(input_data, "input data"));
            DscDag::DagNodeGroup::SetNodeTokenEnum(
                component_resource_group,
                DscUi::TUiComponentResourceNodeGroup::TInputData,
                input_data
            );
        }
    }

    if (true == in_construction_helper._has_input_rollover_accumulate)
    {
        auto input_rollover_accumulate = in_dag_collection.CreateValueOnValueChange(
            0.0f,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(input_rollover_accumulate, "input_rollover_accumulate"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TInputRolloverAccumulate,
            input_rollover_accumulate
        );
    }

    if (true == in_construction_helper._has_input_active_touch_pos)
    {
        auto input_active_touch_pos = in_dag_collection.CreateValueOnValueChange(
            DscCommon::VectorFloat2::s_zero,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(input_active_touch_pos, "input_active_touch_pos"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TInputActiveTouchPos,
            input_active_touch_pos
        );
    }

    if (true == in_construction_helper._has_crossfade_child_amount) 
    {
        auto crossfade_child_amount = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._crossfade_child_amount,
            owner
        );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(crossfade_child_amount, "crossfade_child_amount"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TCrossfadeChildAmount,
            crossfade_child_amount
        );

        auto crossfade_child_panel_tint = in_dag_collection.CreateCalculate<DscCommon::VectorFloat4>([](DscCommon::VectorFloat4& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                float amount = DscDag::GetValueType<float>(in_input_array[0]);
                amount = sqrt(amount);
                value = DscCommon::VectorFloat4(amount, amount, amount, amount);
            },
            owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(crossfade_child_panel_tint, "crossfade_child_panel_tint"));
        DscDag::LinkIndexNodes(0,
            DscDag::DagNodeGroup::GetNodeTokenEnum(component_resource_group, DscUi::TUiComponentResourceNodeGroup::TCrossfadeChildAmount),
            crossfade_child_panel_tint);

        DscDag::DagNodeGroup::SetNodeTokenEnum(
            in_owner,
            DscUi::TUiNodeGroup::TUiPanelTint,
            crossfade_child_panel_tint
        );
    }

    // the crossfade, on a change to TUiRootNodeGroup::TArrayChildUiNodeGroup, how do we set the child crossfade amount
    // we need tick, child array, and the _crossfade_active_child node
    if (TUiComponentType::TCrossFade == in_construction_helper._component_type)
    {
        DscDag::NodeToken crossfade_active_child = in_dag_collection.CreateValueOnValueChange<DscDag::NodeToken>(
            in_construction_helper._crossfade_active_child,
            owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(crossfade_active_child, "crossfade active child"));

        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TCrossfadeActiveChild,
            crossfade_active_child
        );

        DscDag::NodeToken condition = in_dag_collection.CreateCalculate<bool>([](bool& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const DscDag::NodeToken child_active_node = DscDag::GetValueType<DscDag::NodeToken>(in_input_array[0]);
                const std::vector<DscDag::NodeToken>& child_array = DscDag::GetValueNodeArray(in_input_array[1]);

                bool data_correct = true;
                for (const auto& child : child_array)
                {
                    const bool active = (child == child_active_node);
                    DscDag::NodeToken child_resource_node_group = DscDag::DagNodeGroup::GetNodeTokenEnum(child, TUiNodeGroup::TUiComponentResources);
                    DscDag::NodeToken child_crossfade_amount_node = DscDag::DagNodeGroup::GetNodeTokenEnum(child_resource_node_group, TUiComponentResourceNodeGroup::TCrossfadeChildAmount);
                    const float cross_fade_amount = DscDag::GetValueType<float>(child_crossfade_amount_node);
                    if (true == active)
                    {
                        data_correct &= (1.0f == cross_fade_amount);
                    }
                    else
                    {
                        data_correct &= (0.0f == cross_fade_amount);
                    }
                }
                // we return true when the data is not correct, and fade needs to be adjusted
                out_value = (false == data_correct);
            },
            owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(condition, "crossfade condition"));

        DscDag::LinkIndexNodes(0, crossfade_active_child, condition);
        DscDag::LinkIndexNodes(
            1,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_owner, TUiNodeGroup::TArrayChildUiNodeGroup),
            condition);

        //DscDag::DagNodeGroup::SetNodeTokenEnum(
        //    component_resource_group,
        //    DscUi::TUiComponentResourceNodeGroup::TCrossfadeCondition,
        //    condition
        //    );

        DscDag::NodeToken pass_along_tick = in_dag_collection.CreateValueNotZero<float>(
            0.0f,
            owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(pass_along_tick, "pass_along_tick"));

        in_dag_collection.CreateCondition<float, bool>(
            condition,
            in_time_delta,
            nullptr,
            pass_along_tick,
            nullptr,
            owner);

        DscDag::NodeToken crossfade_node = in_dag_collection.CreateCalculate<bool>([](bool& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscDag::NodeToken child_active_node = DscDag::GetValueType<DscDag::NodeToken>(in_input_array[0]);
            //const DscDag::NodeToken child_active_node = DscDag::GetValueType<DscDag::NodeToken>(child_active_node_node);
            const std::vector<DscDag::NodeToken>& child_array = DscDag::GetValueNodeArray(in_input_array[1]);
            const float time_deta = DscDag::GetValueType<float>(in_input_array[2]);

            for (const auto& child : child_array)
            {
                const bool active = (child == child_active_node);
                DscDag::NodeToken child_resource_node_group = DscDag::DagNodeGroup::GetNodeTokenEnum(child, TUiNodeGroup::TUiComponentResources);
                DscDag::NodeToken child_crossfade_amount_node = DscDag::DagNodeGroup::GetNodeTokenEnum(child_resource_node_group, TUiComponentResourceNodeGroup::TCrossfadeChildAmount);
                float cross_fade_amount = DscDag::GetValueType<float>(child_crossfade_amount_node);
                cross_fade_amount += (active ? time_deta : -time_deta);
                cross_fade_amount = std::clamp(cross_fade_amount, 0.0f, 1.0f);
                DscDag::SetValueType<float>(child_crossfade_amount_node, cross_fade_amount);
            }

            value = true;
        },
            owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(crossfade_node, "crossfade calculate child fade amount"));

        DscDag::LinkIndexNodes(0, crossfade_active_child, crossfade_node);
        DscDag::LinkIndexNodes(
            1,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_owner, TUiNodeGroup::TArrayChildUiNodeGroup),
            crossfade_node);
        DscDag::LinkIndexNodes(2, pass_along_tick, crossfade_node);

        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TCrossfadeNode,
            crossfade_node
        );
    }

    if (TUiComponentType::TCelticKnotFill == in_construction_helper._component_type)
    {
		const int32 knot_size_pixels = in_construction_helper._celtic_knot_size_pixels;
		DSC_ASSERT(0 < knot_size_pixels, "invalid state");
        auto knot_size_node = in_dag_collection.CreateCalculate<int32>([knot_size_pixels](int32& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
			const float ui_scale = DscDag::GetValueType<float>(in_input_array[0]);
            value = DscCommon::Math::ScaleInt(knot_size_pixels, ui_scale);
        },
            owner
            );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(knot_size_node, "celtic knot size calculate"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TCelticKnotSize,
            knot_size_node
        );
        DscDag::LinkIndexNodes(0, in_ui_scale, knot_size_node);


        DscDag::NodeToken knot_tint_node = in_dag_collection.CreateValueOnValueChange(
            in_construction_helper._celtic_knot_tint,
            owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(knot_tint_node, "knot_tint"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(
            component_resource_group,
            DscUi::TUiComponentResourceNodeGroup::TCelticKnotTint,
            knot_tint_node
        );
    }

    return component_resource_group;
}

DscUi::ComponentConstructionHelper DscUi::MakeComponentDebugGrid()
{
    return ComponentConstructionHelper({ TUiComponentType::TDebugGrid});
}

DscUi::ComponentConstructionHelper DscUi::MakeComponentFill(const DscCommon::VectorFloat4& in_colour)
{
    ComponentConstructionHelper result({ TUiComponentType::TFill });
    result._fill = in_colour;
    result._has_fill = true;
    return result;
}

DscUi::ComponentConstructionHelper DscUi::MakeComponentGradientFill(
    const TGradientFillConstantBuffer& in_gradient_fill
)
{
    ComponentConstructionHelper result({ TUiComponentType::TGradientFill });
    result._has_gradient_fill = true;
    result._gradient_fill_constant_buffer = in_gradient_fill;
    return result;
}

DscUi::ComponentConstructionHelper DscUi::MakeComponentMultiGradientFill(
    DscDag::NodeToken in_multi_gradient_fill_node
)
{
    ComponentConstructionHelper result({ TUiComponentType::TGradientFill });
    result._has_multi_gradient_fill = true;
    result._multi_gradient_fill_node = in_multi_gradient_fill_node;
    return result;
}

DscUi::ComponentConstructionHelper DscUi::MakeComponentImage(const std::shared_ptr<DscRenderResource::ShaderResource>& in_texture)
{
    ComponentConstructionHelper result({ TUiComponentType::TImage });
    result._texture = in_texture;
    return result;
}

DscUi::ComponentConstructionHelper DscUi::MakeComponentCanvas()
{
    ComponentConstructionHelper result({ TUiComponentType::TCanvas });
    return result;
}

DscUi::ComponentConstructionHelper DscUi::MakeComponentText(
    const std::shared_ptr<DscText::TextRun>& in_text_run,
    DscText::TextManager* const in_text_manager,
    const bool in_has_scroll
)
{
    ComponentConstructionHelper result({ TUiComponentType::TText });
    result._text_run = in_text_run;
    result._text_manager = in_text_manager;
    result._has_scroll = in_has_scroll;
    return result;
}

DscUi::ComponentConstructionHelper DscUi::MakeComponentTextNode(
    DscDag::NodeToken in_text_run_node,
    DscText::TextManager* const in_text_manager, // so, either the text manager needs to be told to upload the glyph texture before draw and we can grab the text shader pointer, or our draw method needs a ref to the text manager
    const bool in_has_scroll
)
{
    ComponentConstructionHelper result({ TUiComponentType::TTextNode });
    result._text_run_node = in_text_run_node;
    result._text_manager = in_text_manager;
    result._has_scroll = in_has_scroll;
    return result;
}

DscUi::ComponentConstructionHelper DscUi::MakeComponentStack(
    const TUiFlow in_flow_direction,
    const UiCoord& in_gap,
    const bool in_desired_size_from_children_max,
    const bool in_has_scroll
)
{
    ComponentConstructionHelper result({ TUiComponentType::TStack });
    result._flow_direction = in_flow_direction;
    result._has_gap = true;
    result._gap = in_gap;
    result._desired_size_from_children_max = in_desired_size_from_children_max;
    result._has_scroll = in_has_scroll;
    return result;
}

DscUi::ComponentConstructionHelper DscUi::MakeComponentCrossfade(
    DscDag::NodeToken in_crossfade_active_child
    )
{
    ComponentConstructionHelper result({ TUiComponentType::TCrossFade });
    result._crossfade_active_child = in_crossfade_active_child;
    return result;
}

DscUi::ComponentConstructionHelper DscUi::MakeComponentCelticKnot(
    const int32 in_knot_size_pixels,
    const DscCommon::VectorFloat4& in_knot_tint
)
{
    ComponentConstructionHelper result({ TUiComponentType::TCelticKnotFill });
    result._celtic_knot_size_pixels = in_knot_size_pixels;
    result._celtic_knot_tint = in_knot_tint;
    return result;
}
