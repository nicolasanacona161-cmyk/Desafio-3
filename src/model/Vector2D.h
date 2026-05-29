#pragma once

#include <cmath>

struct Vector2D {
    float x = 0.0f;
    float y = 0.0f;

    Vector2D() = default;
    Vector2D(float xValue, float yValue) : x(xValue), y(yValue) {}

    float magnitud() const
    {
        return std::sqrt(x * x + y * y);
    }

    Vector2D normalizar() const
    {
        const float m = magnitud();
        if (m <= 0.0001f) {
            return {};
        }
        return {x / m, y / m};
    }

    Vector2D operator+(const Vector2D& other) const { return {x + other.x, y + other.y}; }
    Vector2D operator-(const Vector2D& other) const { return {x - other.x, y - other.y}; }
    Vector2D operator*(float scalar) const { return {x * scalar, y * scalar}; }
    Vector2D& operator+=(const Vector2D& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
};
