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
    DscUi::UiManager* _ui_manager = {};
    DscRender::DrawSystem* _draw_system = {};
    DscDag::DagCollection* _dag_collection = {};
    DscCommon::FileSystem* _file_system = {};
    std::shared_ptr<DscUi::UiRenderTarget> _root_external_render_target_or_null = {};
    DscDag::NodeToken _data_source = {};
    DscDag::NodeToken _root_node_or_null = {};
    DscDag::NodeToken _parent_node_or_null = {};

};
