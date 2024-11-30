#include "c_camera.h"

cc::Matrix4x4 make_camera_view_matrix(const Camera &cam, const cc::Vec2DInt windowSize)
{
    cc::Matrix4x4 mat = {};
    mat[0][0] = Camera::k_scale;
    mat[1][1] = Camera::k_scale;
    mat[3][3] = 1.0f;
    mat[3][0] = -(cam.pos.x - (windowSize.x / 2.0f));
    mat[3][1] = -(cam.pos.y - (windowSize.y / 2.0f));
    return mat;
}
