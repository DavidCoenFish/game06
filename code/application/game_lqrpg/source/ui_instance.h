#pragma once
#include "game_lqrpg.h"
#include <dsc_ui\i_ui_instance.h>
#include <dsc_ui\ui_instance_factory.h>

namespace DscCommon
{
    class FileSystem;
}

namespace DscDag
{
    class DagCollection;
    class IDagNode;
    typedef IDagNode* NodeToken;
}

namespace DscData
{
    struct JsonValue;
}

namespace DscRender
{
    class DagCollection;
}

namespace DscRender
{
    class DrawSystem;
}

namespace DscText
{
    class TextManager;
}

namespace UiInstance
{
    struct TButtonData
    {
        DscDag::NodeToken _button_text = {};
        std::function<void(DscDag::NodeToken)> _on_click = {};
    };

    DscDag::NodeToken MakeLocaleKey(
        DscDag::DagCollection& in_dag_collection,
        DscDag::IDagOwner* const in_data_source_owner,
        DscDag::NodeToken in_root_data_source_node,
        const std::string& in_locale_key
    );

    void AddButton(
        const TButtonData& in_data,
        DscUi::UiManager& in_ui_manager,
        DscRender::DrawSystem& in_draw_system,
        DscDag::DagCollection& in_dag_collection,
        DscCommon::FileSystem& in_file_system,
        DscText::TextManager& in_text_manager,
        DscDag::NodeToken in_root_node,
        DscDag::NodeToken in_parent_node,
        DscDag::NodeToken in_root_data_source_node_group
    );
}


