#pragma once
#include <new>

namespace foliage {
namespace utils {

	template<class _type>
	class placement_delete
	{
	public:
		placement_delete() {}

		template<class _rhs_type, 
			class = typename std::enable_if<std::is_convertible<_rhs_type *, _type *>::value, void>::type>
			placement_delete(const placement_delete<_rhs_type>&) {}

		void operator()(_type *_ptr) const
		{
			_ptr->~_type();
		}
	};

}
}
