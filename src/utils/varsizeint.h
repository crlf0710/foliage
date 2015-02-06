#pragma once

#include <cstdint>

namespace foliage {
namespace utils {

	template <int _size> struct varsizeint_t;
	template <int _size> struct varsizeuint_t;

	template<> struct varsizeint_t  < 8 >  { typedef std::int8_t type; };
	template<> struct varsizeuint_t < 8 >  { typedef std::uint8_t type; };
	template<> struct varsizeint_t  < 16 > { typedef std::int16_t type; };
	template<> struct varsizeuint_t < 16 > { typedef std::uint16_t type; };
	template<> struct varsizeint_t  < 32 > { typedef std::int32_t type; };
	template<> struct varsizeuint_t < 32 > { typedef std::uint32_t type; };
	template<> struct varsizeint_t  < 64 > { typedef std::int64_t type; };
	template<> struct varsizeuint_t < 64 > { typedef std::uint64_t type; };



}
}