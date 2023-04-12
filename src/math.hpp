#pragma once
#include <stdint.h>
#include <math.h>
#include <float.h>

typedef uint8_t     u8;
typedef int         i32;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef float       f32;
typedef double      f64;

#define MAX_U8  (0xFF)
#define MAX_U16 (0xFFFF)
#define MAX_U32 (0xFFFFFFFFUL)
#define MAX_U64 (0xFFFFFFFFFFFFFFFFULL)

#define EPSILON_F32 FLT_EPSILON
#define EPSILON_F64 DBL_EPSILON

#undef MIN
#undef MAX
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define CLAMP(V, A, B) (MAX((A), MIN((V), (B))))
#define CLAMP_CEIL(V, A) MIN(V, A)
#define CLAMP_FLOOR(V, A) MAX(V, A)
#define ABS(V) ((V) < 0 ? -(V) : (V))

// ========================================================
// [MATH]
// Math defines
#define PI 3.14159265358979323846

#define TO_DEG(X) (X * (180.0 / PI))
#define TO_RAD(X) (X * (PI / 180.0))
// Vectors
// Vector2f (f32: x,y | u,v)
struct v2f
{
    union
    {
        struct
        {
            union { f32 x; f32 u; };
            union { f32 y; f32 v; };
        };
        f32 data[2];
    };
};
bool operator==(const v2f& a, const v2f& b);
v2f operator+(const v2f& a, const v2f& b);
v2f operator-(const v2f& a, const v2f& b);
v2f operator*(const v2f& a, const v2f& b);
v2f operator*(const v2f& a, const f32& b);
v2f operator*(const f32& a, const v2f& b);

f32 Dot(const v2f& a, const v2f& b);
f32 Cross(const v2f& a, const v2f& b);
f32 Len2(const v2f& v);
f32 Len(const v2f& v);
v2f Normalize(const v2f& v);
f32 AngleBetween(const v2f& a, const v2f& b);

// Vector2i (i32: x,y)
struct v2i
{
    union
    {
        struct
        {
            i32 x; i32 y;
        };
        i32 data[2];
    };
};
bool operator==(const v2i& a, const v2i& b);
v2i operator+(const v2i& a, const v2i& b);
v2i operator-(const v2i& a, const v2i& b);

// Vector3f (f32: x,y,z | r,g,b)
struct v3f
{
    union
    {
        struct
        {
            union { f32 x; f32 r; };
            union { f32 y; f32 g; };
            union { f32 z; f32 b; };
        };
        f32 data[3];
    };
};
bool operator==(const v3f& a, const v3f& b);
v3f operator+(const v3f& a, const v3f& b);
v3f operator-(const v3f& a, const v3f& b);
v3f operator*(const v3f& a, const v3f& b);
v3f operator*(const v3f& a, const f32& b);
v3f operator*(const f32& a, const v3f& b);
f32 Dot(const v3f& a, const v3f& b);
v3f Cross(const v3f& a, const v3f& b);
f32 Len2(const v3f& v);
f32 Len(const v3f& v);
v3f Normalize(const v3f& v);

// Vector4f (f32: x,y,z,w | r,g,b,a)
struct v4f
{
    union
    {
        struct
        {
            union { f32 x; f32 r; };
            union { f32 y; f32 g; };
            union { f32 z; f32 b; };
            union { f32 w; f32 a; };
        };
        f32 data[4];
    };
};
bool operator==(const v4f& a, const v4f& b);
v4f operator+(const v4f& a, const v4f& b);
v4f operator-(const v4f& a, const v4f& b);
v4f operator*(const v4f& a, const v4f& b);
v4f operator*(const v4f& a, const f32& b);
v4f operator*(const f32& a, const v4f& b);

f32 Dot(const v4f& a, const v4f& b);
f32 Len2(const v4f& v);
f32 Len(const v4f& v);
v4f Normalize(const v4f& v);
v4f v4f_AsDirection(const v3f& v);
v4f v4f_AsPosition(const v3f& v);
v3f v3f_As(const v4f& v);

// TODO(caio)#MATH: Implement quaternion type

// Matrix4f (f32, row-major)    // TODO(caio)#MATH: Test and profile row/column major perf
struct m4f
{
    union
    {
        struct
        {
            f32 m00 = 0; f32 m01 = 0; f32 m02 = 0; f32 m03 = 0;
            f32 m10 = 0; f32 m11 = 0; f32 m12 = 0; f32 m13 = 0;
            f32 m20 = 0; f32 m21 = 0; f32 m22 = 0; f32 m23 = 0;
            f32 m30 = 0; f32 m31 = 0; f32 m32 = 0; f32 m33 = 0;
        };
        f32 data[16];
    };
};
bool operator==(const m4f& a, const m4f& b);
m4f operator+(const m4f& a, const m4f& b);
m4f operator-(const m4f& a, const m4f& b);
m4f operator*(const m4f& a, const m4f& b);
m4f operator*(const f32& a, const m4f& b);
v4f operator*(const m4f& a, const v4f& v);

f32 Determinant(const m4f& m);
m4f Transpose(const m4f& m);
m4f Inverse(const m4f& m);

m4f Identity();
m4f ScaleMatrix(const v3f& scale);
m4f RotationMatrix(const f32& angle, const v3f& axis);
m4f TranslationMatrix(const v3f& move);

v3f TransformPosition(const v3f& position, const m4f& transform);
v3f TransformDirection(const v3f& direction, const m4f& transform);

m4f LookAtMatrix(const v3f& center, const v3f& target, const v3f& up);
m4f PerspectiveProjectionMatrix(const f32& fovY, const f32& aspectRatio, const f32& nearPlane, const f32& farPlane);
m4f OrthographicProjectionMatrix(const f32& left, const f32& right, const f32& bottom, const f32& top, const f32& nearPlane, const f32& farPlane);

m4f VkViewMatrix(v3f center, v3f target, v3f up);
m4f VkPerspectiveProjectionMatrix(f32 fovY, f32 aspect, f32 nearPlane, f32 farPlane);

// Math utilities
f32 Lerp(const f32& a, const f32& b, const f32& t);
v2f Lerp(const v2f& a, const v2f& b, const f32& t);
v3f Lerp(const v3f& a, const v3f& b, const f32& t);
