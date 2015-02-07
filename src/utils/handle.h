#pragma once

#include <type_traits>
#include <cstdint>
#include <cstdlib>

namespace foliage {
namespace utils {
	template <class T, class = void >
	class handle;

	template <class T>
	class handle<T, typename std::enable_if<std::is_class<T>::value || std::is_void<T>::value>::type >
	{
	public:
		T* operator ->() { return pointer; }
	public:
		T* pointer;
	};

}
}