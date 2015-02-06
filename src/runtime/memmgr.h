#pragma once

#include <new>
#include "object.h"

namespace foliage {
namespace runtime {

	template <class _strategy>
	class memory_mgr : public object_n < 3 + _strategy::private_slot_count >
	{
	public:
		enum
		{
			slot_type_token = 0,
			slot_fnptr_allocate = 1,
			slot_fnptr_deallocate = 2,
		};
	public:
		template <typename... _types>
		memory_mgr(_types&&... args)
		{
			_strategy::initialize(*this, std::forward<_types>(args)...);
		}


		template <typename... _types>
		void* operator new(size_t _size, _types&&... args)
		{
			return _strategy::allocself(_size, std::forward<_types>(args)...);
		}

		template <typename... _types>
		void operator delete(void *ptr, _types&&... args)
		{
			return _strategy::deallocself(ptr, std::forward<_types>(args)...);
		}

	};

	class memory_mgr_stategy_system
	{
	public:
		enum
		{
			private_slot_count = 0
		};

	public:
		static void* allocself(size_t _size)
		{
			return ::operator new(_size);
		}
		
		static void deallocself(void* _ptr)
		{
			::operator delete(_ptr);
		}

		static void initialize(memory_mgr<memory_mgr_stategy_system>& _memmgr)
		{

		}
	};

	class memory_mgr_stategy_embedded
	{
	public:
		enum
		{
			private_slot_count = 1
		};

	public:
		static void* allocself(size_t _size, foliage::utils::byte_t* _place)
		{
			return ::operator new(_size, _place);
		}

		static void deallocself(void* _ptr, foliage::utils::byte_t* _place)
		{
			::operator delete(_ptr, _place);
		}

		static void initialize(memory_mgr<memory_mgr_stategy_embedded>& _memmgr)
		{

		}

	};
}
}