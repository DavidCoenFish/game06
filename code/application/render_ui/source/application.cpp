#include "application.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_common/math.h>
#include <dsc_common/i_file_overlay.h>
#include <dsc_common/timer.h>
#include <dsc_common/vector_int2.h>
#include <dsc_dag/dag_collection.h>
#include <dsc_dag/i_dag_owner.h>
#include <dsc_dag/debug_print.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render_resource/frame.h>
#include <dsc_render_resource/shader.h>
#include <dsc_render_resource/shader_constant_buffer.h>
#include <dsc_render_resource/shader_resource.h>
#include <dsc_render_resource_png/dsc_render_resource_png.h>
#include <dsc_text/text_manager.h>
#include <dsc_onscreen_version/onscreen_version.h>
#include <dsc_ui/i_ui_instance.h>
#include <dsc_ui/ui_enum.h>
#include <dsc_ui/component_construction_helper.h>
#include <dsc_ui/ui_manager.h>
#include <dsc_ui/ui_render_target.h>
#include <dsc_ui/ui_input_param.h>
#include <dsc_ui/ui_instance_factory.h>
#include <dsc_locale/dsc_locale.h>
#include <dsc_png/dsc_png.h>
#include <dsc_text/text_manager.h>
#include <dsc_text/text_run.h>
#include <dsc_text/text_run_text.h>
#include <dsc_text/glyph_collection_text.h>
#include <dsc_windows/window_helper.h>

namespace
{
    enum class TUiNodeGroupDataSourceApplication : uint8
    {
        TMainScreenDataSource = DscUi::TUiNodeGroupDataSource::TCount,
        TCount
    };

    class UiInstanceApp : public DscUi::IUiInstance
    {
    public:
        UiInstanceApp() = delete;
        UiInstanceApp& operator=(const UiInstanceApp&) = delete;
        UiInstanceApp(const UiInstanceApp&) = delete;

        UiInstanceApp(
            const std::shared_ptr<DscUi::UiRenderTarget>& in_root_external_render_target_or_null,
            DscUi::UiManager& in_ui_manager,
            DscRender::DrawSystem& in_draw_system,
            DscDag::DagCollection& in_dag_collection
            )
            : _ui_manager(in_ui_manager)
            , _draw_system(in_draw_system)
            , _dag_collection(in_dag_collection)
        {
            _root_node_group = _ui_manager.MakeRootNode(
                DscUi::MakeComponentCanvas(
                ).SetClearColour(
                    DscCommon::VectorFloat4(1.0f, 0.0f, 0.0f, 1.0f)
                ),
                _draw_system,
                _dag_collection,
                in_root_external_render_target_or_null
            );

            _main_screen_cross_fade = _ui_manager.AddChildNode(
                DscUi::MakeComponentCrossfade(
                    nullptr
                ).SetChildSlot(
                    DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
                    DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
                    DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f))
                ),
                _draw_system,
                _dag_collection,
                _root_node_group,
                _root_node_group,
                std::vector<DscUi::UiManager::TEffectConstructionHelper>()
                DSC_DEBUG_ONLY(DSC_COMMA "app crossfade main")
            );
        }

        ~UiInstanceApp()
        {
            DscDag::DebugPrintRecurseInputs(_root_node_group);
            DscDag::DebugPrintRecurseOutputs(_root_node_group);

            _ui_manager.DestroyNode(
                _dag_collection,
                _root_node_group
                );
        }
    private:
        virtual void Update() override
        {
            //todo
        }
        virtual DscDag::NodeToken GetDagUiGroupNode() override
        {
            return _root_node_group;
        }
        virtual DscDag::NodeToken GetDagUiDrawNode() override
        {
            return DscDag::DagNodeGroup::GetNodeTokenEnum(_root_node_group, DscUi::TUiNodeGroup::TDrawNode);
        }
        virtual DscDag::NodeToken GetDagUiDrawBaseNode() override
        {
            return DscDag::DagNodeGroup::GetNodeTokenEnum(_root_node_group, DscUi::TUiNodeGroup::TDrawBaseNode);
        }
    private:
        DscUi::UiManager& _ui_manager;
        DscRender::DrawSystem& _draw_system;
        DscDag::DagCollection& _dag_collection;

        DscDag::NodeToken _root_node_group = {};
        DscDag::NodeToken _main_screen_cross_fade = {};
    };

    std::shared_ptr<DscUi::IUiInstance> UiInstanceFactoryApp(
        DscUi::UiInstanceFactory& in_ui_instance_factory,
        DscUi::UiManager& in_ui_manager,
        DscRender::DrawSystem& in_draw_system,
        DscDag::DagCollection& in_dag_collection,
        const std::shared_ptr<DscUi::UiRenderTarget>& in_root_external_render_target_or_null,
        DscDag::NodeToken in_data_source, // data source
        DscDag::NodeToken in_parent_node_or_null// parent node or null
    )
    {
        DSC_UNUSED(in_ui_instance_factory);
        DSC_UNUSED(in_data_source);
        DSC_UNUSED(in_parent_node_or_null);


        //_calculate_main_screen = in_ui_instance_factory

        std::shared_ptr<DscUi::IUiInstance> result = std::make_shared<UiInstanceApp>(
            in_root_external_render_target_or_null,
            in_ui_manager,
            in_draw_system,
            in_dag_collection
            );

        return result;
    }
}

Application::Resources::Resources()
{
    //nop
}

Application::Application(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight)
    : DscWindows::IWindowApplication(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight)
{
    _keep_running = true;

    _file_system = std::make_unique<DscCommon::FileSystem>();
    _draw_system = DscRender::DrawSystem::FactoryClearColour(in_hwnd, DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 0.0f));

    _resources = std::make_unique<Resources>();

    if (nullptr != _resources)
    {
        _resources->_timer = std::make_unique<DscCommon::Timer>();
    }

    if ((nullptr != _file_system) && (nullptr != _draw_system))
    {
        _resources->_text_manager = std::make_unique<DscText::TextManager>(*_draw_system, *_file_system);
        _resources->_onscreen_version = std::make_unique<DscOnscreenVersion::OnscreenVersion>(*_draw_system, *_file_system, *(_resources->_text_manager));
        _resources->_dag_collection = std::make_unique<DscDag::DagCollection>();
        _resources->_ui_manager = std::make_unique<DscUi::UiManager>(*_draw_system, *_file_system, *(_resources->_dag_collection));
        _resources->_ui_instance_factory = std::make_unique<DscUi::UiInstanceFactory>();
    }

    if (nullptr != _resources->_ui_instance_factory)
    {
        _resources->_data_source_node_group = _resources->_dag_collection->CreateGroupEnum<TUiNodeGroupDataSourceApplication, DscUi::TUiNodeGroupDataSource>();
        DscDag::IDagOwner* const data_source_owner = dynamic_cast<DscDag::IDagOwner*>(_resources->_data_source_node_group);
        {
            auto node = _resources->_dag_collection->CreateValueOnValueChange<std::string>("app", data_source_owner);
            DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "template name"));
            DscDag::DagNodeGroup::SetNodeTokenEnum(
                _resources->_data_source_node_group, 
                DscUi::TUiNodeGroupDataSource::TTemplateName,
                node
                );
        }

        _resources->_ui_instance_factory->AddFactory("app", UiInstanceFactoryApp);

        auto ui_render_target = _resources->_ui_manager->MakeUiRenderTarget(_draw_system->GetRenderTargetBackBuffer(), true);
        _resources->_ui_instance_node =
            _resources->_ui_instance_factory->BuildInstance(
            _resources->_data_source_node_group,
            *_resources->_ui_manager,
            *_draw_system,
            *_resources->_dag_collection,
            ui_render_target,
            nullptr
            );
    }

    return;
}

Application::~Application()
{
    if (_draw_system)
    {
        _draw_system->WaitForGpu();
    }

    _resources->_dag_collection->DeleteNode(_resources->_ui_instance_node);
    _resources->_dag_collection->DeleteNode(_resources->_data_source_node_group);

    _resources.reset();
    _draw_system.reset();
    _file_system.reset();
}

const bool Application::Update()
{
    BaseType::Update();
    if (_draw_system && _resources && (false == GetMinimized()))
    {
        std::unique_ptr<DscRenderResource::Frame> frame = DscRenderResource::Frame::CreateNewFrame(*_draw_system);

        float time_delta = 0.0f;
        if (_resources && _resources->_timer)
        {
            time_delta = _resources->_timer->GetDeltaSeconds();
        }

        DscUi::UiInputParam input_param = {};
        {
            DscCommon::VectorInt2 pos = {};
            bool left_button = false;
            bool right_button = false;

            DscWindows::GetMouseState(
                GetHwnd(),
                pos,
                left_button,
                right_button
            );
            input_param.SetMouseTouch(
                pos,
                left_button,
                right_button
                );
        }

        if (_resources->_ui_manager)
        {
            auto ui_instance = DscDag::GetValueType<std::shared_ptr<DscUi::IUiInstance>>(_resources->_ui_instance_node);

            ui_instance->Update();

            _resources->_ui_manager->Update(
                ui_instance->GetDagUiGroupNode(),
                time_delta,
                input_param,
                _draw_system->GetRenderTargetBackBuffer()
            );

            _resources->_ui_manager->Draw(
                ui_instance->GetDagUiGroupNode(),
                *_resources->_dag_collection,
                *frame,
                true,
                _draw_system->GetRenderTargetBackBuffer()
            );
        }

        if (_resources->_onscreen_version)
        {
            _resources->_onscreen_version->Update(*_draw_system, *frame, *_resources->_text_manager);
        }

#if defined(_DEBUG)
        //DscDag::DagCollection::DebugDumpNode(_resources->_ui_root_node_group.GetNodeToken(DscUi::TUiRootNodeGroup::TDrawNode));
#endif //#if defined(_DEBUG)

    }

    return _keep_running;
}

void Application::OnWindowSizeChanged(const DscCommon::VectorInt2& in_size, const float in_monitor_scale)
{
    BaseType::OnWindowSizeChanged(in_size, in_monitor_scale);

    DSC_LOG_DIAGNOSTIC(LOG_TOPIC_APPLICATION, "OnWindowSizeChanged size:%d %d scale:%f\n", in_size.GetX(), in_size.GetY(), in_monitor_scale);

    if (_draw_system)
    {
        _draw_system->OnResize();
    }

    return;
}


