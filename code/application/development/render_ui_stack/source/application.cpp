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
    std::shared_ptr<DscText::TextRun> MakeTextRun(DscText::TextManager& in_text_manager, DscCommon::FileSystem& in_file_system, const std::string& in_message)
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
            32,
            16
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

        _resources->_ui_manager->AddChildNode(
            DscUi::MakeComponentDebugGrid().SetChildSlot(
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 1.0f), DscUi::UiCoord(0, 1.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f)),
                DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.0f), DscUi::UiCoord(0, 0.0f))
            ),
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            _resources->_ui_root_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "child one")
        );

        auto stack_node = _resources->_ui_manager->AddChildNode(
            DscUi::MakeComponentStack(
                DscUi::TUiFlow::TVertical,
                DscUi::UiCoord(16, 0.0f)
                ).SetClearColour(
                    DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.5f)
                ).SetChildSlot(
                    DscUi::VectorUiCoord2(DscUi::UiCoord(256, 0.0f), DscUi::UiCoord(512, 0.0f)),
                    DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
                    DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f))
                ),
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            _resources->_ui_root_node_group,
            std::vector<DscUi::UiManager::TEffectConstructionHelper>()
            DSC_DEBUG_ONLY(DSC_COMMA "child two")
        );

        std::vector<DscUi::UiManager::TEffectConstructionHelper> effect_array;
        effect_array.push_back({ 
            DscUi::TUiEffectType::TEffectTint, 
            DscCommon::VectorFloat4::s_zero, 
            DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.5f) 
            });

        for (int32 index = 0; index < 8; ++index)
        {
            _resources->_ui_manager->AddChildNode(
                DscUi::MakeComponentText(
                    MakeTextRun(*_resources->_text_manager, *_file_system, std::string("hello world ") + std::to_string(index)),
                    _resources->_text_manager.get()
                ).SetClearColour(
                    DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 1.0f)
                ),
                *_draw_system,
                *_resources->_dag_collection,
                _resources->_ui_root_node_group,
                stack_node,
                effect_array
                DSC_DEBUG_ONLY(DSC_COMMA "stack child")
            );
        }

        _resources->_ui_manager->AddChildNode(
            DscUi::MakeComponentText(
                MakeTextRun(*_resources->_text_manager, *_file_system, "Exit"),
                _resources->_text_manager.get()
            ).SetClearColour(
                DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 1.0f)
            ).SetInputData([this](const DscDag::NodeToken) {
                this->_keep_running = false;
                }
            ),
            *_draw_system,
            *_resources->_dag_collection,
            _resources->_ui_root_node_group,
            stack_node,
            effect_array
            DSC_DEBUG_ONLY(DSC_COMMA "stack child")
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

    //DSC_DEBUG_ONLY(DscDag::DebugPrintRecurseInputs(_resources->_ui_root_node_group));

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
                true, //false,
                _draw_system->GetRenderTargetBackBuffer()
            );
        }

        frame->SetRenderTarget(_draw_system->GetRenderTargetBackBuffer(), false);
        if (_resources->_onscreen_version)
        {
            _resources->_onscreen_version->Update(*_draw_system, *frame, *_resources->_text_manager);
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


