#include <math.hpp>
#include <string.h>

bool operator==(const v2f& a, const v2f& b)
{
    return a.x == b.x && a.y == b.y;
}

v2f operator+(const v2f& a, const v2f& b)
{
    return
    {
        a.x + b.x,
        a.y + b.y,
    };
}

v2f operator-(const v2f& a, const v2f& b)
{
    return
    {
        a.x - b.x,
        a.y - b.y,
    };
}

v2f operator*(const v2f& a, const v2f& b)
{
    return
    {
        a.x * b.x,
        a.y * b.y,
    };
}

v2f operator*(const v2f& a, const f32& b)
{
    return
    {
        a.x * b,
        a.y * b,
    };
}

v2f operator*(const f32& a, const v2f& b)
{
    return
    {
        b.x * a,
        b.y * a,
    };
}

f32 Dot(const v2f& a, const v2f& b)
{
    return a.x * b.x + a.y * b.y;
}

f32 Cross(const v2f& a, const v2f& b)
{
    // Cross-product is only defined in 3D space. 2D version only returns Z coordinate.
    // This can be useful for calculating winding order between points, for example.
    return a.x * b.y - a.y * b.x;
}

f32 Len2(const v2f& v)
{
    return Dot(v, v);
}

f32 Len(const v2f& v)
{
    return sqrt(Len2(v));
}

v2f Normalize(const v2f& v)
{
    f32 l = Len(v);
    if(l < EPSILON_F32) return {0.f, 0.f};
    return v * (1.f/l);
}

f32 AngleBetween(const v2f& a, const v2f& b)
{
    // Angle in radians between 2 vectors, from 0 to PI
    return acos(Dot(Normalize(a), Normalize(b)));
}

bool operator==(const v2i& a, const v2i& b)
{
    return a.x == b.x && a.y == b.y;
}

v2i operator+(const v2i& a, const v2i& b)
{
    return
    {
        a.x + b.x,
        a.y + b.y,
    };
}

v2i operator-(const v2i& a, const v2i& b)
{
    return
    {
        a.x - b.x,
        a.y - b.y,
    };
}

bool operator==(const v3f& a, const v3f& b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

v3f operator+(const v3f& a, const v3f& b)
{
    return
    {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
    };
}

v3f operator-(const v3f& a, const v3f& b)
{
    return
    {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
    };
}

v3f operator*(const v3f& a, const v3f& b)
{
    return
    {
        a.x * b.x,
        a.y * b.y,
        a.z * b.z,
    };
}

v3f operator*(const v3f& a, const f32& b)
{
    return
    {
        a.x * b,
        a.y * b,
        a.z * b,
    };
}

v3f operator*(const f32& a, const v3f& b)
{
    return
    {
        b.x * a,
        b.y * a,
        b.z * a,
    };
}

f32 Dot(const v3f& a, const v3f& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

v3f Cross(const v3f& a, const v3f& b)
{
    return 
    {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

f32 Len2(const v3f& v)
{
    return Dot(v, v);
}

f32 Len(const v3f& v)
{
    return sqrt(Len2(v));
}

v3f Normalize(const v3f& v)
{
    f32 l = Len(v);
    if(l < EPSILON_F32) return {0.f, 0.f};
    return v * (1.f/l);
}

bool operator==(const v4f& a, const v4f& b)
{
    return a.x == b.x
        && a.y == b.y
        && a.z == b.z
        && a.w == b.w;
}

v4f operator+(const v4f& a, const v4f& b)
{
    return
    {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
        a.w + b.w,
    };
}

v4f operator-(const v4f& a, const v4f& b)
{
    return
    {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
        a.w - b.w,
    };
}

v4f operator*(const v4f& a, const v4f& b)
{
    return
    {
        a.x * b.x,
        a.y * b.y,
        a.z * b.z,
        a.w * b.w,
    };
}

v4f operator*(const v4f& a, const f32& b)
{
    return
    {
        a.x * b,
        a.y * b,
        a.z * b,
        a.w * b,
    };
}

v4f operator*(const f32& a, const v4f& b)
{
    return
    {
        b.x * a,
        b.y * a,
        b.z * a,
        b.w * a,
    };
}

f32 Dot(const v4f& a, const v4f& b)
{
    return a.x * b.x
         + a.y * b.y
         + a.z * b.z
         + a.w * b.w;
}

f32 Len2(const v4f& v)
{
    return Dot(v, v);
}

f32 Len(const v4f& v)
{
    return sqrt(Len2(v));
}

v4f Normalize(const v4f& v)
{
    f32 l = Len(v);
    if(l < EPSILON_F32) return {0.f, 0.f};
    return v * (1.f/l);
}

v4f v4f_AsDirection(const v3f& v)
{
    return {v.x, v.y, v.z, 0.f};
}

v4f v4f_AsPosition(const v3f& v)
{
    return {v.x, v.y, v.z, 1.f};
}

v3f v3f_As(const v4f& v)
{
    return {v.x, v.y, v.z};
}

bool operator==(const m4f& a, const m4f& b)
{
    return memcmp(a.data, b.data, sizeof(a.data)) == 0;
}

// TODO(caio)#MATH: Matrix operations should be vectorized
m4f operator+(const m4f& a, const m4f& b)
{
    return
    {
        a.m00 + b.m00,
        a.m01 + b.m01,
        a.m02 + b.m02,
        a.m03 + b.m03,
        a.m10 + b.m10,
        a.m11 + b.m11,
        a.m12 + b.m12,
        a.m13 + b.m13,
        a.m20 + b.m20,
        a.m21 + b.m21,
        a.m22 + b.m22,
        a.m23 + b.m23,
        a.m30 + b.m30,
        a.m31 + b.m31,
        a.m32 + b.m32,
        a.m33 + b.m33,
    };
}

m4f operator-(const m4f& a, const m4f& b)
{
    return
    {
        a.m00 - b.m00,
        a.m01 - b.m01,
        a.m02 - b.m02,
        a.m03 - b.m03,
        a.m10 - b.m10,
        a.m11 - b.m11,
        a.m12 - b.m12,
        a.m13 - b.m13,
        a.m20 - b.m20,
        a.m21 - b.m21,
        a.m22 - b.m22,
        a.m23 - b.m23,
        a.m30 - b.m30,
        a.m31 - b.m31,
        a.m32 - b.m32,
        a.m33 - b.m33,
    };
}

m4f operator*(const m4f& a, const m4f& b)
{
    return
    {
        // Row 0
        a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20 + a.m03 * b.m30, 
        a.m00 * b.m01 + a.m01 * b.m11 + a.m02 * b.m21 + a.m03 * b.m31, 
        a.m00 * b.m02 + a.m01 * b.m12 + a.m02 * b.m22 + a.m03 * b.m32, 
        a.m00 * b.m03 + a.m01 * b.m13 + a.m02 * b.m23 + a.m03 * b.m33, 

        // Row 1
        a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20 + a.m13 * b.m30, 
        a.m10 * b.m01 + a.m11 * b.m11 + a.m12 * b.m21 + a.m13 * b.m31, 
        a.m10 * b.m02 + a.m11 * b.m12 + a.m12 * b.m22 + a.m13 * b.m32, 
        a.m10 * b.m03 + a.m11 * b.m13 + a.m12 * b.m23 + a.m13 * b.m33, 

        // Row 2
        a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20 + a.m23 * b.m30, 
        a.m20 * b.m01 + a.m21 * b.m11 + a.m22 * b.m21 + a.m23 * b.m31, 
        a.m20 * b.m02 + a.m21 * b.m12 + a.m22 * b.m22 + a.m23 * b.m32, 
        a.m20 * b.m03 + a.m21 * b.m13 + a.m22 * b.m23 + a.m23 * b.m33, 

        // Row 3
        a.m30 * b.m00 + a.m31 * b.m10 + a.m32 * b.m20 + a.m33 * b.m30, 
        a.m30 * b.m01 + a.m31 * b.m11 + a.m32 * b.m21 + a.m33 * b.m31, 
        a.m30 * b.m02 + a.m31 * b.m12 + a.m32 * b.m22 + a.m33 * b.m32, 
        a.m30 * b.m03 + a.m31 * b.m13 + a.m32 * b.m23 + a.m33 * b.m33, 
    };
}

m4f operator*(const f32& b, const m4f& a)
{
    return
    {
        a.m00 * b, a.m01 * b, a.m02 * b, a.m03 * b,
        a.m10 * b, a.m11 * b, a.m12 * b, a.m13 * b,
        a.m20 * b, a.m21 * b, a.m22 * b, a.m23 * b,
        a.m30 * b, a.m31 * b, a.m32 * b, a.m33 * b,
    };
}

v4f operator*(const m4f& a, const v4f& v)
{
    return
    {
        a.m00 * v.x + a.m01 * v.y + a.m02 * v.z + a.m03 * v.w,
        a.m10 * v.x + a.m11 * v.y + a.m12 * v.z + a.m13 * v.w,
        a.m20 * v.x + a.m21 * v.y + a.m22 * v.z + a.m23 * v.w,
        a.m30 * v.x + a.m31 * v.y + a.m32 * v.z + a.m33 * v.w,
    };
}

f32 Determinant(const m4f& m)
{
    return
        m.m03 * m.m12 * m.m21 * m.m30 - m.m02 * m.m13 * m.m21 * m.m30 -
        m.m03 * m.m11 * m.m22 * m.m30 + m.m01 * m.m13 * m.m22 * m.m30 +
        m.m02 * m.m11 * m.m23 * m.m30 - m.m01 * m.m12 * m.m23 * m.m30 -
        m.m03 * m.m12 * m.m20 * m.m31 + m.m02 * m.m13 * m.m20 * m.m31 +
        m.m03 * m.m10 * m.m22 * m.m31 - m.m00 * m.m13 * m.m22 * m.m31 -
        m.m02 * m.m10 * m.m23 * m.m31 + m.m00 * m.m12 * m.m23 * m.m31 +
        m.m03 * m.m11 * m.m20 * m.m32 - m.m01 * m.m13 * m.m20 * m.m32 -
        m.m03 * m.m10 * m.m21 * m.m32 + m.m00 * m.m13 * m.m21 * m.m32 +
        m.m01 * m.m10 * m.m23 * m.m32 - m.m00 * m.m11 * m.m23 * m.m32 -
        m.m02 * m.m11 * m.m20 * m.m33 + m.m01 * m.m12 * m.m20 * m.m33 +
        m.m02 * m.m10 * m.m21 * m.m33 - m.m00 * m.m12 * m.m21 * m.m33 -
        m.m01 * m.m10 * m.m22 * m.m33 + m.m00 * m.m11 * m.m22 * m.m33;
}

m4f Transpose(const m4f& m)
{
    return
    {
        m.m00, m.m10, m.m20, m.m30,
        m.m01, m.m11, m.m21, m.m31,
        m.m02, m.m12, m.m22, m.m32,
        m.m03, m.m13, m.m23, m.m33,
    };
}

m4f Inverse(const m4f& m)
{
    f32 A2323 = m.m22 * m.m33 - m.m23 * m.m32;
    f32 A1323 = m.m21 * m.m33 - m.m23 * m.m31;
    f32 A1223 = m.m21 * m.m32 - m.m22 * m.m31;
    f32 A0323 = m.m20 * m.m33 - m.m23 * m.m30;
    f32 A0223 = m.m20 * m.m32 - m.m22 * m.m30;
    f32 A0123 = m.m20 * m.m31 - m.m21 * m.m30;
    f32 A2313 = m.m12 * m.m33 - m.m13 * m.m32;
    f32 A1313 = m.m11 * m.m33 - m.m13 * m.m31;
    f32 A1213 = m.m11 * m.m32 - m.m12 * m.m31;
    f32 A2312 = m.m12 * m.m23 - m.m13 * m.m22;
    f32 A1312 = m.m11 * m.m23 - m.m13 * m.m21;
    f32 A1212 = m.m11 * m.m22 - m.m12 * m.m21;
    f32 A0313 = m.m10 * m.m33 - m.m13 * m.m30;
    f32 A0213 = m.m10 * m.m32 - m.m12 * m.m30;
    f32 A0312 = m.m10 * m.m23 - m.m13 * m.m20;
    f32 A0212 = m.m10 * m.m22 - m.m12 * m.m20;
    f32 A0113 = m.m10 * m.m31 - m.m11 * m.m30;
    f32 A0112 = m.m10 * m.m21 - m.m11 * m.m20;

    f32 det = m.m00 * (m.m11 * A2323 - m.m12 * A1323 + m.m13 * A1223)
    - m.m01 * (m.m10 * A2323 - m.m12 * A0323 + m.m13 * A0223)
    + m.m02 * (m.m10 * A1323 - m.m11 * A0323 + m.m13 * A0123)
    - m.m03 * (m.m10 * A1223 - m.m11 * A0223 + m.m12 * A0123);
    det = 1 / det;

    return
    {
        det *  (m.m11 * A2323 - m.m12 * A1323 + m.m13 * A1223),
        det * -(m.m01 * A2323 - m.m02 * A1323 + m.m03 * A1223),
        det *  (m.m01 * A2313 - m.m02 * A1313 + m.m03 * A1213),
        det * -(m.m01 * A2312 - m.m02 * A1312 + m.m03 * A1212),
        det * -(m.m10 * A2323 - m.m12 * A0323 + m.m13 * A0223),
        det *  (m.m00 * A2323 - m.m02 * A0323 + m.m03 * A0223),
        det * -(m.m00 * A2313 - m.m02 * A0313 + m.m03 * A0213),
        det *  (m.m00 * A2312 - m.m02 * A0312 + m.m03 * A0212),
        det *  (m.m10 * A1323 - m.m11 * A0323 + m.m13 * A0123),
        det * -(m.m00 * A1323 - m.m01 * A0323 + m.m03 * A0123),
        det *  (m.m00 * A1313 - m.m01 * A0313 + m.m03 * A0113),
        det * -(m.m00 * A1312 - m.m01 * A0312 + m.m03 * A0112),
        det * -(m.m10 * A1223 - m.m11 * A0223 + m.m12 * A0123),
        det *  (m.m00 * A1223 - m.m01 * A0223 + m.m02 * A0123),
        det * -(m.m00 * A1213 - m.m01 * A0213 + m.m02 * A0113),
        det *  (m.m00 * A1212 - m.m01 * A0212 + m.m02 * A0112),
    };
};

m4f Identity()
{
    return
    {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
};

m4f ScaleMatrix(const v3f& scale)
{
    return
    {
        scale.x, 0.f, 0.f, 0.f,
        0.f, scale.y, 0.f, 0.f,
        0.f, 0.f, scale.z, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
};

m4f RotationMatrix(const f32& angle, const v3f& axis)
{
    f32 angSin = sinf(angle); f32 angCos = cosf(angle); f32 invCos = 1.f - angCos;
    return
    {
        axis.x * axis.x * invCos + angCos,          axis.y * axis.x * invCos - axis.z * angSin, axis.z * axis.x * invCos + axis.y * angSin, 0.f,
        axis.x * axis.y * invCos + axis.z * angSin, axis.y * axis.y * invCos + angCos,          axis.z * axis.y * invCos - axis.x * angSin, 0.f,
        axis.x * axis.z * invCos - axis.y * angSin, axis.y * axis.z * invCos + axis.x * angSin, axis.z * axis.z * invCos + angCos,          0.f,
        0.f, 0.f, 0.f, 1.f,
    };
}

m4f TranslationMatrix(const v3f& move)
{
    return
    {
        1.f, 0.f, 0.f, move.x,
        0.f, 1.f, 0.f, move.y,
        0.f, 0.f, 1.f, move.z,
        0.f, 0.f, 0.f, 1.f,
    };
};

v3f TransformPosition(const v3f& position, const m4f& transform)
{
    v4f v = {position.x, position.y, position.z, 1.f};
    v = transform * v;
    return {v.x, v.y, v.z};
}

v3f TransformDirection(const v3f& direction, const m4f& transform)
{
    v4f v = {direction.x, direction.y, direction.z, 0.f};
    v = transform * v;
    return {v.x, v.y, v.z};
}

m4f LookAtMatrix(const v3f& center, const v3f& target, const v3f& up)
{
    v3f lookDir = Normalize(center - target);
    v3f lookRight = Normalize(Cross(up, lookDir));
    v3f lookUp = Normalize(Cross(lookDir, lookRight));
    m4f lookRotation =
    {
        lookRight.x, lookRight.y, lookRight.z, 0.f,
        lookUp.x, lookUp.y, lookUp.z, 0.f,
        lookDir.x, lookDir.y, lookDir.z, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
    m4f lookTranslation = TranslationMatrix({-center.x, -center.y, -center.z});
    return lookRotation * lookTranslation;
}

m4f PerspectiveProjectionMatrix(const f32& fovY, const f32& aspectRatio, const f32& nearPlane, const f32& farPlane)
{
    f32 top = tanf(fovY / 2.f) * nearPlane;
    f32 bottom = -top;
    f32 right = top * aspectRatio;
    f32 left = bottom * aspectRatio;
    m4f result =
    {
        (2 * nearPlane) / (right - left), 0, 0, 0,
        0, (2 * nearPlane) / (top - bottom), 0, 0,
        (right + left) / (right - left), (top + bottom) / (top - bottom), -(farPlane + nearPlane) / (farPlane - nearPlane), -1,
        0, 0, -(2 * farPlane * nearPlane) / (farPlane - nearPlane), 0,
    };
    result.m11 *= -1;           // m11 needs to be scaled by -1 to account for coordinate system conversion.
                                // My math library uses LEFT-HANDED, while vulkan uses RIGHT-HANDED.
    
    return Transpose(result);   // TODO(caio): Figure out why this transpose is needed since I'm already
                                // transposing when sending to shaders
}

m4f OrthographicProjectionMatrix(const f32& left, const f32& right, const f32& bottom, const f32& top, const f32& nearPlane, const f32& farPlane)
{
    m4f result =
    {
        2.f / (right - left), 0, 0, 0,
        0, 2.f / (top - bottom), 0, 0,
        0, 0, 2.f / (farPlane - nearPlane), 0,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(farPlane + nearPlane) / (farPlane - nearPlane), 1,
    };
    return result;
}

f32 Lerp(const f32& a, const f32& b, const f32& t)
{
    return a + (b - a) * CLAMP(t, 0, 1);
}

v2f Lerp(const v2f& a, const v2f& b, const f32& t)
{
    return
    {
        Lerp(a.x, b.x, t),
        Lerp(a.y, b.y, t),
    };
}

v3f Lerp(const v3f& a, const v3f& b, const f32& t)
{
    return
    {
        Lerp(a.x, b.x, t),
        Lerp(a.y, b.y, t),
        Lerp(a.z, b.z, t),
    };
}
