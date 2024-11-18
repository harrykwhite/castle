#pragma once

#include <cmath>

namespace cc
{

struct s_vec_2d
{
    float x, y;

    constexpr s_vec_2d operator+(const s_vec_2d &other) const
    {
        return {x + other.x, y + other.y};
    }

    constexpr s_vec_2d &operator+=(const s_vec_2d &other)
    {
        x += other.x;
        y += other.y;

        return *this;
    }

    constexpr s_vec_2d operator-(const s_vec_2d &other) const
    {
        return {x - other.x, y - other.y};
    }

    constexpr s_vec_2d &operator-=(const s_vec_2d &other)
    {
        x -= other.x;
        y -= other.y;

        return *this;
    }

    constexpr s_vec_2d operator*(const float scalar) const
    {
        return {x * scalar, y * scalar};
    }

    constexpr s_vec_2d &operator*=(const float scalar)
    {
        x *= scalar;
        y *= scalar;

        return *this;
    }

    constexpr s_vec_2d operator/(const float scalar) const
    {
        return {x / scalar, y / scalar};
    }

    constexpr s_vec_2d &operator/=(const float scalar)
    {
        x /= scalar;
        y /= scalar;

        return *this;
    }

    constexpr s_vec_2d operator-() const
    {
        return {-x, -y};
    }

    constexpr bool operator==(const s_vec_2d &other) const
    {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(const s_vec_2d &other) const
    {
        return !(*this == other);
    }
};

struct s_vec_2d_i
{
    int x, y;

    constexpr s_vec_2d_i operator+(const s_vec_2d_i &other) const
    {
        return {x + other.x, y + other.y};
    }

    constexpr s_vec_2d_i &operator+=(const s_vec_2d_i &other)
    {
        x += other.x;
        y += other.y;

        return *this;
    }

    constexpr s_vec_2d_i operator-(const s_vec_2d_i &other) const
    {
        return {x - other.x, y - other.y};
    }

    constexpr s_vec_2d_i &operator-=(const s_vec_2d_i &other)
    {
        x -= other.x;
        y -= other.y;

        return *this;
    }

    constexpr s_vec_2d_i operator*(const int scalar) const
    {
        return {x * scalar, y * scalar};
    }

    constexpr s_vec_2d_i &operator*=(const int scalar)
    {
        x *= scalar;
        y *= scalar;

        return *this;
    }

    constexpr s_vec_2d_i operator/(const int scalar) const
    {
        return {x / scalar, y / scalar};
    }

    constexpr s_vec_2d_i &operator/=(const int scalar)
    {
        x /= scalar;
        y /= scalar;

        return *this;
    }

    constexpr s_vec_2d_i operator-() const
    {
        return {-x, -y};
    }

    constexpr bool operator==(const s_vec_2d_i &other) const
    {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(const s_vec_2d_i &other) const
    {
        return !(*this == other);
    }
};

struct s_matrix_4x4
{
    static inline s_matrix_4x4 identity()
    {
        s_matrix_4x4 mat = {};
        mat[0][0] = 1.0f;
        mat[1][1] = 1.0f;
        mat[2][2] = 1.0f;
        mat[3][3] = 1.0f;
        return mat;
    }

    static inline s_matrix_4x4 ortho(const float left, const float right, const float bottom, const float top, const float near, const float far)
    {
        s_matrix_4x4 mat = {};
        mat[0][0] = 2.0f / (right - left);
        mat[1][1] = 2.0f / (top - bottom);
        mat[2][2] = -2.0f / (far - near);
        mat[3][0] = -(right + left) / (right - left);
        mat[3][1] = -(top + bottom) / (top - bottom);
        mat[3][2] = -(far + near) / (far - near);
        mat[3][3] = 1.0f;
        return mat;
    }

    float elems[4][4];

    float *operator[](const int index)
    {
        return elems[index];
    }
};

struct s_rect
{
    int x, y;
    int width, height;

    inline int get_right() const
    {
        return x + width;
    }

    inline int get_bottom() const
    {
        return y + height;
    }

    inline bool intersects(const s_rect &other) const
    {
        return get_right() > other.x && other.get_right() > x && get_bottom() > other.y && other.get_bottom() > y;
    }

    bool operator==(const s_rect &other) const
    {
        return x == other.x && y == other.y && width == other.width && height == other.height;
    }

    bool operator!=(const s_rect &other) const
    {
        return !(*this == other);
    }
};

struct s_rect_f
{
    float x, y;
    float width, height;

    inline float get_right() const
    {
        return x + width;
    }

    inline float get_bottom() const
    {
        return y + height;
    }

    inline bool intersects(const s_rect_f &other) const
    {
        return get_right() > other.x && other.get_right() > x && get_bottom() > other.y && other.get_bottom() > y;
    }

    bool operator==(const s_rect_f &other) const
    {
        return x == other.x && y == other.y && width == other.width && height == other.height;
    }

    bool operator!=(const s_rect_f &other) const
    {
        return !(*this == other);
    }
};

inline float get_dir(const s_vec_2d src, const s_vec_2d dest)
{
    return std::atan2f(src.y - dest.y, dest.x - src.x);
}

}
