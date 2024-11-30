#pragma once

#include <castle_common/cc_math.h>

struct Camera
{
    static constexpr float k_scale = 2.0f; // TEMP: Likely to be modifiable in an options menu later.
    cc::Vec2D pos;
};

cc::Matrix4x4 make_camera_view_matrix(const Camera &cam, const cc::Vec2DInt windowSize);
