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
	inline typename std::enable_if<std::is_integral<_type>::value, std::uint8_t>::type&
		last_byte_of(_type& _buf)
	{
		return *(reinterpret_cast<std::uint8_t*>(&_buf) + (sizeof(_type) - 1));
	}
}
}