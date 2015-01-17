#include <type_traits>
#include <cstdint>
#include <cstdlib>

namespace foliage {
namespace utils {

	template <class _type>
	typename std::enable_if<std::is_integral<_type>::value, _type>::type
		ceildiv(_type lhs, _type rhs)
	{
		return (lhs + rhs - 1) / rhs;
	}

}
}