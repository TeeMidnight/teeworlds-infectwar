/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef BASE_VMATH_H
#define BASE_VMATH_H

#include <cmath>
#include <cstdint>

// ------------------------------------

template<typename T>
class vector2_base
{
public:
	union
	{
		T x, u;
	};
	union
	{
		T y, v;
	};

	constexpr vector2_base() = default;
	constexpr vector2_base(T nx, T ny) :
		x(nx), y(ny)
	{
	}

	vector2_base operator-() const { return vector2_base(-x, -y); }
	vector2_base operator-(const vector2_base &vec) const { return vector2_base(x - vec.x, y - vec.y); }
	vector2_base operator+(const vector2_base &vec) const { return vector2_base(x + vec.x, y + vec.y); }
	vector2_base operator*(const T rhs) const { return vector2_base(x * rhs, y * rhs); }
	vector2_base operator*(const vector2_base &vec) const { return vector2_base(x * vec.x, y * vec.y); }
	vector2_base operator/(const T rhs) const { return vector2_base(x / rhs, y / rhs); }
	vector2_base operator/(const vector2_base &vec) const { return vector2_base(x / vec.x, y / vec.y); }

	const vector2_base &operator+=(const vector2_base &vec)
	{
		x += vec.x;
		y += vec.y;
		return *this;
	}
	const vector2_base &operator-=(const vector2_base &vec)
	{
		x -= vec.x;
		y -= vec.y;
		return *this;
	}
	const vector2_base &operator*=(const T rhs)
	{
		x *= rhs;
		y *= rhs;
		return *this;
	}
	const vector2_base &operator*=(const vector2_base &vec)
	{
		x *= vec.x;
		y *= vec.y;
		return *this;
	}
	const vector2_base &operator/=(const T rhs)
	{
		x /= rhs;
		y /= rhs;
		return *this;
	}
	const vector2_base &operator/=(const vector2_base &vec)
	{
		x /= vec.x;
		y /= vec.y;
		return *this;
	}

	bool operator==(const vector2_base &vec) const { return x == vec.x && y == vec.y; } //TODO: do this with an eps instead
	bool operator!=(const vector2_base &vec) const { return x != vec.x || y != vec.y; }

	T &operator[](const int index) { return index ? y : x; }
};

template<typename T>
inline T distance(const vector2_base<T> a, const vector2_base<T> &b)
{
	return length(a - b);
}

template<typename T>
constexpr inline T dot(const vector2_base<T> a, const vector2_base<T> &b)
{
	return a.x * b.x + a.y * b.y;
}

inline float length(const vector2_base<float> &a)
{
	return std::sqrt(dot(a, a));
}

inline float length(const vector2_base<int> &a)
{
	return std::sqrt(dot(a, a));
}

inline float length_squared(const vector2_base<float> &a)
{
	return dot(a, a);
}

template<typename T>
constexpr inline vector2_base<T> normalize_pre_length(const vector2_base<T> &v, T len)
{
	if(len == 0)
		return vector2_base<T>();
	return vector2_base<T>(v.x / len, v.y / len);
}

inline vector2_base<float> normalize(const vector2_base<float> &v)
{
	float divisor = length(v);
	if(divisor == 0.0f)
		return vector2_base<float>(0.0f, 0.0f);
	float l = (float)(1.0f / divisor);
	return vector2_base<float>(v.x * l, v.y * l);
}

inline vector2_base<float> direction(float angle)
{
	return vector2_base<float>(std::cos(angle), std::sin(angle));
}

typedef vector2_base<float> vec2;
typedef vector2_base<bool> bvec2;
typedef vector2_base<int> ivec2;

template<typename T>
constexpr inline bool closest_point_on_line(vector2_base<T> line_pointA, vector2_base<T> line_pointB, vector2_base<T> target_point, vector2_base<T> &out_pos)
{
	vector2_base<T> AB = line_pointB - line_pointA;
	T SquaredMagnitudeAB = dot(AB, AB);
	if(SquaredMagnitudeAB > 0)
	{
		vector2_base<T> AP = target_point - line_pointA;
		T APdotAB = dot(AP, AB);
		T t = APdotAB / SquaredMagnitudeAB;
		out_pos = line_pointA + AB * clamp(t, (T)0, (T)1);
		return true;
	}
	else
		return false;
}

template<typename T>
constexpr inline vector2_base<T> closest_point_on_line(vector2_base<T> line_point0, vector2_base<T> line_point1, vector2_base<T> target_point)
{
	vector2_base<T> c = target_point - line_point0;
	vector2_base<T> v = (line_point1 - line_point0);
	v = normalize(v);
	T d = length(line_point0-line_point1);
	T t = dot(v, c)/d;
	return mix(line_point0, line_point1, clamp(t, (T)0, (T)1));
	/*
	if (t < 0) t = 0;
	if (t > 1.0f) return 1.0f;
	return t;*/
}

// ------------------------------------
template<typename T>
class vector3_base
{
public:
	union
	{
		T x, r, h, u;
	};
	union
	{
		T y, g, s, v;
	};
	union
	{
		T z, b, l, w;
	};

	constexpr vector3_base() = default;
	constexpr vector3_base(T nx, T ny, T nz) :
		x(nx), y(ny), z(nz)
	{
	}

	vector3_base operator-(const vector3_base &vec) const { return vector3_base(x - vec.x, y - vec.y, z - vec.z); }
	vector3_base operator-() const { return vector3_base(-x, -y, -z); }
	vector3_base operator+(const vector3_base &vec) const { return vector3_base(x + vec.x, y + vec.y, z + vec.z); }
	vector3_base operator*(const T rhs) const { return vector3_base(x * rhs, y * rhs, z * rhs); }
	vector3_base operator*(const vector3_base &vec) const { return vector3_base(x * vec.x, y * vec.y, z * vec.z); }
	vector3_base operator/(const T rhs) const { return vector3_base(x / rhs, y / rhs, z / rhs); }
	vector3_base operator/(const vector3_base &vec) const { return vector3_base(x / vec.x, y / vec.y, z / vec.z); }

	const vector3_base &operator+=(const vector3_base &vec)
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
		return *this;
	}
	const vector3_base &operator-=(const vector3_base &vec)
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		return *this;
	}
	const vector3_base &operator*=(const T rhs)
	{
		x *= rhs;
		y *= rhs;
		z *= rhs;
		return *this;
	}
	const vector3_base &operator*=(const vector3_base &vec)
	{
		x *= vec.x;
		y *= vec.y;
		z *= vec.z;
		return *this;
	}
	const vector3_base &operator/=(const T rhs)
	{
		x /= rhs;
		y /= rhs;
		z /= rhs;
		return *this;
	}
	const vector3_base &operator/=(const vector3_base &vec)
	{
		x /= vec.x;
		y /= vec.y;
		z /= vec.z;
		return *this;
	}

	bool operator==(const vector3_base &vec) const { return x == vec.x && y == vec.y && z == vec.z; } //TODO: do this with an eps instead
	bool operator!=(const vector3_base &vec) const { return x != vec.x || y != vec.y || z != vec.z; }
};

template<typename T>
inline T distance(const vector3_base<T> &a, const vector3_base<T> &b)
{
	return length(a - b);
}

template<typename T>
constexpr inline T dot(const vector3_base<T> &a, const vector3_base<T> &b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

template<typename T>
constexpr inline vector3_base<T> cross(const vector3_base<T> &a, const vector3_base<T> &b)
{
	return vector3_base<T>(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}

//
inline float length(const vector3_base<float> &a)
{
	return std::sqrt(dot(a, a));
}

inline vector3_base<float> normalize(const vector3_base<float> &v)
{
	float divisor = length(v);
	if(divisor == 0.0f)
		return vector3_base<float>(0.0f, 0.0f, 0.0f);
	float l = (float)(1.0f / divisor);
	return vector3_base<float>(v.x * l, v.y * l, v.z * l);
}

typedef vector3_base<float> vec3;
typedef vector3_base<bool> bvec3;
typedef vector3_base<int> ivec3;

// ------------------------------------

template<typename T>
class vector4_base
{
public:
	union
	{
		T x, r, h;
	};
	union
	{
		T y, g, s;
	};
	union
	{
		T z, b, l;
	};
	union
	{
		T w, a;
	};

	constexpr vector4_base() = default;
	constexpr vector4_base(T nx, T ny, T nz, T nw) :
		x(nx), y(ny), z(nz), w(nw)
	{
	}

	vector4_base operator+(const vector4_base &vec) const { return vector4_base(x + vec.x, y + vec.y, z + vec.z, w + vec.w); }
	vector4_base operator-(const vector4_base &vec) const { return vector4_base(x - vec.x, y - vec.y, z - vec.z, w - vec.w); }
	vector4_base operator-() const { return vector4_base(-x, -y, -z, -w); }
	vector4_base operator*(const vector4_base &vec) const { return vector4_base(x * vec.x, y * vec.y, z * vec.z, w * vec.w); }
	vector4_base operator*(const T rhs) const { return vector4_base(x * rhs, y * rhs, z * rhs, w * rhs); }
	vector4_base operator/(const vector4_base &vec) const { return vector4_base(x / vec.x, y / vec.y, z / vec.z, w / vec.w); }
	vector4_base operator/(const T vec) const { return vector4_base(x / vec, y / vec, z / vec, w / vec); }

	const vector4_base &operator+=(const vector4_base &vec)
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
		w += vec.w;
		return *this;
	}
	const vector4_base &operator-=(const vector4_base &vec)
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		w -= vec.w;
		return *this;
	}
	const vector4_base &operator*=(const T rhs)
	{
		x *= rhs;
		y *= rhs;
		z *= rhs;
		w *= rhs;
		return *this;
	}
	const vector4_base &operator*=(const vector4_base &vec)
	{
		x *= vec.x;
		y *= vec.y;
		z *= vec.z;
		w *= vec.w;
		return *this;
	}
	const vector4_base &operator/=(const T rhs)
	{
		x /= rhs;
		y /= rhs;
		z /= rhs;
		w /= rhs;
		return *this;
	}
	const vector4_base &operator/=(const vector4_base &vec)
	{
		x /= vec.x;
		y /= vec.y;
		z /= vec.z;
		w /= vec.w;
		return *this;
	}

	bool operator==(const vector4_base &vec) const { return x == vec.x && y == vec.y && z == vec.z && w == vec.w; } //TODO: do this with an eps instead
	bool operator!=(const vector4_base &vec) const { return x != vec.x || y != vec.y || z != vec.z || w != vec.w; }
};

typedef vector4_base<float> vec4;
typedef vector4_base<bool> bvec4;
typedef vector4_base<int> ivec4;
typedef vector4_base<uint8_t> ubvec4;

#endif
