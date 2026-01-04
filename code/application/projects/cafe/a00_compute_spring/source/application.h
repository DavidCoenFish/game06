#pragma once
#include "a00_compute_spring.h"

#include <dsc_common/dsc_common.h>
#include <dsc_windows/i_window_application.h>

#define LOG_TOPIC_APPLICATION "APPLICATION"

namespace DscCamera
{
    class Camera;
}

namespace DscCommon
{
    class FileSystem;
    template <typename TYPE>
    class Vector4;
    typedef Vector4<float> VectorFloat4;
}

namespace DscOnscreenDebug
{
    class OnscreenDebug;
}

namespace DscOnscreenVersion
{
    class OnscreenVersion;
}

namespace DscRender
{
    class DrawSystem;
}

namespace DscRenderResource
{
    class Frame;
    class GeometryGeneric;
    class RenderTargetTexture;
    class Shader;
    class ShaderResource;
    class ShaderConstantBuffer;
    class UnorderedAccess;
}

namespace DscStatistics
{
    class EventStore;
}

namespace DscText
{
    class TextManager;
}

class Application : public DscWindows::IWindowApplication
{
public:
    Application(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight);
    ~Application();

    Application() = delete;
    Application& operator=(const Application&) = delete;
    Application(const Application&) = delete;

private:
    virtual const bool Update() override;
    virtual void OnKey(const int in_vk_code, const int in_scan_code, const bool in_repeat_flag, const int in_repeat_count, bool in_up_flag) override;
    virtual void OnWindowSizeChanged(const DscCommon::VectorInt2& in_size, const float in_monitor_scale) override;

    void StepPosDataIndex();
    const int32 GetPosDataIndex() const;
    const int32 GetPosDataIndexPrev() const;
    const int32 GetPosDataIndexPrevPrev() const;

    void PresentPos(std::unique_ptr<DscRenderResource::Frame>& in_frame, const int32 in_pos_data_index, const DscCommon::VectorFloat4& in_colour);

    void UpdateDebugText();

private:
    typedef DscWindows::IWindowApplication BaseType;

    std::unique_ptr<DscCommon::FileSystem> _file_system;
    std::unique_ptr<DscRender::DrawSystem> _draw_system;
    std::unique_ptr<DscStatistics::EventStore> _event_store;

    static constexpr int32 k_pos_data_count = 3;
    int32 _pos_data_index = 0;

    struct Resources
    {
        Resources();
        Resources& operator=(const Resources&) = delete;
        Resources(const Resources&) = delete;

        std::unique_ptr<DscText::TextManager> _text_manager;
        std::unique_ptr<DscOnscreenVersion::OnscreenVersion> _onscreen_version = {};
        std::unique_ptr<DscOnscreenDebug::OnscreenDebug> _onscreen_debug = {};
        std::unique_ptr<DscCamera::Camera> _camera = {};

        // unable to get StructBuffer acceleration to clear, so make a shader to do it
        std::shared_ptr<DscRenderResource::Shader> _clear_spring_acceleration_shader;
        std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _clear_spring_acceleration_constant_buffer;

        std::shared_ptr<DscRenderResource::Shader> _accumulate_spring_acceleration_shader;
        std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _accumulate_spring_acceleration_constant_buffer;

        std::shared_ptr<DscRenderResource::Shader> _click_spring_acceleration_shader;
        std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _click_spring_acceleration_constant_buffer;

        std::shared_ptr<DscRenderResource::Shader> _apply_spring_acceleration_shader;
        std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _apply_spring_acceleration_constant_buffer;

        // 3 pos textures, [prev prev pos , prev pos, pos] (tripple buffer of pos so we can just fetch the current vel from (prev - prev2)
        std::shared_ptr<DscRenderResource::UnorderedAccess> _pos_data[k_pos_data_count];

        // shader resource read only
        std::shared_ptr<DscRenderResource::UnorderedAccess> _link_data;

        // read as shader resource, write as unordered access view
        std::shared_ptr<DscRenderResource::UnorderedAccess> _acceleration_data;

        // draw the given pos texture as point cloud with the provided colour
        std::shared_ptr<DscRenderResource::Shader> _present_pos_shader;
        std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _present_pos_constant_buffer[k_pos_data_count];

        // geometry data is the pos index?
        std::shared_ptr<DscRenderResource::GeometryGeneric> _geometry;
    };
    std::unique_ptr<Resources> _resources;

    struct TClearSpringAccelerationConstantBuffer
    {
        int _acceleration_count;
        // clear value, like a gravity constant?
    };
    TClearSpringAccelerationConstantBuffer _clear_spring_acceleration_constant_buffer = {};

    struct TAccumulateSpringAccelerationConstantBuffer
    {
        int _link_count;
        float _spring_constant;
    };
    TAccumulateSpringAccelerationConstantBuffer _accumulate_spring_acceleration_constant_buffer = {};

    struct TClickSpringAccelerationConstantBuffer
    {
        float _click_pos_weight[4];
        float _click_norm_range[4];
        int _pos_count;
    };
    TClickSpringAccelerationConstantBuffer _click_spring_acceleration_constant_buffer = {};

    struct TApplySpringAccelerationConstantBuffer
    {
        int _point_count;
        float _time_step;
        float _dampen;
    };
    TApplySpringAccelerationConstantBuffer _apply_spring_acceleration_constant_buffer = {};

};