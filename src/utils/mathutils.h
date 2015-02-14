#pragma once

#include <type_traits>
#include <cstdint>
#include <cstdlib>

namespace foliage {
namespace utils {
	template <class _type>
	inline typename std::enable_if<std::is_integral<_type>::value, _type>::type
		ceildiv(_type lhs, _type rhs)
	{
		return (lhs + rhs - 1) / rhs;
	}

	template <class _type>
	inline typename std::enable_if<std::is_integral<_type>::value, _type>::type
		ceiltomultiple(_type lhs, _type rhs)
	{
		return (lhs + rhs - 1) / rhs * rhs;
	}

	template <class _type>
	inline typename std::enable_if<std::is_integral<_type>::value, _type>::type
		floordiv(_type lhs, _type rhs)
	{
		return lhs / rhs;
	}

	template <class _type>
	inline typename std::enable_if<std::is_integral<_type>::value, _type>::type
		floortomultiple(_type lhs, _type rhs)
	{
		return lhs / rhs * rhs;
	}

	template <class _type>
	inline typename std::enable_if<std::is_integral<_type>::value, byte_t>::type&
		last_byte_of(_type& _buf)
	{
		return *(reinterpret_cast<byte_t*>(&_buf) + (sizeof(_type) - 1));
	}

	template <class _type>
	inline typename std::enable_if<std::is_integral<_type>::value && std::is_unsigned<_type>::value, _type>::type
		ceiltopowerof2(_type x)
	{
		x--;
		if (sizeof(_type) * CHAR_BIT > 1)
			x |= (x >> 1);
		if (sizeof(_type) * CHAR_BIT > 2)
			x |= (x >> 2);
		if (sizeof(_type) * CHAR_BIT > 4)
			x |= (x >> 4);
		if (sizeof(_type) * CHAR_BIT > 8)
			x |= (x >> 8);
		if (sizeof(_type) * CHAR_BIT > 16)
			x |= (x >> 16);
		if (sizeof(_type) * CHAR_BIT > 32)
			x |= ((x >> 16) >> 16);
		if (sizeof(_type) * CHAR_BIT > 64)
			x |= ((((x >> 16) >> 16) >> 16) >> 16);
		if (sizeof(_type) * CHAR_BIT > 128)
			x |= ((((((((x >> 16) >> 16) >> 16) >> 16) >> 16) >> 16) >> 16) >> 16);
		return x + 1;
	}

	template <class _type>
	inline typename std::enable_if<std::is_integral<_type>::value && std::is_unsigned<_type>::value, size_t>::type
		counttrailingzero(_type x)
	{
		const std::size_t w = sizeof(_type) * CHAR_BIT;
		if (x == 0) return w;
		size_t n = w - 1;
		_type y; 
		if (w > 64) { y = ((((x << 16) << 16) << 16) << 16); if (y != 0) { n -= 64; x = y; } }
		if (w > 32) { y = ((x << 16) << 16); if (y != 0) { n -= 32; x = y; } }
		if (w > 16) { y = x << 16; if (y != 0) { n -= 16; x = y; } }
		if (w > 8)  { y = x << 8;  if (y != 0) { n -= 8 ; x = y; } }
		if (w > 4)  { y = x << 4;  if (y != 0) { n -= 4 ; x = y; } }
		if (w > 2)  { y = x << 2;  if (y != 0) { n -= 2 ; x = y; } }
		if (w > 1)  { y = x << 1;  if (y != 0) { n -= 1 ; x = y; } }

		return n;

	}

	template <class _type>
	inline typename std::enable_if<std::is_integral<_type>::value && std::is_unsigned<_type>::value, size_t>::type
		leftbitshift(_type x, _type y)
	{
		while (y > 16)
		{
			x <<= 16;
			y -= 16;
		}
		x <<= y;
		return x;
	}

	template <class _type>
	inline typename std::enable_if<std::is_integral<_type>::value && std::is_unsigned<_type>::value, size_t>::type
		rightbitshift(_type x, _type y)
	{
		while (y > 16)
		{
			x >>= 16;
			y -= 16;
		}
		x >>= y;
		return x;
	}

}
}

