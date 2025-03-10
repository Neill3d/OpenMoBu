/*********************************************************************NVMH4****
File:  nv_math.h

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.



Comments:


******************************************************************************/
#ifndef _nv_math_h_
#define _nv_math_h_

#include <assert.h>
#include <math.h>

#ifdef _WIN32
#include <limits>
#else
#include <limits.h>
#endif

#ifdef MACOS
#define sqrtf sqrt
#define sinf sin
#define cosf cos
#define tanf tan
#endif

#include <memory.h>
#include <stdlib.h>
#include <float.h>

namespace nv
{
    typedef float nv_scalar;

#define nv_zero			      nv_scalar(0)
#define nv_zero_5             nv_scalar(0.5)
#define nv_one			      nv_scalar(1.0)
#define nv_two			      nv_scalar(2)
#define nv_half_pi            nv_scalar(3.14159265358979323846264338327950288419716939937510582 * 0.5)
#define nv_quarter_pi         nv_scalar(3.14159265358979323846264338327950288419716939937510582 * 0.25)
#define nv_pi			      nv_scalar(3.14159265358979323846264338327950288419716939937510582)
#define nv_two_pi			  nv_scalar(3.14159265358979323846264338327950288419716939937510582 * 2.0)
#define nv_oo_pi			  nv_one / nv_pi
#define nv_oo_two_pi	      nv_one / nv_two_pi
#define nv_oo_255   	      nv_one / nv_scalar(255)
#define nv_oo_128   	      nv_one / nv_scalar(128)
#define nv_to_rad             nv_pi / nv_scalar(180)
#define nv_to_deg             nv_scalar(180) / nv_pi
#define nv_eps		          nv_scalar(10e-6)
#define nv_double_eps	      nv_scalar(10e-6) * nv_two
#define nv_big_eps            nv_scalar(10e-2)
#define nv_small_eps          nv_scalar(10e-6)
#define nv_sqrthalf           nv_scalar(0.7071067811865475244)

#define nv_scalar_max         nv_scalar(FLT_MAX)
#define nv_scalar_min         nv_scalar(FLT_MIN)

    struct vec2;
    struct vec2t;
    struct vec3;
    struct vec3t;
    struct vec4;
    struct vec4t;
    struct mat3;
    struct mat4;
    struct quat;

    static const nv_scalar array16_id[] = { nv_one, nv_zero, nv_zero, nv_zero,
                                            nv_zero, nv_one, nv_zero, nv_zero,
                                            nv_zero, nv_zero, nv_one, nv_zero,
                                            nv_zero, nv_zero, nv_zero, nv_one };

    static const nv_scalar array16_null[] = { nv_zero, nv_zero, nv_zero, nv_zero,
                                            nv_zero, nv_zero, nv_zero, nv_zero,
                                            nv_zero, nv_zero, nv_zero, nv_zero,
                                            nv_zero, nv_zero, nv_zero, nv_zero };

    static const nv_scalar array16_scale_bias[] = { nv_zero_5, nv_zero,   nv_zero,   nv_zero,
                                             nv_zero,   nv_zero_5, nv_zero,   nv_zero,
                                             nv_zero,   nv_zero,   nv_zero_5, nv_zero,
                                             nv_zero_5, nv_zero_5, nv_zero_5, nv_one };

    static const nv_scalar array9_id[] = { nv_one, nv_zero, nv_zero,
                                            nv_zero, nv_one, nv_zero,
                                            nv_zero, nv_zero, nv_one };


    struct vec2
    {
        vec2() { }
        vec2(nv_scalar x, nv_scalar y) : x(x), y(y) { }
        vec2(const nv_scalar* xy) : x(xy[0]), y(xy[1]) { }
        vec2(const vec2& u) : x(u.x), y(u.y) { }
        vec2(const vec3&);

        bool operator==(const vec2& u) const
        {
            return (u.x == x && u.y == y) ? true : false;
        }

        bool operator!=(const vec2& u) const
        {
            return !(*this == u);
        }

        vec2& operator*=(const nv_scalar& lambda)
        {
            x *= lambda;
            y *= lambda;
            return *this;
        }

        vec2& operator-=(const vec2& u)
        {
            x -= u.x;
            y -= u.y;
            return *this;
        }

        vec2& operator+=(const vec2& u)
        {
            x += u.x;
            y += u.y;
            return *this;
        }

        nv_scalar& operator[](int i)
        {
            return vec_array[i];
        }

        const nv_scalar operator[](int i) const
        {
            return vec_array[i];
        }

        nv_scalar sq_norm() const { return x * x + y * y; }
        nv_scalar norm() const { return sqrtf(sq_norm()); }

        union {
            struct {
                nv_scalar x, y;          // standard names for components
            };
            struct {
                nv_scalar s, t;          // standard names for components
            };
            nv_scalar vec_array[2];     // array access
        };
    };

    inline const vec2 operator+(const vec2& u, const vec2& v)
    {
        return vec2(u.x + v.x, u.y + v.y);
    }

    inline const vec2 operator-(const vec2& u, const vec2& v)
    {
        return vec2(u.x - v.x, u.y - v.y);
    }

    inline const vec2 operator*(const nv_scalar s, const vec2& u)
    {
        return vec2(s * u.x, s * u.y);
    }

    inline const vec2 operator/(const vec2& u, const nv_scalar s)
    {
        return vec2(u.x / s, u.y / s);
    }

    inline const vec2 operator*(const vec2& u, const vec2& v)
    {
        return vec2(u.x * v.x, u.y * v.y);
    }

    struct vec3
    {
        vec3() { }
        vec3(unsigned int ux, unsigned int uy, unsigned int uz) : x((nv_scalar)ux), y((nv_scalar)uy), z((nv_scalar)uz) {}
        vec3(nv_scalar x, nv_scalar y, nv_scalar z) : x(x), y(y), z(z) { }
        vec3(const nv_scalar* xyz) : x(xyz[0]), y(xyz[1]), z(xyz[2]) { }
        vec3(const vec2& u) : x(u.x), y(u.y), z(1.0f) { }
        vec3(const vec3& u) : x(u.x), y(u.y), z(u.z) { }
        vec3(const vec4&);

        bool operator==(const vec3& u) const
        {
            return (u.x == x && u.y == y && u.z == z) ? true : false;
        }

        bool operator!=(const vec3& rhs) const
        {
            return !(*this == rhs);
        }

        // TL
        vec3& operator*=(const mat3& M)
        {
            vec3 dst;
            extern vec3& mult(vec3 & u, const mat3 & M, const vec3 & v);
            mult(dst, M, *this);
            x = dst.x;
            y = dst.y;
            z = dst.z;
            return (*this);
        }

        vec3& operator*=(const nv_scalar& lambda)
        {
            x *= lambda;
            y *= lambda;
            z *= lambda;
            return *this;
        }
        // TL
        vec3& operator*=(const vec3& v)
        {
            x *= v.x;
            y *= v.y;
            z *= v.z;
            return *this;
        }

        vec3 operator - () const
        {
            return vec3(-x, -y, -z);
        }

        vec3& operator-=(const vec3& u)
        {
            x -= u.x;
            y -= u.y;
            z -= u.z;
            return *this;
        }

        vec3& operator+=(const vec3& u)
        {
            x += u.x;
            y += u.y;
            z += u.z;
            return *this;
        }
        vec3& rotateBy(const quat& q);
        nv_scalar normalize();
        void orthogonalize(const vec3& v);
        void orthonormalize(const vec3& v)
        {
            orthogonalize(v); //  just orthogonalize...
            normalize();        //  ...and normalize it
        }

        nv_scalar sq_norm() const { return x * x + y * y + z * z; }
        nv_scalar norm() const { return sqrtf(sq_norm()); }

        nv_scalar& operator[](int i)
        {
            return vec_array[i];
        }

        const nv_scalar operator[](int i) const
        {
            return vec_array[i];
        }

        union {
            struct {
                nv_scalar x, y, z;        // standard names for components
            };
            struct {
                nv_scalar s, t, r;        // standard names for components
            };
            nv_scalar vec_array[3];     // array access
        };
    };

    inline const vec3 operator+(const vec3& u, const vec3& v)
    {
        return vec3(u.x + v.x, u.y + v.y, u.z + v.z);
    }

    inline const vec3 operator-(const vec3& u, const vec3& v)
    {
        return vec3(u.x - v.x, u.y - v.y, u.z - v.z);
    }

    inline const vec3 operator^(const vec3& u, const vec3& v)
    {
        return vec3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
    }

    inline const vec3 operator*(const nv_scalar s, const vec3& u)
    {
        return vec3(s * u.x, s * u.y, s * u.z);
    }

    inline const vec3 operator/(const vec3& u, const nv_scalar s)
    {
        return vec3(u.x / s, u.y / s, u.z / s);
    }

    inline const vec3 operator*(const vec3& u, const vec3& v)
    {
        return vec3(u.x * v.x, u.y * v.y, u.z * v.z);
    }

    inline vec2::vec2(const vec3& u)
    {
        nv_scalar k = 1 / u.z;
        x = k * u.x;
        y = k * u.y;
    }

    struct vec4
    {
        vec4() { }
        vec4(nv_scalar x, nv_scalar y, nv_scalar z, nv_scalar w) : x(x), y(y), z(z), w(w) { }
        vec4(const nv_scalar* xyzw) : x(xyzw[0]), y(xyzw[1]), z(xyzw[2]), w(xyzw[3]) { }
        vec4(const vec3& u) : x(u.x), y(u.y), z(u.z), w(1.0f) { }
        vec4(const vec3& u, const nv_scalar w) : x(u.x), y(u.y), z(u.z), w(w) { }
        vec4(const vec4& u) : x(u.x), y(u.y), z(u.z), w(u.w) { }

        bool operator==(const vec4& u) const
        {
            return (u.x == x && u.y == y && u.z == z && u.w == w) ? true : false;
        }

        bool operator!=(const vec4& rhs) const
        {
            return !(*this == rhs);
        }


        vec4& operator*=(const nv_scalar& lambda)
        {
            x *= lambda;
            y *= lambda;
            z *= lambda;
            w *= lambda;
            return *this;
        }

        vec4& operator-=(const vec4& u)
        {
            x -= u.x;
            y -= u.y;
            z -= u.z;
            w -= u.w;
            return *this;
        }

        vec4& operator+=(const vec4& u)
        {
            x += u.x;
            y += u.y;
            z += u.z;
            w += u.w;
            return *this;
        }

        vec4 operator - () const
        {
            return vec4(-x, -y, -z, -w);
        }

        nv_scalar& operator[](int i)
        {
            return vec_array[i];
        }

        const nv_scalar operator[](int i) const
        {
            return vec_array[i];
        }

        union {
            struct {
                nv_scalar x, y, z, w;          // standard names for components
            };
            struct {
                nv_scalar s, t, r, q;          // standard names for components
            };
            nv_scalar vec_array[4];     // array access
        };
    };

    inline const vec4 operator+(const vec4& u, const vec4& v)
    {
        return vec4(u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w);
    }

    inline const vec4 operator-(const vec4& u, const vec4& v)
    {
        return vec4(u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w);
    }

    inline const vec4 operator*(const nv_scalar s, const vec4& u)
    {
        return vec4(s * u.x, s * u.y, s * u.z, s * u.w);
    }

    inline const vec4 operator/(const vec4& u, const nv_scalar s)
    {
        return vec4(u.x / s, u.y / s, u.z / s, u.w / s);
    }

    inline const vec4 operator*(const vec4& u, const vec4& v)
    {
        return vec4(u.x * v.x, u.y * v.y, u.z * v.z, u.w * v.w);
    }

    inline vec3::vec3(const vec4& u)
    {
        x = u.x;
        y = u.y;
        z = u.z;
    }

    // quaternion
    struct quat;

    /*
        for all the matrices...a<x><y> indicates the element at row x, col y

        For example:
        a01 <-> row 0, col 1
    */

    struct mat3
    {
        mat3();
        mat3(const nv_scalar* array);
        mat3(const mat3& M);
        mat3(const nv_scalar& f0, const nv_scalar& f1, const nv_scalar& f2,
            const nv_scalar& f3, const nv_scalar& f4, const nv_scalar& f5,
            const nv_scalar& f6, const nv_scalar& f7, const nv_scalar& f8)
            : a00(f0), a10(f1), a20(f2),
            a01(f3), a11(f4), a21(f5),
            a02(f6), a12(f7), a22(f8) { }

        const vec3 col(const int i) const
        {
            return vec3(&mat_array[i * 3]);
        }

        const vec3 operator[](int i) const
        {
            return vec3(mat_array[i], mat_array[i + 3], mat_array[i + 6]);
        }

        const nv_scalar& operator()(const int& i, const int& j) const
        {
            return mat_array[j * 3 + i];
        }

        nv_scalar& operator()(const int& i, const int& j)
        {
            return  mat_array[j * 3 + i];
        }

        mat3& operator*=(const nv_scalar& lambda)
        {
            for (int i = 0; i < 9; ++i)
                mat_array[i] *= lambda;
            return *this;
        }
        //TL
        mat3& operator*=(const mat3& M)
        {
            return *this;
        }

        mat3& operator-=(const mat3& M)
        {
            for (int i = 0; i < 9; ++i)
                mat_array[i] -= M.mat_array[i];
            return *this;
        }

        mat3& set_row(int i, const vec3& v)
        {
            mat_array[i] = v.x;
            mat_array[i + 3] = v.y;
            mat_array[i + 6] = v.z;
            return *this;
        }

        mat3& set_col(int i, const vec3& v)
        {
            mat_array[i * 3] = v.x;
            mat_array[i * 3 + 1] = v.y;
            mat_array[i * 3 + 2] = v.z;
            return *this;
        }

        mat3& set_rot(const nv_scalar& theta, const vec3& v);
        mat3& set_rot(const vec3& u, const vec3& v);

        // Matrix norms...
        // Compute || M ||
        //                1
        nv_scalar norm_one();

        // Compute || M ||
        //                +inf
        nv_scalar norm_inf();

        union {
            struct {
                nv_scalar a00, a10, a20;        // standard names for components
                nv_scalar a01, a11, a21;        // standard names for components
                nv_scalar a02, a12, a22;        // standard names for components
            };
            nv_scalar mat_array[9];     // array access
        };
    };

    const vec3 operator*(const mat3&, const vec3&);
    const vec3 operator*(const vec3&, const mat3&);

    //
    // Note : as_...() means that the whole matrix is being modified
    // set_...() only changes the concerned fields of the matrix
    //
    // translate()/scale()/identity()/rotate() : act as OpenGL functions. Example :
    // M.identity()
    // M.translate(t)
    // M.scale(s)
    // drawMyVertex(vtx)
    //
    // is like : newVtx = Midentiry * Mt * Ms * vtx
    //
    struct mat4
    {
        mat4();
        mat4(const nv_scalar* array);
        mat4(const mat4& M);

        mat4(const nv_scalar& f0, const nv_scalar& f1, const nv_scalar& f2, const nv_scalar& f3,
            const nv_scalar& f4, const nv_scalar& f5, const nv_scalar& f6, const nv_scalar& f7,
            const nv_scalar& f8, const nv_scalar& f9, const nv_scalar& f10, const nv_scalar& f11,
            const nv_scalar& f12, const nv_scalar& f13, const nv_scalar& f14, const nv_scalar& f15)
            : a00(f0), a10(f1), a20(f2), a30(f3),
            a01(f4), a11(f5), a21(f6), a31(f7),
            a02(f8), a12(f9), a22(f10), a32(f11),
            a03(f12), a13(f13), a23(f14), a33(f15) { }

        const vec4 col(const int i) const
        {
            return vec4(&mat_array[i * 4]);
        }

        const vec4 operator[](const int& i) const
        {
            return vec4(mat_array[i], mat_array[i + 4], mat_array[i + 8], mat_array[i + 12]);
        }

        const nv_scalar& operator()(const int& i, const int& j) const
        {
            return mat_array[j * 4 + i];
        }

        nv_scalar& operator()(const int& i, const int& j)
        {
            return  mat_array[j * 4 + i];
        }

        mat4& set_col(int i, const vec4& v)
        {
            mat_array[i * 4] = v.x;
            mat_array[i * 4 + 1] = v.y;
            mat_array[i * 4 + 2] = v.z;
            mat_array[i * 4 + 3] = v.w;
            return *this;
        }

        mat4& set_row(int i, const vec4& v)
        {
            mat_array[i] = v.x;
            mat_array[i + 4] = v.y;
            mat_array[i + 8] = v.z;
            mat_array[i + 12] = v.w;
            return *this;
        }

        mat3& get_rot(mat3& M) const;
        quat& get_rot(quat& q) const;
        mat4& set_rot(const quat& q);
        mat4& set_rot(const mat3& M);
        mat4& set_rot(const nv_scalar& theta, const vec3& v);
        mat4& set_rot(const vec3& u, const vec3& v);

        mat4& as_rot(const quat& rotation)
        {
            a30 = a31 = a32 = 0.0; a33 = 1.0;
            a03 = a13 = a23 = 0.0;
            set_rot(rotation);
            return *this;
        }
        mat4& as_rot(const mat3& M)
        {
            a30 = a31 = a32 = 0.0; a33 = 1.0;
            a03 = a13 = a23 = 0.0;
            set_rot(M);
            return *this;
        }
        mat4& as_rot(const nv_scalar& theta, const vec3& v)
        {
            set_rot(theta, v);
            a30 = a31 = a32 = 0.0; a33 = 1.0;
            a03 = a13 = a23 = 0.0;
            return *this;
        }
        mat4& as_rot(const vec3& u, const vec3& v)
        {
            a30 = a31 = a32 = 0.0; a33 = 1.0;
            a03 = a13 = a23 = 0.0;
            set_rot(u, v);
            return *this;
        }

        mat4& set_scale(const vec3& s);
        vec3& get_scale(vec3& s) const;
        mat4& as_scale(const vec3& s);
        mat4& as_scale(const nv_scalar& s);
        mat4& set_translation(const vec3& t);
        vec3& get_translation(vec3& t) const;
        mat4& as_translation(const vec3& t);

        mat4 operator*(const mat4&) const;
        //TL
        mat4& operator*=(const mat4& M)
        {
            mat4 Mtmp;
            mat4& mult(mat4 & C, const mat4 & A, const mat4 & B);
            *this = mult(Mtmp, *this, M);
            return *this;
        }

        //TL: some additional methods that look like OpenGL...
        // they behave the same as the OpenGL matrix system
        // But: using vec3 class; rotation is in Radians
        // TODO: optimize
        mat4& identity()
        {
            memcpy(mat_array, array16_id, sizeof(nv_scalar) * 16);
            return *this;
        }
        mat4& translate(vec3 t)
        {
            *this *= mat4().as_translation(t);
            return *this;
        }
        mat4& scale(vec3 s)
        {
            *this *= mat4().as_scale(s);
            return *this;
        }
        mat4& scale(nv_scalar s)
        {
            *this *= mat4().as_scale(s);
            return *this;
        }
        mat4& rotate(const nv_scalar& theta, const vec3& v)
        {
            *this *= mat4().as_rot(theta, v);
            return *this;
        }
        mat4& rotate(quat& rotation)
        {
            *this *= mat4().identity().set_rot(rotation);
            return *this;
        }

        union {
            struct {
                nv_scalar a00, a10, a20, a30;   // standard names for components
                nv_scalar a01, a11, a21, a31;   // standard names for components
                nv_scalar a02, a12, a22, a32;   // standard names for components
                nv_scalar a03, a13, a23, a33;   // standard names for components
            };
            struct {
                nv_scalar _11, _12, _13, _14;   // standard names for components
                nv_scalar _21, _22, _23, _24;   // standard names for components
                nv_scalar _31, _32, _33, _34;   // standard names for components
                nv_scalar _41, _42, _43, _44;   // standard names for components
            };
            union {
                struct {
                    nv_scalar b00, b10, b20, p; // standard names for components
                    nv_scalar b01, b11, b21, q; // standard names for components
                    nv_scalar b02, b12, b22, r; // standard names for components
                    nv_scalar x, y, z, w;       // standard names for components
                };
            };
            nv_scalar mat_array[16];     // array access
        };
    };

    const vec4 operator*(const mat4&, const vec4&);
    const vec4 operator*(const vec4&, const mat4&);

    // quaternion
    struct quat {
    public:
        quat();
        quat(nv_scalar* q);
        quat(nv_scalar x, nv_scalar y, nv_scalar z, nv_scalar w);
        quat(const quat& quat);
        quat(const vec3& axis, nv_scalar angle);
        quat(const vec3& eulerXYZ); // From Euler
        quat(const mat3& rot);
        quat& operator=(const quat& quat);
        quat operator-()
        {
            return quat(-x, -y, -z, -w);
        }
        quat Inverse();
        quat Conjugate();
        void Normalize();
        void FromMatrix(const mat3& mat);
        void ToMatrix(mat3& mat) const;
        void ToEulerXYZ(vec3& r);
        void FromEulerXYZ(vec3 r);
        quat& operator*=(const quat& q);
        static const quat Identity;
        nv_scalar& operator[](int i) { return comp[i]; }
        const nv_scalar operator[](int i) const { return comp[i]; }
        union {
            struct {
                nv_scalar x, y, z, w;
            };
            nv_scalar comp[4];
        };
    };
    const quat operator*(const quat&, const quat&);
    extern quat& add_quats(quat& p, const quat& q1, const quat& q2);
    extern nv_scalar dot(const quat& p, const quat& q);
    extern quat& dot(nv_scalar s, const quat& p, const quat& q);
    extern quat& slerp_quats(quat& p, nv_scalar s, const quat& q1, const quat& q2);
    extern quat& axis_to_quat(quat& q, const vec3& a, const nv_scalar phi);
    extern mat3& quat_2_mat(mat3& M, const quat& q);
    extern quat& mat_2_quat(quat& q, const mat3& M);

    // constant algebraic values

    static const vec2      vec2_null(nv_zero, nv_zero);
    static const vec4      vec4_one(nv_one, nv_one, nv_one, nv_one);
    static const vec3      vec3_one(nv_one, nv_one, nv_one);
    static const vec3      vec3_null(nv_zero, nv_zero, nv_zero);
    static const vec3      vec3_x(nv_one, nv_zero, nv_zero);
    static const vec3      vec3_y(nv_zero, nv_one, nv_zero);
    static const vec3      vec3_z(nv_zero, nv_zero, nv_one);
    static const vec3      vec3_neg_x(-nv_one, nv_zero, nv_zero);
    static const vec3      vec3_neg_y(nv_zero, -nv_one, nv_zero);
    static const vec3      vec3_neg_z(nv_zero, nv_zero, -nv_one);
    static const vec4      vec4_null(nv_zero, nv_zero, nv_zero, nv_zero);
    static const vec4      vec4_x(nv_one, nv_zero, nv_zero, nv_zero);
    static const vec4      vec4_neg_x(-nv_one, nv_zero, nv_zero, nv_zero);
    static const vec4      vec4_y(nv_zero, nv_one, nv_zero, nv_zero);
    static const vec4      vec4_neg_y(nv_zero, -nv_one, nv_zero, nv_zero);
    static const vec4      vec4_z(nv_zero, nv_zero, nv_one, nv_zero);
    static const vec4      vec4_neg_z(nv_zero, nv_zero, -nv_one, nv_zero);
    static const vec4      vec4_w(nv_zero, nv_zero, nv_zero, nv_one);
    static const vec4      vec4_neg_w(nv_zero, nv_zero, nv_zero, -nv_one);
    static const quat      quat_id(nv_zero, nv_zero, nv_zero, nv_one);
    static const mat4      mat4_id(array16_id);
    static const mat3      mat3_id(array9_id);
    static const mat4      mat4_null(array16_null);
    static const mat4      mat4_scale_bias(array16_scale_bias);

    // normalizes a vector and return a reference of itself
    extern vec2& normalize(vec2& u);
    extern vec3& normalize(vec3& u);
    extern vec4& normalize(vec4& u);

    // Computes the squared magnitude
    inline nv_scalar nv_sq_norm(const vec3& n)
    {
        return n.x * n.x + n.y * n.y + n.z * n.z;
    }

    inline nv_scalar nv_sq_norm(const vec4& n)
    {
        return n.x * n.x + n.y * n.y + n.z * n.z + n.w * n.w;
    }

    // Computes the magnitude
    inline nv_scalar nv_norm(const vec3& n)
    {
        return sqrtf(nv_sq_norm(n));
    }

    inline nv_scalar nv_norm(const vec4& n)
    {
        return sqrtf(nv_sq_norm(n));
    }


    // computes the cross product ( v cross w) and stores the result in u
    // i.e.     u = v cross w
    extern vec3& cross(vec3& u, const vec3& v, const vec3& w);

    // computes the dot product ( v dot w) and stores the result in u
    // i.e.     u = v dot w
    extern nv_scalar& dot(nv_scalar& u, const vec3& v, const vec3& w);
    extern nv_scalar dot(const vec3& v, const vec3& w);
    extern nv_scalar& dot(nv_scalar& u, const vec4& v, const vec4& w);
    extern nv_scalar dot(const vec4& v, const vec4& w);
    extern nv_scalar& dot(nv_scalar& u, const vec3& v, const vec4& w);
    extern nv_scalar dot(const vec3& v, const vec4& w);
    extern nv_scalar& dot(nv_scalar& u, const vec4& v, const vec3& w);
    extern nv_scalar dot(const vec4& v, const vec3& w);

    // compute the reflected vector R of L w.r.t N - vectors need to be 
    // normalized
    //
    //                R     N     L
    //                  _       _
    //                 |\   ^   /|
    //                   \  |  /
    //                    \ | /
    //                     \|/
    //                      +
    extern vec3& reflect(vec3& r, const vec3& n, const vec3& l);

    // Computes u = v * lambda + u
    extern vec3& madd(vec3& u, const vec3& v, const nv_scalar& lambda);
    // Computes u = v * lambda
    extern vec3& mult(vec3& u, const vec3& v, const nv_scalar& lambda);
    // Computes u = v * w
    extern vec3& mult(vec3& u, const vec3& v, const vec3& w);
    // Computes u = v + w
    extern vec3& add(vec3& u, const vec3& v, const vec3& w);
    // Computes u = v - w
    extern vec3& sub(vec3& u, const vec3& v, const vec3& w);

    // Computes u = u * s
    extern vec2& scale(vec2& u, const nv_scalar s);
    extern vec3& scale(vec3& u, const nv_scalar s);
    extern vec4& scale(vec4& u, const nv_scalar s);

    // Computes u = M * v
    extern vec3& mult(vec3& u, const mat3& M, const vec3& v);
    extern vec4& mult(vec4& u, const mat4& M, const vec4& v);

    // Computes u = v * M
    extern vec3& mult(vec3& u, const vec3& v, const mat3& M);
    extern vec4& mult(vec4& u, const vec4& v, const mat4& M);

    // Computes u = M(4x4) * v and divides by w
    extern vec3& mult_pos(vec3& u, const mat4& M, const vec3& v);
    // Computes u = M(4x4) * v
    extern vec3& mult_dir(vec3& u, const mat4& M, const vec3& v);
    // Computes u = M(4x4) * v and does not divide by w (assumed to be 1)
    extern vec3& mult(vec3& u, const mat4& M, const vec3& v);

    // Computes u = v * M(4x4) and divides by w
    extern vec3& mult_pos(vec3& u, const vec3& v, const mat4& M);
    // Computes u = v * M(4x4)
    extern vec3& mult_dir(vec3& u, const vec3& v, const mat4& M);
    // Computes u = v * M(4x4) and does not divide by w (assumed to be 1)
    extern vec3& mult(vec3& u, const vec3& v, const mat4& M);

    // Computes A += B
    extern mat4& add(mat4& A, const mat4& B);
    extern mat3& add(mat3& A, const mat3& B);

    // Computes C = A + B
    extern mat4& add(mat4& C, const mat4& A, const mat4& B);
    extern mat3& add(mat3& C, const mat3& A, const mat3& B);

    // Computes C = A * B
    extern mat4& mult(mat4& C, const mat4& A, const mat4& B);
    extern mat3& mult(mat3& C, const mat3& A, const mat3& B);

    // Compute M = -M
    extern mat4& negate(mat4& M);
    extern mat3& negate(mat3& M);

    // Computes B = Transpose(A)
    //       T
    //  B = A
    extern mat3& transpose(mat3& B, const mat3& A);
    extern mat4& transpose(mat4& B, const mat4& A);

    // Computes B = Transpose(B)
    //       T
    //  B = B
    extern mat3& transpose(mat3& B);
    extern mat4& transpose(mat4& B);

    // Computes B = inverse(A)
    //       -1
    //  B = A
    extern mat4& invert(mat4& B, const mat4& A);
    extern mat3& invert(mat3& B, const mat3& A);

    // Computes B = inverse(A)
    //                                       T  T
    //                   (R t)             (R -R t)
    // assuming that A = (0 1) so that B = (0    1)
    //  B = A
    extern mat4& invert_rot_trans(mat4& B, const mat4& A);

    extern mat4& look_at(mat4& M, const vec3& eye, const vec3& center, const vec3& up);
    extern mat4& frustum(mat4& M, const nv_scalar l, const nv_scalar r, const nv_scalar b,
        const nv_scalar t, const nv_scalar n, const nv_scalar f);

    extern mat4& perspective(mat4& M, const nv_scalar fovy, const nv_scalar aspect, const nv_scalar n, const nv_scalar f);
    extern mat4& ortho(mat4& M, const nv_scalar left,
        const nv_scalar right,
        const nv_scalar bottom,
        const nv_scalar top,
        const nv_scalar n,
        const nv_scalar f);

    /* Decompose Affine Matrix
     *    A = TQS, where
     * A is the affine transform
     * T is the translation vector
     * Q is the rotation (quaternion)
     * S is the scale vector
     * f is the sign of the determinant
    */
    extern void decomp_affine(const mat4& A, vec3& T, quat& Q, quat& U, vec3& S, nv_scalar& f);
    // quaternion
    extern quat& normalize(quat& p);
    extern quat& conj(quat& p);
    extern quat& conj(quat& p, const quat& q);
    extern quat& add_quats(quat& p, const quat& q1, const quat& q2);
    extern quat& axis_to_quat(quat& q, const vec3& a, const nv_scalar phi);
    extern mat3& quat_2_mat(mat3& M, const quat& q);
    extern quat& mat_2_quat(quat& q, const mat3& M);
    extern quat& mat_2_quat(quat& q, const mat4& M);

    // surface properties
    extern mat3& tangent_basis(mat3& basis, const vec3& v0, const vec3& v1, const vec3& v2, const vec2& t0, const vec2& t1, const vec2& t2, const vec3& n);

    // linear interpolation
#ifdef USEOPTIX
#pragma message("**WARNING** nv_math.h : Canceling the lerp() function here : already declared in OptiX")
#else
    inline nv_scalar lerp(nv_scalar t, nv_scalar a, nv_scalar b)
    {
        return a * (nv_one - t) + t * b;
    }

    inline vec3& lerp(vec3& w, const nv_scalar& t, const vec3& u, const vec3& v)
    {
        w.x = lerp(t, u.x, v.x); w.y = lerp(t, u.y, v.y); w.z = lerp(t, u.z, v.z); return w;
    }

    inline vec4& lerp(vec4& w, const nv_scalar& t, const vec4& u, const vec4& v)
    {
        w.x = lerp(t, u.x, v.x); w.y = lerp(t, u.y, v.y); w.z = lerp(t, u.z, v.z); w.w = lerp(t, u.w, v.w); return w;
    }
#endif

    // utilities
    inline nv_scalar nv_min(const nv_scalar& lambda, const nv_scalar& n)
    {
        return (lambda < n) ? lambda : n;
    }

    inline nv_scalar nv_max(const nv_scalar& lambda, const nv_scalar& n)
    {
        return (lambda > n) ? lambda : n;
    }

    inline nv_scalar nv_clamp(nv_scalar u, const nv_scalar min, const nv_scalar max)
    {
        u = (u < min) ? min : u; u = (u > max) ? max : u; return u;
    }

    extern nv_scalar nv_random();

    extern quat& trackball(quat& q, vec2& pt1, vec2& pt2, nv_scalar trackballsize);

    extern vec3& cube_map_normal(int i, int x, int y, int cubesize, vec3& v);

    // Componentwise maximum and minium 
    inline void nv_max(vec3& vOut, const vec3& vFirst, const vec3& vSecond)
    {
        vOut.x = nv_max(vFirst.x, vSecond.x);
        vOut.y = nv_max(vFirst.y, vSecond.y);
        vOut.z = nv_max(vFirst.z, vSecond.z);
    }

    inline void nv_max(vec4& vOut, const vec4& vFirst, const vec4& vSecond)
    {
        vOut.x = nv_max(vFirst.x, vSecond.x);
        vOut.y = nv_max(vFirst.y, vSecond.y);
        vOut.z = nv_max(vFirst.z, vSecond.z);
        vOut.w = nv_max(vFirst.w, vSecond.w);
    }

    inline void nv_min(vec3& vOut, const vec3& vFirst, const vec3& vSecond)
    {
        vOut.x = nv_min(vFirst.x, vSecond.x);
        vOut.y = nv_min(vFirst.y, vSecond.y);
        vOut.z = nv_min(vFirst.z, vSecond.z);
    }

    inline void nv_min(vec4& vOut, const vec4& vFirst, const vec4& vSecond)
    {
        vOut.x = nv_min(vFirst.x, vSecond.x);
        vOut.y = nv_min(vFirst.y, vSecond.y);
        vOut.z = nv_min(vFirst.z, vSecond.z);
        vOut.w = nv_min(vFirst.w, vSecond.w);
    }

    // geometry
    // computes the area of a triangle
    extern nv_scalar nv_area(const vec3& v1, const vec3& v2, const vec3& v3);
    // computes the perimeter of a triangle
    extern nv_scalar nv_perimeter(const vec3& v1, const vec3& v2, const vec3& v3);
    // find the inscribed circle
    extern nv_scalar nv_find_in_circle(vec3& center, const vec3& v1, const vec3& v2, const vec3& v3);
    // find the circumscribed circle
    extern nv_scalar nv_find_circ_circle(vec3& center, const vec3& v1, const vec3& v2, const vec3& v3);

    // fast cosine functions
    extern nv_scalar fast_cos(const nv_scalar x);
    extern nv_scalar ffast_cos(const nv_scalar x);

    // determinant
    nv_scalar det(const mat3& A);

    extern void nv_is_valid(const vec3& v);
    extern void nv_is_valid(nv_scalar lambda);

    // TL : v1 and v2 MUST be normalized. Not done inot this to avoid redundant work...
    extern nv_scalar getAngle(const vec3& v1, const vec3& v2);
    extern vec3& rotateBy(vec3& dst, const vec3& src, const quat& q);

}; // NVMath namespace

#endif //_nv_math_h_
