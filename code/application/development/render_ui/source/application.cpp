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
#include <dsc_data/dsc_data.h>
#include <dsc_data/accessor.h>
#include <dsc_data/json.h>
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
#include <dsc_text/text_manager.h>
#include <dsc_text/text.h>
#include <dsc_text/text_run_text.h>
#include <dsc_text/glyph_collection_text.h>
#include <dsc_windows/window_helper.h>

namespace
{
    void AddListItem(
        DscDag::NodeToken in_selected_index_node,
        const int32 in_index,
        const std::string& in_message,
        DscUi::UiManager& in_ui_manager,
        DscRender::DrawSystem& in_draw_system,
        DscDag::DagCollection& in_dag_collection,
        DscCommon::FileSystem& in_file_system,
        DscText::TextManager& in_text_manager,
        DscDag::NodeToken in_root_node,
        DscDag::NodeToken in_parent_node
        )
    {
        auto item_node_group = in_ui_manager.AddChildNode(
            DscUi::MakeComponentCanvas(
            ).SetInputData(
                [in_index, in_selected_index_node]
                (DscDag::NodeToken, const DscCommon::VectorFloat2&) {
                    DscDag::SetValueType(in_selected_index_node, in_index);
                    return;
                },
				nullptr,
                true,
                true
            ).SetHasItemIndex(
                in_index
            ).SetChildStackData(
                DscUi::UiCoord(0, 1.0f),
                DscUi::UiCoord(0, 0.5f),
                DscUi::UiCoord(0, 0.5f)
            ).SetDesiredSize(
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(48, 0.0f))
            ).SetClearColour(
                DscCommon::VectorFloat4(1.0f, 0.0f, 0.0f, 1.0f)
            ),
            in_draw_system,
            in_dag_collection,
            in_root_node,
            in_parent_node,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "item")
        );

        DscDag::NodeToken multi_gradient = in_dag_collection.FetchNodeName("item_multi_gradient");
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
                {
                    {0.0f, 0.1f, 0.5f, 1.0f},
                    {0.0f, 0.314f, 0.208f, 1.0f},
                    {0.0f, 0.439f, 0.286f, 1.0f},
                    {0.0f, 0.627f, 0.396f, 1.0f},
                    {0.0f, 0.757f, 0.525f, 1.0f}
                }
            );
            multi_gradient_data.push_back(
                //in_gradient_fill_constant_buffer_rollover_selection,
                {
                    {0.0f, 0.5f, 0.9f, 1.0f},
                    {0.0f, 0.314f, 0.208f, 1.0f},
                    {0.0f, 0.439f, 0.286f, 1.0f},
                    {0.0f, 0.627f, 0.396f, 1.0f},
                    {0.0f, 0.757f, 0.525f, 1.0f}
                }
            );
            multi_gradient_data.push_back(
                //in_gradient_fill_constant_buffer_click_selection,
                {
                    {0.0f, 0.75f, 1.0f, 1.1f},
                    {0.0f, 0.529f, 0.361f, 1.0f},
                    {0.0f, 0.306f, 0.196f, 1.0f},
                    {0.0f, 0.098f, 0.051f, 1.0f},
                    {0.0f, 0.063f, 0.055f, 1.0f}
                }
            );
            multi_gradient_data.push_back(
                //in_gradient_fill_constant_buffer_rollover_click_selection
                {
                    {0.0f, 0.9f, 1.5f, 1.6f},
                    {0.0f, 0.529f, 0.361f, 1.0f},
                    {0.0f, 0.306f, 0.196f, 1.0f},
                    {0.0f, 0.098f, 0.051f, 1.0f},
                    {0.0f, 0.063f, 0.055f, 1.0f}
                }
            );

            multi_gradient = in_dag_collection.CreateValueNone(multi_gradient_data);
            DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(multi_gradient, "item multi gadient data"));
            in_dag_collection.AddNodeName(multi_gradient, "item_multi_gradient");
        }

        in_ui_manager.AddChildNode(
            DscUi::MakeComponentMultiGradientFill(
                multi_gradient
            ),
            in_draw_system,
            in_dag_collection,
            in_root_node,
            item_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "item multi fill")
        );


        auto text = in_dag_collection.CreateCalculate<std::shared_ptr<DscText::Text>>(
            [&in_text_manager, &in_file_system, in_message]
        (std::shared_ptr<DscText::Text>& output, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>&) {
            const DscText::TextLocale* const pLocale = in_text_manager.GetLocaleToken(DscLocale::LocaleISO_639_1::English);
            DscText::GlyphCollectionText* font = in_text_manager.LoadFont(
                in_file_system, 
                DscCommon::FileSystem::JoinPath("data", "font", "code2000.ttf")
            );

            std::vector<std::unique_ptr<DscText::ITextRun>> text_run_array;
            DscCommon::VectorInt2 container_size = {};

            text_run_array.push_back(DscText::Text::MakeTextRunDataString(
                in_message,
                pLocale,
                font,
                20,
                DscCommon::Math::ConvertColourToInt(111, 28, 11, 255)//,
                //20,
                //10
                , 0, 0, 15
            ));

            const int32 current_width = 0;
            output = std::make_shared<DscText::Text>(
                std::move(text_run_array),
                container_size,
                true,
                current_width,
                DscText::THorizontalAlignment::TMiddle,
                DscText::TVerticalAlignment::TMiddle
                );

        }, dynamic_cast<DscDag::IDagOwner*>(item_node_group)
            );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(text, "text node"));

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
            item_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "item text")
        );

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
    _draw_system = DscRender::DrawSystem::FactoryClearColour(in_hwnd, DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 1.0f));

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
        _resources->_ui_manager = std::make_unique<DscUi::UiManager>(
			*_draw_system, 
			*_file_system, 
			*(_resources->_dag_collection),
			std::vector<DscUi::UiManager::TEffectConstructionHelper>({
				{
					DscUi::TUiEffectType::TEffectInnerShadow,
					DscCommon::VectorFloat4(0.0f, 0.0f, 4.0f, 0.0f),
					DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 1.0f)
				}
				})
			);
    }

    if (nullptr != _resources->_ui_manager)
    {
        _resources->_ui_root_node_group = _resources->_ui_manager->MakeRootNode(
            DscUi::MakeComponentCanvas().SetClearColour(DscCommon::VectorFloat4::s_zero),
            *_draw_system,
            *_resources->_dag_collection
        );

        _resources->_selected_index_node = _resources->_dag_collection->CreateValueOnValueChange<int32>(0);

        auto stack_selector_node_group = _resources->_ui_manager->AddChildNode(
            DscUi::MakeComponentStack(
                DscUi::TUiFlow::TVertical,
                DscUi::UiCoord(8, 0.0f)
			).SetManualScrollY(
				true
            ).SetClearColour(
                DscCommon::VectorFloat4(1.0f, 0.0f, 0.0f, 1.0f)
            ).SetChildSlot(
                DscUi::VectorUiCoord2(DscUi::UiCoord(300, 0.0f), DscUi::UiCoord(600, 0.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f))
            ).SetHasSelectedChild(
                0,
                _resources->_selected_index_node
            ),
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            _resources->_ui_root_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "stack")
        );
#if 0
		auto scroll_owner = dynamic_cast<DscDag::IDagOwner*>(stack_selector_node_group);
		DscDag::NodeToken scrollbar_write_y = _resources->_dag_collection->CreateValueOnValueChange<float>(
			0.5f,
			scroll_owner
			);
		DscDag::NodeToken fake_geometry_size = _resources->_dag_collection->CreateValueOnValueChange(
			DscCommon::VectorInt2(200, 300),
			scroll_owner
			);
		DscDag::NodeToken fake_render_request_size = _resources->_dag_collection->CreateValueOnValueChange(
			DscCommon::VectorInt2(200, 900),
			scroll_owner
			);
		DscDag::NodeToken scrollbar_range_read_y = _resources->_dag_collection->CreateCalculate<DscCommon::VectorFloat4>([]
    (DscCommon::VectorFloat4& output, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
        const DscCommon::VectorInt2& geometry_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
        const DscCommon::VectorInt2& render_request_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
        //const DscCommon::VectorFloat2& scroll_pos = DscDag::GetValueType<DscCommon::VectorFloat2>(in_input_array[2]);
        const float scroll_pos = DscDag::GetValueType<float>(in_input_array[2]);
        //const float scroll_x = std::min(1.0f, std::max(0.0f, std::abs(scroll_pos.GetX())));
        const float scroll_y = std::min(1.0f, std::max(0.0f, std::abs(scroll_pos)));
		const int32 overhang = std::max(0, render_request_size.GetY() - geometry_size.GetY());
		const float low = static_cast<float>(overhang) * scroll_y;
		output.Set(
			low,
			low + static_cast<float>(geometry_size.GetY()),
			static_cast<float>(render_request_size.GetY()),
			0.0f
			);
    }, dynamic_cast<DscDag::IDagOwner*>(stack_selector_node_group)
        );
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(scrollbar_range_read_y, "node to scroll data convertor"));

    DscDag::LinkIndexNodes(0, fake_geometry_size, scrollbar_range_read_y);
    DscDag::LinkIndexNodes(1, fake_render_request_size, scrollbar_range_read_y);
    DscDag::LinkIndexNodes(2, scrollbar_write_y, scrollbar_range_read_y);

		// scroll_bar_node
        _resources->_ui_manager->AddChildNode(
            DscUi::MakeComponentScrollbarY(
				DscCommon::VectorFloat4(1.0f, 0.0f, 0.0f, 1.0f),
                DscUi::UiCoord(8, 0.0f),
				scrollbar_write_y,
				scrollbar_range_read_y
			).SetClearColour(
                DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.0f)
            ).SetChildSlot(
                DscUi::VectorUiCoord2(DscUi::UiCoord(16, 0.0f), DscUi::UiCoord(400, 0.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.5f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.5f))
            ).SetInputData(
                [scrollbar_write_y]
                (DscDag::NodeToken in_node, const DscCommon::VectorFloat2& in_node_rel_click) {
					const DscUi::ScreenSpace& screen_space = DscDag::GetValueType<DscUi::ScreenSpace>(
						DscDag::DagNodeGroup::GetNodeTokenEnum(in_node, DscUi::TUiNodeGroup::TScreenSpace)
						);
					const float length = screen_space._screen_space.GetW() - screen_space._screen_space.GetY();
					const float ratio = (0.0f != length) ? DscCommon::Math::Clamp(in_node_rel_click.GetY() / length, 0.0f, 1.0f) : 0.0f;
					DscDag::SetValueType(scrollbar_write_y, ratio);
                    return;
                },
                [scrollbar_write_y]
                (DscDag::NodeToken in_node, const DscCommon::VectorFloat2&, const DscCommon::VectorFloat2& in_node_rel_click) {
					const DscUi::ScreenSpace& screen_space = DscDag::GetValueType<DscUi::ScreenSpace>(
						DscDag::DagNodeGroup::GetNodeTokenEnum(in_node, DscUi::TUiNodeGroup::TScreenSpace)
						);
					const float length = screen_space._screen_space.GetW() - screen_space._screen_space.GetY();
					const float ratio = (0.0f != length) ? DscCommon::Math::Clamp(in_node_rel_click.GetY() / length, 0.0f, 1.0f) : 0.0f;
					DscDag::SetValueType(scrollbar_write_y, ratio);
                    return;
                },
                true,
                true
            ),
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            _resources->_ui_root_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "scrollbar test")
        );
#endif
        for (int32 index = 0; index < 20; ++index)
        {
            AddListItem(
                _resources->_selected_index_node,
                index,
                std::to_string(index),
                *_resources->_ui_manager,
                *_draw_system,
                *_resources->_dag_collection,
                *_file_system,
                *_resources->_text_manager,
                _resources->_ui_root_node_group,
                stack_selector_node_group
            );
        }

    }

    return;
}

Application::~Application()
{
    if (_draw_system)
    {
        _draw_system->WaitForGpu();
    }

    _resources->_ui_manager->DestroyNode(
        *(_resources->_dag_collection),
        _resources->_ui_root_node_group
    );
    _resources->_ui_root_node_group = nullptr;

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

        //DSC_LOG_DIAGNOSTIC(LOG_TOPIC_APPLICATION, "Update time_delta:%f\n", time_delta);

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

        DscUi::UiRenderTarget* ui_texture = nullptr;
        if (_resources->_ui_manager)
        {
            _resources->_ui_manager->Update(
                _resources->_ui_root_node_group,
                time_delta,
                input_param,
                _draw_system->GetRenderTargetBackBuffer()->GetViewportSize()
            );

            ui_texture = _resources->_ui_manager->Draw(
                _resources->_ui_root_node_group,
                *_resources->_dag_collection,
                *frame
            );
        }

        frame->SetRenderTarget(_draw_system->GetRenderTargetBackBuffer());

        if (nullptr != ui_texture)
        {
            _resources->_ui_manager->DrawUiTextureToCurrentRenderTarget(
                *frame,
                _resources->_ui_root_node_group,
                ui_texture
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


