#pragma once
#include "memmgr.h"
#include "../utils/bitmap.h"

namespace foliage {
namespace runtime {

	class memory_mgr_stategy_embedded
	{
	public:
		enum
		{
			private_slot_count = 8,
			private_slot_memory_block = memory_mgr::slot_offset_strategydata + 0,
			private_slot_memory_size = memory_mgr::slot_offset_strategydata + 1,
			private_slot_freelist_startpos = memory_mgr::slot_offset_strategydata + 2,
			private_slot_freelist_size = memory_mgr::slot_offset_strategydata + 3,
			private_slot_bitmap_startpos = memory_mgr::slot_offset_strategydata + 4,
			private_slot_bitmap_size = memory_mgr::slot_offset_strategydata + 5,
			private_slot_arena_startpos = memory_mgr::slot_offset_strategydata + 6,
			private_slot_arena_size = memory_mgr::slot_offset_strategydata + 7,
		};

		/* { slots | free list ptrs | bitmap | buddy section {chunk, + size_ptr} }*/

	public:
		static void* allocself(size_t _size, foliage::utils::byte_t* _place)
		{
			return ::operator new(_size, _place);
		}

		static void deallocself(void* _ptr, foliage::utils::byte_t* _place)
		{
			::operator delete(_ptr, _place);
		}

		static void initialize(memory_mgr_impl<memory_mgr_stategy_embedded>& _memmgr, void* _memory_block, size_t _memory_size);

		static void* alloc(memory_mgr* _memmgr, size_t _size);

		static void dealloc(memory_mgr* _memmgr, void* _ptr, size_t _size);

	};

}
}
