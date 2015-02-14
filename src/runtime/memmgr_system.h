#pragma once
#include "memmgr.h"

namespace foliage {
namespace runtime {
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

		static void* alloc(memory_mgr*, size_t _size)
		{
			return ::operator new(_size);
		}

		static void dealloc(memory_mgr*, void* _ptr, size_t _size)
		{
			return ::operator delete(_ptr);
		}

		static void initialize(memory_mgr_impl<memory_mgr_stategy_system>& _memmgr)
		{
			_memmgr.slotvalue<memory_mgr::slot_fnptr_allocate>() = cast_slot_value(alloc);
			_memmgr.slotvalue<memory_mgr::slot_fnptr_deallocate>() = cast_slot_value(dealloc);
		}
	};


}
}