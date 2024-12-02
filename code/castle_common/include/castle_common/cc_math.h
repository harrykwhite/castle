#pragma once

#include <math.h>

namespace cc
{

struct Vec2D
{
    float x, y;

    constexpr Vec2D operator+(const Vec2D &other) const
    {
        return {x + other.x, y + other.y};
    }

    constexpr Vec2D &operator+=(const Vec2D &other)
    {
        x += other.x;
        y += other.y;

        return *this;
    }

    constexpr Vec2D operator-(const Vec2D &other) const
    {
        return {x - other.x, y - other.y};
    }

    constexpr Vec2D &operator-=(const Vec2D &other)
    {
        x -= other.x;
        y -= other.y;

        return *this;
    }

    constexpr Vec2D operator*(const float scalar) const
    {
        return {x * scalar, y * scalar};
    }

    constexpr Vec2D &operator*=(const float scalar)
    {
        x *= scalar;
        y *= scalar;

        return *this;
    }

    constexpr Vec2D operator/(const float scalar) const
    {
        return {x / scalar, y / scalar};
    }

    constexpr Vec2D &operator/=(const float scalar)
    {
        x /= scalar;
        y /= scalar;

        return *this;
    }

    constexpr Vec2D operator-() const
    {
        return {-x, -y};
    }

    constexpr bool operator==(const Vec2D &other) const
    {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(const Vec2D &other) const
    {
        return !(*this == other);
    }
};

struct Vec2DInt
{
    int x, y;

    operator struct Vec2D() const {
        return Vec2D(x, y);
    }

    constexpr Vec2DInt operator+(const Vec2DInt &other) const
    {
        return {x + other.x, y + other.y};
    }

    constexpr Vec2DInt &operator+=(const Vec2DInt &other)
    {
        x += other.x;
        y += other.y;

        return *this;
    }

    constexpr Vec2DInt operator-(const Vec2DInt &other) const
    {
        return {x - other.x, y - other.y};
    }

    constexpr Vec2DInt &operator-=(const Vec2DInt &other)
    {
        x -= other.x;
        y -= other.y;

        return *this;
    }

    constexpr Vec2DInt operator*(const int scalar) const
    {
        return {x * scalar, y * scalar};
    }

    constexpr Vec2DInt &operator*=(const int scalar)
    {
        x *= scalar;
        y *= scalar;

        return *this;
    }

    constexpr Vec2DInt operator/(const int scalar) const
    {
        return {x / scalar, y / scalar};
    }

    constexpr Vec2DInt &operator/=(const int scalar)
    {
        x /= scalar;
        y /= scalar;

        return *this;
    }

    constexpr Vec2DInt operator-() const
    {
        return {-x, -y};
    }

    constexpr bool operator==(const Vec2DInt &other) const
    {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(const Vec2DInt &other) const
    {
        return !(*this == other);
    }
};

struct Matrix4x4
{
    float elems[4][4];

    float *operator[](const int index)
    {
        return elems[index];
    }
};

union RectFloat
{
    struct {
        Vec2D pos;
        Vec2D size;
    };

    struct {
        float x, y;
        float width, height;
    };

    inline float right() const
    {
        return x + width;
    }

    inline float bottom() const
    {
        return y + height;
    }

    bool operator==(const RectFloat &other) const
    {
        return pos == other.pos && size == other.size;
    }

    bool operator!=(const RectFloat &other) const
    {
        return !(*this == other);
    }
};

union Rect
{
    struct {
        Vec2DInt pos;
        Vec2DInt size;
    };

    struct {
        int x, y;
        int width, height;
    };

    operator union RectFloat() const {
        return {pos, size};
    }

    inline int right() const
    {
        return x + width;
    }

    inline int bottom() const
    {
        return y + height;
    }

    bool operator==(const Rect &other) const
    {
        return pos == other.pos && size == other.size;
    }

    bool operator!=(const Rect &other) const
    {
        return !(*this == other);
    }
};

struct Range
{
    int begin, end;
};

inline Matrix4x4 make_identity_matrix_4x4()
{
    Matrix4x4 mat = {};
    mat[0][0] = 1.0f;
    mat[1][1] = 1.0f;
    mat[2][2] = 1.0f;
    mat[3][3] = 1.0f;
    return mat;
}

inline Matrix4x4 make_ortho_matrix_4x4(const float left, const float right, const float bottom, const float top, const float near, const float far)
{
    Matrix4x4 mat = {};
    mat[0][0] = 2.0f / (right - left);
    mat[1][1] = 2.0f / (top - bottom);
    mat[2][2] = -2.0f / (far - near);
    mat[3][0] = -(right + left) / (right - left);
    mat[3][1] = -(top + bottom) / (top - bottom);
    mat[3][2] = -(far + near) / (far - near);
    mat[3][3] = 1.0f;
    return mat;
}

inline bool do_rects_intersect(const RectFloat &a, const RectFloat &b)
{
    return a.right() > b.x && b.right() > a.x && a.bottom() > b.y && b.bottom() > a.y;
}

inline float calc_dir(const Vec2D src, const Vec2D dest)
{
    return atan2f(src.y - dest.y, dest.x - src.x);
}

}
