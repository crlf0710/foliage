#pragma once

#define _OLD_GCC_COMPATIBILITY_MODE

namespace foliage {
namespace utils {
	
#ifdef _OLD_GCC_COMPATIBILITY_MODE
#define memory_allocator memory_allocator_base
#endif

	template <class _type, class _memmgr_type>
	class memory_allocator
	{
	public:
		typedef _type value_type;
		typedef value_type* pointer;
	public:
		memory_allocator(_memmgr_type _memmgr)
			: memmgr(_memmgr)
		{
		}

		template <class _type2>
		explicit memory_allocator(const memory_allocator<_type2, _memmgr_type>& _other)
		{
			memmgr = _other.memmgr;
		}

		pointer allocate(size_t _count)
		{
			return static_cast<pointer>(operator new(_count * sizeof(value_type), memmgr));
		}

		void deallocate(void* _ptr, size_t _count)
		{
			return ::operator delete(_ptr, _count * sizeof(value_type), memmgr);
		}

	protected:
		_memmgr_type memmgr;
		
		template<typename _type2, typename _memmgr_type2>
		friend class memory_allocator;
	};

#ifdef _OLD_GCC_COMPATIBILITY_MODE
#undef memory_allocator

	// not actually needed, only for compatibility.
	template <class _type, class _memmgr_type>
	class memory_allocator : public memory_allocator_base<_type, _memmgr_type>
	{
	public:
		typedef memory_allocator_base<_type, _memmgr_type> _base_type;
		typedef typename _base_type::value_type value_type;
		typedef const value_type* const_pointer;

		typedef typename std::conditional<std::is_void<value_type>::value, int, value_type>::type& reference;
		typedef typename std::conditional<std::is_void<const value_type>::value, const int, const value_type > ::type& const_reference;

		template <class _type2>
		struct rebind
		{
			typedef memory_allocator<_type2, _memmgr_type> other;
		};

		memory_allocator(_memmgr_type _memmgr) : _base_type(_memmgr) {}

		template <class _type2>
		explicit memory_allocator(const memory_allocator<_type2, _memmgr_type>& _other) : _base_type(_other) {}
	};
#endif


}
}