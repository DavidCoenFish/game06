#pragma once
#include "render_ui.h"

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

namespace DscUi
{
    template <typename CONTEXT>
    class UiInstanceFactory;
    class UiRenderTarget;
    class UiManager;
}

struct UiInstanceContext
{
public:
    UiInstanceContext MakeChild(DscDag::NodeToken in_data_source, DscDag::NodeToken in_parent) const;

public:
    DscUi::UiManager* _ui_manager = {};
    DscRender::DrawSystem* _draw_system = {};
    DscDag::DagCollection* _dag_collection = {};
    DscCommon::FileSystem* _file_system = {};
    DscText::TextManager* _text_manager = {};
    std::shared_ptr<DscUi::UiRenderTarget> _root_external_render_target_or_null = {};
    DscDag::NodeToken _data_source_node = {}; // a dag node <node token> with the value of the data source DagNodeGroup 
    DscDag::NodeToken _root_node_or_null = {};
    DscDag::NodeToken _parent_node_or_null = {};

};
