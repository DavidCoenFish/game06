#include "component_construction_helper.h"

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
