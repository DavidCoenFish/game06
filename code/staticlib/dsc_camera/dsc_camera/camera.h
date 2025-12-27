#pragma once
#include "dsc_camera.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_int2.h>
#include <dsc_common/vector_float3.h>

/*
namespace DscCommon
{
    template <typename TYPE>
    class Vector3;
    typedef Vector3<float> VectorFloat3;
}
*/

namespace DscCamera
{
    class Camera {
    public:
        // On one hand, this increaces coupling by using a layout that may be of use to a render shader, on the other hand, convienience
        struct TConstantBuffer
        {
            float _camera_pos_fov_horizontal[4];
            float _camera_at_fov_vertical[4];
            float _camera_up_camera_far[4];
        };

        Camera(
            const DscCommon::VectorFloat3& in_initial_pos,
            const DscCommon::VectorFloat3& in_initial_at,
            const DscCommon::VectorFloat3& in_initial_up,
            const float in_far,
            const float in_target_vertical_fov_deg,
            const DscCommon::VectorInt2& in_viewport_size
        );

        void OnResize(const DscCommon::VectorInt2& in_viewport_size);
        void OnKey(const int in_vk_code, const int in_scan_code, const bool in_repeat_flag, const int in_repeat_count, bool in_up_flag);

        // mouse rotate camera, input move camera
        void Update(HWND in_hwnd, const bool in_has_focus);

        const TConstantBuffer& GetConstantBuffer() const {
            return _constant_buffer;
        }

    private:
        TConstantBuffer _constant_buffer;

        DscCommon::VectorInt2 _viewport_size;

        // add bool/ enum to select vertial or horizontal dominant calculation, or lambda to calculate hor/vert fov given screen size
        float _target_vertical_fov_deg = 0.0f;

        bool _has_focus = false;

        // prev input state, mouse pos, buttons up/down
        DscCommon::VectorInt2 _prev_mouse_pos;

        int32 _move_at_repeat_count = 0;
        int32 _move_right_repeat_count = 0;
        int32 _move_up_repeat_count = 0;

        DscCommon::VectorFloat3 _initial_pos;
        DscCommon::VectorFloat3 _initial_at;
        DscCommon::VectorFloat3 _initial_up;

    };
}