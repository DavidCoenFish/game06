#include "component_construction_helper.h"

DscUi::UiComponentResourceNodeGroup DscUi::MakeComponentResourceGroup(
    DscDag::DagCollection& in_dag_collection,
    const DscUi::ComponentConstructionHelper& in_construction_helper,
    DscDag::NodeToken in_time_delta,
    DscDag::NodeToken in_ui_scale,
    //DscDag::NodeToken in_avaliable_size
    const UiNodeGroup& in_parent,
    UiNodeGroup& in_self
)
{
    DscUi::UiComponentResourceNodeGroup component_resource_group;
    component_resource_group.SetNodeToken(
        DscUi::TUiComponentResourceNodeGroup::TClearColour,
        in_dag_collection.CreateValue(
            in_construction_helper._clear_colour,
            DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
            &component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA "clear colour")));

    if (true == in_construction_helper._has_scroll)
    {
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::THasManualScrollX,
            in_dag_collection.CreateValue(
                in_construction_helper._has_manual_scroll_x,
                DscDag::CallbackOnValueChange<bool>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "has manual scroll x")));

        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TManualScrollX,
            in_dag_collection.CreateValue(
                0.0f,
                DscDag::CallbackOnValueChange<float>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "manual scroll x")));

        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::THasManualScrollY,
            in_dag_collection.CreateValue(
                in_construction_helper._has_manual_scroll_y,
                DscDag::CallbackOnValueChange<bool>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "has manual scroll y")));

        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TManualScrollY,
            in_dag_collection.CreateValue(
                0.0f,
                DscDag::CallbackOnValueChange<float>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "manual scroll y")));
    }

    if (true == in_construction_helper._has_fill)
    {
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TFillColour,
            in_dag_collection.CreateValue(
                in_construction_helper._fill,
                DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "fill colour")));
    }

    if (nullptr != in_construction_helper._texture)
    {
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TTexture,
            in_dag_collection.CreateValue(
                in_construction_helper._texture,
                DscDag::CallbackOnSetValue<std::shared_ptr<DscRenderResource::ShaderResource>>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "texture")));
    }

    if (nullptr != in_construction_helper._text_run)
    {
        DSC_ASSERT(nullptr != in_construction_helper._text_manager, "invalid state");
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TText,
            in_dag_collection.CreateValue(
                DscUi::TUiComponentTextData({ in_construction_helper._text_run , in_construction_helper._text_manager }),
                DscDag::CallbackOnSetValue<DscUi::TUiComponentTextData>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "text")));
    }

    if (true == in_construction_helper._has_ui_scale_by_avaliable_width)
    {
        const int32 scale_width_low_threashhold = in_construction_helper._scale_width_low_threashhold;
        const float scale_factor = in_construction_helper._scale_factor;
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TUiScale,
            in_dag_collection.CreateCalculate<float>([scale_width_low_threashhold, scale_factor](float& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[0]);
            const DscCommon::VectorInt2& avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            if (scale_width_low_threashhold < avaliable_size.GetX())
            {
                ui_scale = (1.0f + (static_cast<float>(avaliable_size.GetX() - scale_width_low_threashhold) * scale_factor));
            }

            value = ui_scale;
        },
                &component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA "ui scale from width")));
        DscDag::DagCollection::LinkIndexNodes(0, in_ui_scale, component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TUiScale));
        //in_parent.GetNodeToken(TUiNodeGroup::TAvaliableSize), // ok, this was ment to be the local avaliable size, but have some dependency order issues here
        DscDag::NodeToken avaliable_size = in_parent.GetNodeToken(TUiNodeGroup::TAvaliableSize);
        DscDag::DagCollection::LinkIndexNodes(1, avaliable_size, component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TUiScale));
    }
    else
    {
        component_resource_group.SetNodeToken(DscUi::TUiComponentResourceNodeGroup::TUiScale, in_ui_scale);
    }

    if (true == in_construction_helper._has_child_slot_data)
    {
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TChildSlotSize,
            in_dag_collection.CreateValue(
                in_construction_helper._child_size,
                DscDag::CallbackOnValueChange<DscUi::VectorUiCoord2>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "child slot size")));
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TChildSlotPivot,
            in_dag_collection.CreateValue(
                in_construction_helper._child_pivot,
                DscDag::CallbackOnValueChange<DscUi::VectorUiCoord2>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "child slot pivot")));
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TChildSlotParentAttach,
            in_dag_collection.CreateValue(
                in_construction_helper._attach_point,
                DscDag::CallbackOnValueChange<DscUi::VectorUiCoord2>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "child slot parent attach")));
    }

    if (true == in_construction_helper._has_desired_size)
    {
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TDesiredSize,
            in_dag_collection.CreateValue(
                in_construction_helper._desired_size,
                DscDag::CallbackOnValueChange<DscUi::VectorUiCoord2>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "desired size")));
    }

    if (true == in_construction_helper._has_padding)
    {
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TPaddingLeft,
            in_dag_collection.CreateValue(
                in_construction_helper._padding_left,
                DscDag::CallbackOnValueChange<DscUi::UiCoord>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "padding left")));
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TPaddingTop,
            in_dag_collection.CreateValue(
                in_construction_helper._padding_top,
                DscDag::CallbackOnValueChange<DscUi::UiCoord>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "padding top")));
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TPaddingRight,
            in_dag_collection.CreateValue(
                in_construction_helper._padding_right,
                DscDag::CallbackOnValueChange<DscUi::UiCoord>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "padding right")));
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TPaddingBottom,
            in_dag_collection.CreateValue(
                in_construction_helper._padding_bottom,
                DscDag::CallbackOnValueChange<DscUi::UiCoord>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "padding bottom")));
    }

    if (true == in_construction_helper._has_gap)
    {
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TGap,
            in_dag_collection.CreateValue(
                in_construction_helper._gap,
                DscDag::CallbackOnValueChange<DscUi::UiCoord>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "gap")));
    }

    if (true == in_construction_helper._has_gradient_fill)
    {
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TGradientFill,
            in_dag_collection.CreateValue(
                in_construction_helper._gradient_fill_constant_buffer,
                DscDag::CallbackOnValueChange<DscUi::TGradientFillConstantBuffer>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "gradient fill")));
    }

    if (true == in_construction_helper._has_multi_gradient_fill)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate<DscUi::TGradientFillConstantBuffer>([](DscUi::TGradientFillConstantBuffer& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                if (nullptr == in_input_array[0])
                {
                    // do we need a warning
                    DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "multi gradent without a parent input state");
                    return;
                }
                const TUiInputStateFlag input_state = DscDag::DagCollection::GetValueType<TUiInputStateFlag>(in_input_array[0]);
                const std::vector<DscUi::TGradientFillConstantBuffer>& array_gradient = DscDag::DagCollection::GetValueType<std::vector<DscUi::TGradientFillConstantBuffer>>(in_input_array[1]);

                const int32 index = static_cast<int32>(input_state);
                DSC_ASSERT((0 <= index) && (index < static_cast<int32>(array_gradient.size())), "invalid state");
                value = array_gradient[index];
            },
            &component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA "gradient fill calculate"));

        const DscUi::UiComponentResourceNodeGroup& parent_resource_group = DscDag::DagCollection::GetValueType<DscUi::UiComponentResourceNodeGroup>(in_parent.GetNodeToken(DscUi::TUiNodeGroup::TUiComponentResources));
        DscDag::DagCollection::LinkIndexNodes(0, parent_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TInputStateFlag), node); // set with the parent TUiInputStateFlag

        DSC_ASSERT(nullptr != in_construction_helper._multi_gradient_fill_node, "invalid param");
        // is there a better way to check type?
        DscDag::DagCollection::GetValueType<std::vector<DscUi::TGradientFillConstantBuffer>>(in_construction_helper._multi_gradient_fill_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_construction_helper._multi_gradient_fill_node, node);

        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TGradientFill,
            node);
    }

    if (DscUi::TUiFlow::TCount != in_construction_helper._flow_direction)
    {
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TFlow,
            in_dag_collection.CreateValue(
                in_construction_helper._flow_direction,
                DscDag::CallbackOnValueChange<DscUi::TUiFlow>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "flow direction")));
    }

    if (true == in_construction_helper._has_child_stack_data)
    {
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TChildStackSize,
            in_dag_collection.CreateValue(
                in_construction_helper._stack_size,
                DscDag::CallbackOnValueChange<DscUi::UiCoord>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "child stack size")));
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TChildStackPivot,
            in_dag_collection.CreateValue(
                in_construction_helper._stack_pivot,
                DscDag::CallbackOnValueChange<DscUi::UiCoord>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "child stack pivot")));
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TChildStackParentAttach,
            in_dag_collection.CreateValue(
                in_construction_helper._stack_parent_attach_point,
                DscDag::CallbackOnValueChange<DscUi::UiCoord>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "child stack parent attach")));
    }

    if (true == in_construction_helper._has_input)
    {
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TInputStateFlag,
            in_dag_collection.CreateValue(
                DscUi::TUiInputStateFlag::TNone,
                DscDag::CallbackOnValueChange<DscUi::TUiInputStateFlag>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "input state flag")));

        if (nullptr != in_construction_helper._input_click_callback)
        {
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TInputData,
                in_dag_collection.CreateValue(
                    DscUi::TUiComponentInputData({ in_construction_helper._input_click_callback}),
                    DscDag::CallbackNever<DscUi::TUiComponentInputData>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "input data")));
        }
    }

    if (true == in_construction_helper._has_input_rollover_accumulate)
    {
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TInputRolloverAccumulate,
            in_dag_collection.CreateValue(
                0.0f,
                DscDag::CallbackOnValueChange<float>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "input rollover accumulate")));
    }

    if (true == in_construction_helper._has_input_active_touch_pos)
    {
        component_resource_group.SetNodeToken(
        DscUi::TUiComponentResourceNodeGroup::TInputActiveTouchPos,
        in_dag_collection.CreateValue(
            DscCommon::VectorFloat2::s_zero,
            // possibly not ideal, dont want render made dirty by mouse movement OUTSIDE a button, but want to react to rollover
            // better would be to unlink depending on a conditional like for rollover accumulate, but being laze and just not updating on change
            //DscDag::CallbackOnValueChange<DscCommon::VectorFloat2>::Function,
            DscDag::CallbackNever<DscCommon::VectorFloat2>::Function,
            &component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA "input rollover accumulate")));

        //  or, we could use the TInputRolloverAccumulate. or both
        // hook up the 
        //const DscUi::UiComponentResourceNodeGroup& parent_resource_group = DscDag::DagCollection::GetValueType<DscUi::UiComponentResourceNodeGroup>(in_parent.GetNodeToken(DscUi::TUiNodeGroup::TUiComponentResources));
        //DscDag::NodeToken parent_input_state = parent_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TInputStateFlag);
        //DSC_ASSERT(nullptr != parent_input_state, "invalid state");
        // can we just link to the parent input state?
        //component_resource_group.SetNodeToken(DscUi::TUiComponentResourceNodeGroup::TInputStateFlag, parent_input_state);
    }

    if (true == in_construction_helper._has_crossfade_child_amount) 
    {
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TCrossfadeChildAmount,
            in_dag_collection.CreateValue(
                in_construction_helper._crossfade_child_amount,
                DscDag::CallbackOnValueChange<float>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "crossfade child amount")));

        in_self.SetNodeToken(
            DscUi::TUiNodeGroup::TUiPanelTint,
            in_dag_collection.CreateCalculate<DscCommon::VectorFloat4>([](DscCommon::VectorFloat4& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                    const float amount = DscDag::DagCollection::GetValueType<float>(in_input_array[0]);
                    value = DscCommon::VectorFloat4(amount, amount, amount, amount);
                },
                & component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "crossfade child panel tint")));

        DscDag::DagCollection::LinkIndexNodes(0,
            component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TCrossfadeChildAmount),
            in_self.GetNodeToken(DscUi::TUiNodeGroup::TUiPanelTint));
    }

    // the crossfade, on a change to TUiRootNodeGroup::TArrayChildUiNodeGroup, how do we set the child crossfade amount
    // we need tick, child array, and the _crossfade_active_child node
    if (nullptr != in_construction_helper._crossfade_active_child)
    {
        //condition node - if the state of the 
        DscDag::NodeToken condition = in_dag_collection.CreateCalculate<bool>([](bool& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const DscDag::NodeToken child_active_draw_node = DscDag::DagCollection::GetValueType<DscDag::NodeToken>(in_input_array[0]);
                const std::vector<UiNodeGroup>& child_array = DscDag::DagCollection::GetValueType<std::vector<UiNodeGroup>>(in_input_array[1]);
            
                bool data_correct = true;
                for (const auto& child : child_array)
                {
                    const bool active = child.GetNodeToken(TUiNodeGroup::TDrawNode) == child_active_draw_node;
                    const UiComponentResourceNodeGroup& child_resource_node_group = DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(child.GetNodeToken(TUiNodeGroup::TUiComponentResources));
                    DscDag::NodeToken child_crossfade_amount_node = child_resource_node_group.GetNodeToken(TUiComponentResourceNodeGroup::TCrossfadeChildAmount);
                    const float cross_fade_amount = DscDag::DagCollection::GetValueType<float>(child_crossfade_amount_node);
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
            & component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA "crossfade condition"));
        DscDag::DagCollection::LinkIndexNodes(0, in_construction_helper._crossfade_active_child, condition);
        DscDag::DagCollection::LinkIndexNodes(1, in_self.GetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup), condition);

        DscDag::NodeToken pass_along_tick = in_dag_collection.CreateValue<float>(
            0.0f,
            DscDag::CallbackNotZero<float>::Function,
            &component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA "pass along tick"));

        in_dag_collection.CreateCondition(
            condition,
            in_time_delta,
            nullptr,
            pass_along_tick,
            nullptr,
            &component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA "crossfade condition node"));

        DscDag::NodeToken crossfade_node = in_dag_collection.CreateCalculate<bool>([](bool& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscDag::NodeToken child_active_draw_node = DscDag::DagCollection::GetValueType<DscDag::NodeToken>(in_input_array[0]);
            const std::vector<UiNodeGroup>& child_array = DscDag::DagCollection::GetValueType<std::vector<UiNodeGroup>>(in_input_array[1]);
            const float time_deta = DscDag::DagCollection::GetValueType<float>(in_input_array[2]);

            for (const auto& child : child_array)
            {
                const bool active = child.GetNodeToken(TUiNodeGroup::TDrawNode) == child_active_draw_node;
                const UiComponentResourceNodeGroup& child_resource_node_group = DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(child.GetNodeToken(TUiNodeGroup::TUiComponentResources));
                DscDag::NodeToken child_crossfade_amount_node = child_resource_node_group.GetNodeToken(TUiComponentResourceNodeGroup::TCrossfadeChildAmount);
                float cross_fade_amount = DscDag::DagCollection::GetValueType<float>(child_crossfade_amount_node);
                cross_fade_amount += (active ? time_deta : -time_deta);
                cross_fade_amount = std::clamp(cross_fade_amount, 0.0f, 1.0f);
                DscDag::DagCollection::SetValueType<float>(child_crossfade_amount_node, cross_fade_amount);
            }

            value = true;
        },
            & component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA "crossfade calculate child fade amount"));

        DscDag::DagCollection::LinkIndexNodes(0, in_construction_helper._crossfade_active_child, crossfade_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_self.GetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup), crossfade_node);
        DscDag::DagCollection::LinkIndexNodes(2, pass_along_tick, crossfade_node);

        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TCrossfadeNode,
            crossfade_node
            );

        //CAN not do this yet, TDrawBaseNode not set
        // set the cross fade as a dependency of the cross fade base draw. 
        // Warning, this will dirty the CrossFade EVERY time tick is set. how can we do better? some sort of condition looking at if the cross fade needs to be updated, which hooks up the time delta?
        //DscDag::DagCollection::LinkNodes(crossfade_node, in_self.GetNodeToken(TUiNodeGroup::TDrawBaseNode));

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

