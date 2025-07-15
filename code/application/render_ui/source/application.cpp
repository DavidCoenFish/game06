#include "application.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_common/math.h>
#include <dsc_common/i_file_overlay.h>
#include <dsc_common/timer.h>
#include <dsc_common/vector_int2.h>
#include <dsc_dag/dag_collection.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render_resource/frame.h>
#include <dsc_render_resource/shader.h>
#include <dsc_render_resource/shader_constant_buffer.h>
#include <dsc_render_resource/shader_resource.h>
#include <dsc_text/text_manager.h>
#include <dsc_onscreen_version/onscreen_version.h>
#include <dsc_ui/ui_enum.h>
#include <dsc_ui/component_construction_helper.h>
#include <dsc_ui/ui_manager.h>
#include <dsc_ui/ui_render_target.h>
#include <dsc_ui/ui_input_param.h>
#include <dsc_locale/dsc_locale.h>
#include <dsc_png/dsc_png.h>
#include <dsc_text/text_manager.h>
#include <dsc_text/text_run.h>
#include <dsc_text/text_run_text.h>
#include <dsc_text/glyph_collection_text.h>
#include <dsc_windows/window_helper.h>

namespace
{
#if 1
    std::shared_ptr<DscText::TextRun> MakeTextRun(
        DscText::TextManager& in_text_manager, 
        DscCommon::FileSystem& in_file_system, 
        const std::string& in_message)
    {
        DscText::GlyphCollectionText* font = in_text_manager.LoadFont(in_file_system, DscCommon::FileSystem::JoinPath("data", "font", "code2000.ttf"));

        std::vector<std::unique_ptr<DscText::ITextRun>> text_run_array;
        DscCommon::VectorInt2 container_size = {};
        const DscText::TextLocale* const pLocale = in_text_manager.GetLocaleToken(DscLocale::LocaleISO_639_1::English);

        text_run_array.push_back(DscText::TextRun::MakeTextRunDataString(
            in_message,
            pLocale,
            font,
            16,
            DscCommon::Math::ConvertColourToInt(255, 255, 255, 255),
            8,
            8,
            -4
        ));

        const int32 current_width = 0;
        auto text_run = std::make_shared<DscText::TextRun>(
            std::move(text_run_array),
            container_size,
            true,
            current_width,
            DscText::THorizontalAlignment::TMiddle,
            DscText::TVerticalAlignment::TTop
            );
        return text_run;
    }

    void AddButton(
        DscUi::UiManager& in_ui_manager,
        DscText::TextManager& in_text_manager,
        DscCommon::FileSystem& in_file_system,
        DscRender::DrawSystem& in_draw_system,
        DscDag::DagCollection& in_dag_collection,
        DscUi::UiRootNodeGroup& in_ui_root_node_group,
        DscUi::UiNodeGroup& in_parent_node_group,
        const std::string& in_message
    )
    {
        DSC_UNUSED(in_text_manager);
        DSC_UNUSED(in_file_system);
        DSC_UNUSED(in_message);

        const DscUi::UiCoord padding_ammount_top(0, 0.0f);
        const DscUi::UiCoord padding_ammount(16, 0.0f);

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
            DscUi::MakeComponentCanvas().SetInputData(
                nullptr,
                true
            ).SetDesiredSize(
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(48, 0.0f))
            //).SetClearColour(
            //    DscCommon::VectorFloat4(1.0f, 0.0f, 0.0f, 1.0f)
            ),
            in_draw_system,
            in_dag_collection,
            in_ui_root_node_group,
            in_parent_node_group,
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

            multi_gradient = in_dag_collection.CreateValue(
                multi_gradient_data,
                DscDag::CallbackNever<std::vector<DscUi::TGradientFillConstantBuffer>>::Function,
                nullptr
                DSC_DEBUG_ONLY(DSC_COMMA "multi gadient data"));
            in_dag_collection.AddNodeName(multi_gradient, "multi_gradient");
        }

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
            in_ui_root_node_group,
            button_node_group,
            array_fill_effect
            DSC_DEBUG_ONLY(DSC_COMMA "button multi fill")
        );

        std::vector<DscUi::UiManager::TEffectConstructionHelper> array_text_effect = {};
        array_text_effect.push_back({
            DscUi::TUiEffectType::TEffectDropShadow,
            DscCommon::VectorFloat4(0.0f, 0.0f, 4.0f, 0.0f),
            DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 1.25f)
            });
        in_ui_manager.AddChildNode(
            DscUi::MakeComponentText(
                MakeTextRun(in_text_manager, in_file_system, in_message),
                &in_text_manager
            ).SetChildSlot(
            //).SetClearColour(
            //    DscCommon::VectorFloat4(0.0f, 0.0f, 1.0f, 1.0f)
            ),
            in_draw_system,
            in_dag_collection,
            in_ui_root_node_group,
            button_node_group,
            array_text_effect
            DSC_DEBUG_ONLY(DSC_COMMA "button text")
            );
    }
#endif
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
    }

    {
        auto top_texture = _resources->_ui_manager->MakeUiRenderTarget(_draw_system->GetRenderTargetBackBuffer(), true);
        _resources->_ui_root_node_group = _resources->_ui_manager->MakeRootNode(
            DscUi::MakeComponentCanvas().SetClearColour(DscCommon::VectorFloat4::s_zero),
            *_draw_system,
            *_resources->_dag_collection,
            top_texture
        );

        auto root_as_parent = DscUi::UiManager::ConvertRootNodeGroupToNodeGroup(*_resources->_dag_collection, _resources->_ui_root_node_group);

        _resources->_ui_manager->AddChildNode(
            DscUi::MakeComponentDebugGrid().SetChildSlot(
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f))
            ),
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            root_as_parent,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "child one")
        );

        auto stack_node = _resources->_ui_manager->AddChildNode(
            DscUi::MakeComponentStack(
                DscUi::TUiFlow::TVertical,
                DscUi::UiCoord(0, 0.0f)
                //).SetClearColour(
                //    DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.5f)
                ).SetChildSlot(
                    DscUi::VectorUiCoord2(DscUi::UiCoord(256, 0.0f), DscUi::UiCoord(0, 0.5f)),
                    DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
                    DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f))
                ),
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            root_as_parent,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "child two")
        );

        AddButton(
            *_resources->_ui_manager,
            *_resources->_text_manager,
            *_file_system,
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            stack_node,
            "Button Zero"
        );

        AddButton(
            *_resources->_ui_manager,
            *_resources->_text_manager,
            *_file_system,
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            stack_node,
            "Button One"
        );

        AddButton(
            *_resources->_ui_manager,
            *_resources->_text_manager,
            *_file_system,
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            stack_node,
            "Button Two"
        );

        AddButton(
            *_resources->_ui_manager,
            *_resources->_text_manager,
            *_file_system,
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            stack_node,
            "Button Three"
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
            _resources->_ui_manager->Update(
                _resources->_ui_root_node_group,
                time_delta,
                input_param,
                _draw_system->GetRenderTargetBackBuffer()
            );

            _resources->_ui_manager->Draw(
                _resources->_ui_root_node_group,
                *_resources->_dag_collection,
                *frame,
                true,
                _draw_system->GetRenderTargetBackBuffer()
            );
        }
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


