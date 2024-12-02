#include "c_camera.h"

#include "c_game.h"

cc::Matrix4x4 make_camera_view_matrix(const Camera &cam)
{
    cc::Matrix4x4 mat = {};
    mat[0][0] = gk_cameraScale;
    mat[1][1] = gk_cameraScale;
    mat[3][3] = 1.0f;
    mat[3][0] = -(cam.pos.x - (get_window_size().x / 2.0f));
    mat[3][1] = -(cam.pos.y - (get_window_size().y / 2.0f));
    return mat;
}

cc::Vec2D camera_to_screen_pos(const cc::Vec2D pos, const Camera &cam)
{
    return {
        ((pos.x - cam.pos.x) * gk_cameraScale) + (get_window_size().x / 2.0f),
        ((pos.y - cam.pos.y) * gk_cameraScale) + (get_window_size().y / 2.0f)
    };
}

cc::Vec2D screen_to_camera_pos(const cc::Vec2D pos, const Camera &cam)
{
    return {
        ((pos.x - (get_window_size().x / 2.0f)) / gk_cameraScale) + cam.pos.x,
        ((pos.y - (get_window_size().y / 2.0f)) / gk_cameraScale) + cam.pos.y
    };
}
