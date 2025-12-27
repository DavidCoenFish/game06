#include "camera.h"
#include <dsc_common/log_system.h>
#include <dsc_common/vector_int2.h>
#include <dsc_common/vector_float3.h>
#include <dsc_common/math_angle.h>
#include <dsc_windows/window_helper.h>

/*
static const DirectX::XMFLOAT3 RotateAxis(const DirectX::XMFLOAT3& vector, const DirectX::XMFLOAT3& axis, const float radians)
{
   const DirectX::XMFLOAT3 paralletToK = axis * Dot(axis, vector);
   //const DirectX::XMFLOAT3 perpendicularToK = Cross(axis, vector);
   const DirectX::XMFLOAT3 perpendicularToK = vector - paralletToK;
   const DirectX::XMFLOAT3 kCrossV = Cross(axis, vector);

   DirectX::XMFLOAT3 result = paralletToK + (perpendicularToK * (float)cos(radians)) + (kCrossV * (float)sin(radians));
   return result;
}

*/

DscCamera::Camera::Camera(
    const DscCommon::VectorFloat3& in_initial_pos,
    const DscCommon::VectorFloat3& in_initial_at,
    const DscCommon::VectorFloat3& in_initial_up,
    const float in_far,
    const float in_target_vertical_fov_deg,
    const DscCommon::VectorInt2& in_viewport_size
)
    : _target_vertical_fov_deg(in_target_vertical_fov_deg)
    , _initial_pos(in_initial_pos)
    , _initial_at(in_initial_at)
    , _initial_up(in_initial_up)
{
    _constant_buffer._camera_pos_fov_horizontal[0] = in_initial_pos.GetX();
    _constant_buffer._camera_pos_fov_horizontal[1] = in_initial_pos.GetY();
    _constant_buffer._camera_pos_fov_horizontal[2] = in_initial_pos.GetZ();

    _constant_buffer._camera_at_fov_vertical[0] = in_initial_at.GetX();
    _constant_buffer._camera_at_fov_vertical[1] = in_initial_at.GetY();
    _constant_buffer._camera_at_fov_vertical[2] = in_initial_at.GetZ();

    _constant_buffer._camera_up_camera_far[0] = in_initial_up.GetX();
    _constant_buffer._camera_up_camera_far[1] = in_initial_up.GetY();
    _constant_buffer._camera_up_camera_far[2] = in_initial_up.GetZ();

    _constant_buffer._camera_up_camera_far[3] = in_far;

    OnResize(in_viewport_size);

    return;
}

void DscCamera::Camera::OnResize(const DscCommon::VectorInt2& in_viewport_size)
{
    _viewport_size = in_viewport_size;
    const float mul = 0 == in_viewport_size.GetY() ? 1.0f : (float)in_viewport_size.GetX() / (float)in_viewport_size.GetY();
    _constant_buffer._camera_at_fov_vertical[3] = DscCommon::MathAngle::DegToRadian(_target_vertical_fov_deg);
    _constant_buffer._camera_pos_fov_horizontal[3] = DscCommon::MathAngle::DegToRadian(_target_vertical_fov_deg * mul);
    return;
}

void DscCamera::Camera::OnKey(const int in_vk_code, const int in_scan_code, const bool in_repeat_flag, const int in_repeat_count, bool in_up_flag)
{
    DSC_UNUSED(in_vk_code);
    DSC_UNUSED(in_scan_code);
    DSC_UNUSED(in_repeat_flag);
    DSC_UNUSED(in_repeat_count);
    DSC_UNUSED(in_up_flag);

    DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_CAMERA, "in_vk_code:%d, in_scan_code:%d, in_repeat_flag:%d, in_repeat_count:%d, in_up_flag:%d\n", in_vk_code, in_scan_code, in_repeat_flag, in_repeat_count, in_up_flag);

    switch (in_vk_code)
    {
    default:
        break;
    case VK_PRIOR:
        _move_up_repeat_count += in_repeat_count;
        break;
    case VK_NEXT:
        _move_up_repeat_count -= in_repeat_count;
        break;
    case VK_LEFT:
        _move_right_repeat_count += in_repeat_count;
        break;
    case VK_UP: //38:
        _move_at_repeat_count += in_repeat_count;
        break;
    case VK_RIGHT:
        _move_right_repeat_count -= in_repeat_count;
        break;
    case VK_DOWN: //40:
        _move_at_repeat_count -= in_repeat_count;
        break;
    case VK_HOME:
        _constant_buffer._camera_pos_fov_horizontal[0] = _initial_pos.GetX();
        _constant_buffer._camera_pos_fov_horizontal[1] = _initial_pos.GetY();
        _constant_buffer._camera_pos_fov_horizontal[2] = _initial_pos.GetZ();

        _constant_buffer._camera_at_fov_vertical[0] = _initial_at.GetX();
        _constant_buffer._camera_at_fov_vertical[1] = _initial_at.GetY();
        _constant_buffer._camera_at_fov_vertical[2] = _initial_at.GetZ();

        _constant_buffer._camera_up_camera_far[0] = _initial_up.GetX();
        _constant_buffer._camera_up_camera_far[1] = _initial_up.GetY();
        _constant_buffer._camera_up_camera_far[2] = _initial_up.GetZ();

        break;
    }
}

// mouse rotate camera, input move camera
void DscCamera::Camera::Update(HWND in_hwnd, const bool in_has_focus)
{
    DscCommon::VectorInt2 mouse_pos = {};
    bool left_button = false;
    bool right_button = false;

    // top left is 0,0 and +x is across the screen, +y is down the screen
    const bool mouse_valid = DscWindows::GetMouseState(in_hwnd, mouse_pos, left_button, right_button);
    if (_has_focus && in_has_focus && mouse_valid)
    {
        if (left_button)
        {
            const DscCommon::VectorInt2 half_viewport = _viewport_size / 2;
            const int32 min_half_viewport = std::min(half_viewport.GetX(), half_viewport.GetY());
            const int32 rotate_z_threashold = min_half_viewport * min_half_viewport * 8 / 10;
            const DscCommon::VectorInt2 mouse_viewport_middle_relative = mouse_pos - half_viewport;
            const int32 mouse_radius = DscCommon::Dot(mouse_viewport_middle_relative, mouse_viewport_middle_relative);
            bool modified = false;
            DscCommon::VectorFloat3 at(_constant_buffer._camera_at_fov_vertical[0], _constant_buffer._camera_at_fov_vertical[1], _constant_buffer._camera_at_fov_vertical[2]);
            DscCommon::VectorFloat3 up(_constant_buffer._camera_up_camera_far[0], _constant_buffer._camera_up_camera_far[1], _constant_buffer._camera_up_camera_far[2]);

            // rotate around up, right
            if (mouse_radius < rotate_z_threashold)
            {
                // we also have _target_vertical_fov_deg
                const float deg_per_pixel = _target_vertical_fov_deg / (float)_viewport_size.GetY();
                const float around_up = -DscCommon::MathAngle::DegToRadian(deg_per_pixel * (float)(mouse_pos.GetX() - _prev_mouse_pos.GetX()));
                const float around_right = DscCommon::MathAngle::DegToRadian(deg_per_pixel * (float)(mouse_pos.GetY() - _prev_mouse_pos.GetY()));

                if (0.0f != around_up)
                {
                    at = DscCommon::RotateVectorAroundAxis(
                        at,
                        up,
                        around_up
                    );

                    modified = true;
                }

                if (0.0f != around_right)
                {
                    const DscCommon::VectorFloat3 right = DscCommon::Cross(at, up);
                    at = DscCommon::RotateVectorAroundAxis(
                        at,
                        right,
                        around_right
                    );
                    up = DscCommon::RotateVectorAroundAxis(
                        up,
                        right,
                        around_right
                    );
                    modified = true;
                }

            }
            else // rotate around at
            {
                const DscCommon::VectorInt2 prev_mouse_viewport_middle_relative = _prev_mouse_pos - half_viewport;
                const float around_at = DscCommon::MathAngle::SmallestDelta(
                    atan2<float>((float)prev_mouse_viewport_middle_relative.GetY(), (float)prev_mouse_viewport_middle_relative.GetX()),
                    atan2<float>((float)mouse_viewport_middle_relative.GetY(), (float)mouse_viewport_middle_relative.GetX())
                );

                if (0.0f != around_at)
                {
                    up = DscCommon::RotateVectorAroundAxis(
                        up,
                        at,
                        around_at
                        );
                    modified = true;
                }
            }

            if (true == modified)
            {
                at = DscCommon::ReturnNormal(at);
                up = DscCommon::ReturnNormal(up);
                const DscCommon::VectorFloat3 right = DscCommon::Cross(at, up);

                up = DscCommon::Cross(right, at);
                up = DscCommon::ReturnNormal(up);

                _constant_buffer._camera_at_fov_vertical[0] = at.GetX();
                _constant_buffer._camera_at_fov_vertical[1] = at.GetY();
                _constant_buffer._camera_at_fov_vertical[2] = at.GetZ();
                _constant_buffer._camera_up_camera_far[0] = up.GetX();
                _constant_buffer._camera_up_camera_far[1] = up.GetY();
                _constant_buffer._camera_up_camera_far[2] = up.GetZ();
            }
        }
    }

    if (in_has_focus && mouse_valid)
    {
        _prev_mouse_pos = mouse_pos;
        _has_focus = true;
    }
    else
    {
        _has_focus = false;
    }

    if ((0 != _move_at_repeat_count) || (0 != _move_right_repeat_count) || (0 != _move_up_repeat_count))
    {
        DscCommon::VectorFloat3 pos(_constant_buffer._camera_pos_fov_horizontal[0], _constant_buffer._camera_pos_fov_horizontal[1], _constant_buffer._camera_pos_fov_horizontal[2]);
        const DscCommon::VectorFloat3 at(_constant_buffer._camera_at_fov_vertical[0], _constant_buffer._camera_at_fov_vertical[1], _constant_buffer._camera_at_fov_vertical[2]);
        const DscCommon::VectorFloat3 up(_constant_buffer._camera_up_camera_far[0], _constant_buffer._camera_up_camera_far[1], _constant_buffer._camera_up_camera_far[2]);
        const DscCommon::VectorFloat3 right = DscCommon::Cross(at, up);

        pos += (at * (0.05f * (float)_move_at_repeat_count));
        pos += (right * (0.05f * (float)_move_right_repeat_count));
        pos += (up * (0.05f * (float)_move_up_repeat_count));

        _constant_buffer._camera_pos_fov_horizontal[0] = pos.GetX();
        _constant_buffer._camera_pos_fov_horizontal[1] = pos.GetY();
        _constant_buffer._camera_pos_fov_horizontal[2] = pos.GetZ();

        _move_at_repeat_count = 0;
        _move_right_repeat_count = 0;
        _move_up_repeat_count = 0;
    }
}
