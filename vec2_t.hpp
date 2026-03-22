#pragma once
#include <cmath>

struct vec2_t
{
    float x;
    float y;

    vec2_t(float _x = 0, float _y = 0) : x(_x) , y(_y) {}
    // vec2_t(int _x, int _y) : x((_x)) , y((_y)) {}
    

    // --- Operators ---

    // Addition: (x1 + x2, y1 + y2)
    vec2_t operator+(const vec2_t& v) const {
        return { x + v.x, y + v.y };
    }

    // Subtraction: (x1 - x2, y1 - y2)
    vec2_t operator-(const vec2_t& v) const {
        return { x - v.x, y - v.y };
    }

    // Scalar Multiplication: (x * s, y * s)
    // Useful for scaling a vector
    vec2_t operator*(float scalar) const {
        return { x * scalar, y * scalar };
    }

    // Scalar Division: (x / s, y / s)
    vec2_t operator/(float scalar) const {
        // Optimization: multiply by inverse to avoid repeated division
        float inv = 1.0f / scalar;
        return { x * inv, y * inv };
    }

    // --- Functions ---

    // Length (Magnitude): sqrt(x^2 + y^2)
    int length() const {
        return std::sqrt(x * x + y * y);
    }

    // Dot Product: x1*x2 + y1*y2
    // Used for lighting and checking perpendicularity
    int dot(const vec2_t& v) const {
        return (x * v.x) + (y * v.y);
    }

    // Cross Product (2D Specific): x1*y2 - y1*x2
    // In 2D, this returns a scalar (float).
    // CRITICAL for rasterization: It helps calculate "Edge Functions"
    // to determine if a pixel is inside a triangle.
    int cross(const vec2_t& v) const {
        return (x * v.y) - (y * v.x);
    }

    // Normalize: Make length = 1
    void normalize() {
        int len = length();
        if (len > 0) {
            x /= len;
            y /= len;
        }
    }

    vec2_t vec2_rotate(vec2_t v, vec2_t center, float angle) {
        vec2_t rotation;
        v.x -= center.x;
        v.y -= center.y;
        rotation.x = v.x*cos(angle) - v.y*sin(angle);
        rotation.y = v.x*sin(angle) + v.y*cos(angle);
        rotation.x += center.x;
        rotation.y += center.y;
        rotation.x = std::round(rotation.x);
        rotation.y = std::round(rotation.y);
        return rotation;
    }

};
