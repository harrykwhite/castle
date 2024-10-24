#pragma once

namespace cc
{

struct s_vec_2d
{
    float x, y;

    s_vec_2d operator+(const s_vec_2d &other) const
    {
        return {x + other.x, y + other.y};
    }

    s_vec_2d &operator+=(const s_vec_2d &other)
    {
        x += other.x;
        y += other.y;

        return *this;
    }

    s_vec_2d operator-(const s_vec_2d &other) const
    {
        return {x - other.x, y - other.y};
    }

    s_vec_2d &operator-=(const s_vec_2d &other)
    {
        x -= other.x;
        y -= other.y;

        return *this;
    }

    s_vec_2d operator*(const float scalar) const
    {
        return {x * scalar, y * scalar};
    }

    s_vec_2d &operator*=(const float scalar)
    {
        x *= scalar;
        y *= scalar;

        return *this;
    }

    s_vec_2d operator/(const float scalar) const
    {
        return {x / scalar, y / scalar};
    }

    s_vec_2d &operator/=(const float scalar)
    {
        x /= scalar;
        y /= scalar;

        return *this;
    }

    bool operator==(const s_vec_2d &other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const s_vec_2d &other) const
    {
        return !(*this == other);
    }
};

struct s_vec_2d_int
{
    int x, y;

    s_vec_2d_int operator+(const s_vec_2d_int &other) const
    {
        return {x + other.x, y + other.y};
    }

    s_vec_2d_int &operator+=(const s_vec_2d_int &other)
    {
        x += other.x;
        y += other.y;

        return *this;
    }

    s_vec_2d_int operator-(const s_vec_2d_int &other) const
    {
        return {x - other.x, y - other.y};
    }

    s_vec_2d_int &operator-=(const s_vec_2d_int &other)
    {
        x -= other.x;
        y -= other.y;

        return *this;
    }

    s_vec_2d_int operator*(const int scalar) const
    {
        return {x * scalar, y * scalar};
    }

    s_vec_2d_int &operator*=(const int scalar)
    {
        x *= scalar;
        y *= scalar;

        return *this;
    }

    s_vec_2d_int operator/(const int scalar) const
    {
        return {x / scalar, y / scalar};
    }

    s_vec_2d_int &operator/=(const int scalar)
    {
        x /= scalar;
        y /= scalar;

        return *this;
    }

    bool operator==(const s_vec_2d_int &other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const s_vec_2d_int &other) const
    {
        return !(*this == other);
    }
};

struct s_rect
{
    int x, y;
    int width, height;

    inline s_vec_2d_int get_pos() const
    {
        return {x, y};
    }

    inline s_vec_2d_int get_size() const
    {
        return {width, height};
    }

    inline int get_right() const
    {
        return x + width;
    }

    inline int get_bottom() const
    {
        return y + height;
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

constexpr int get_min_int(const int x, const int y)
{
    return x <= y ? x : y;
}

constexpr float get_min_float(const float x, const float y)
{
    return x <= y ? x : y;
}

constexpr int get_max_int(const int x, const int y)
{
    return x >= y ? x : y;
}

constexpr float get_max_float(const float x, const float y)
{
    return x >= y ? x : y;
}

constexpr s_rect create_rect(const int x, const int y, const int width, const int height)
{
    return {x, y, width, height};
}

}
