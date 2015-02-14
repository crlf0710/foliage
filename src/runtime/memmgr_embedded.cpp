#include "memmgr_embedded.h"

namespace foliage {
namespace runtime {
		

	static foliage::utils::byte_t* request_buddy_memory(memory_mgr* _memmgr, size_t request_size_power_min, size_t request_size_power_max, size_t* actual_size_power_ptr)
	{
		foliage::utils::byte_t* memory_chunk = reinterpret_cast<foliage::utils::byte_t*>(_memmgr->slotvalue<memory_mgr_stategy_embedded::private_slot_memory_block>());
		size_t arena_startpos = _memmgr->slotvalue<memory_mgr_stategy_embedded::private_slot_arena_startpos>();
		size_t arena_count = _memmgr->slotvalue<memory_mgr_stategy_embedded::private_slot_arena_size>();
		size_t bitmap_startpos = _memmgr->slotvalue<memory_mgr_stategy_embedded::private_slot_bitmap_startpos>();
		size_t bitmap_count = _memmgr->slotvalue<memory_mgr_stategy_embedded::private_slot_bitmap_size>();

		foliage::utils::byte_t* bitmap = reinterpret_cast<foliage::utils::byte_t*>(memory_chunk) + bitmap_startpos * sizeof(std::uintptr_t);
		size_t ret_offset = foliage::utils::bitmap_search_aligned_gap(bitmap, arena_startpos, bitmap_count * (CHAR_BIT * sizeof(std::uintptr_t)), request_size_power_min);
		if (ret_offset == (size_t)-1) return nullptr;
		size_t actual_size_power = request_size_power_min;
		if (request_size_power_max > request_size_power_min)
		{
			request_size_power_max = std::min(foliage::utils::counttrailingzero(ret_offset), request_size_power_max);
			if (request_size_power_max > request_size_power_min)
			{
				actual_size_power = foliage::utils::bitmap_find_largest_gap(bitmap, ret_offset, request_size_power_min, request_size_power_max);
			}
		}
		size_t actual_size = foliage::utils::leftbitshift<size_t>(1, actual_size_power);
		foliage::utils::bitmap_fill_n<1>(bitmap, ret_offset, actual_size);
		reinterpret_cast<std::uintptr_t*>(memory_chunk)[ret_offset + actual_size - 1] = actual_size;
		if (actual_size_power_ptr)
			*actual_size_power_ptr = actual_size_power;
		return memory_chunk + ret_offset * sizeof(std::uintptr_t);
	}

	static inline void append_freelist_item(memory_mgr* _memmgr, std::uintptr_t* freelist_array, size_t freelist_idx, foliage::utils::byte_t* memory_chunk, size_t memory_size)
	{
		reinterpret_cast<std::uintptr_t*>(memory_chunk)[memory_size - 1] = memory_size;
		reinterpret_cast<std::uintptr_t*>(memory_chunk)[0] = freelist_array[freelist_idx];
		freelist_array[freelist_idx] = reinterpret_cast<std::uintptr_t>(memory_chunk);
	}


	static foliage::utils::byte_t* request_non_freelist_memory(memory_mgr* _memmgr, size_t request_size_power)
	{
		return request_buddy_memory(_memmgr, request_size_power, request_size_power - 1, nullptr);
	}

	static void split_freelist_large_memory(memory_mgr* _memmgr, std::uintptr_t* freelist_array, size_t freelist_idx, size_t freelist_checkitem)
	{
		foliage::utils::byte_t* oversize_memory = reinterpret_cast<foliage::utils::byte_t*>(freelist_array[freelist_checkitem]);
		freelist_array[freelist_checkitem] = *reinterpret_cast<std::uintptr_t*>(oversize_memory);
		size_t freelist_item_size = foliage::utils::leftbitshift<size_t>(1, freelist_idx);

		size_t freelist_curitem = freelist_idx;
		size_t freelist_curitem_size = freelist_item_size;
		foliage::utils::byte_t* oversize_memory_current = oversize_memory + freelist_curitem_size * sizeof(std::uintptr_t);

		for (; freelist_curitem < freelist_checkitem; freelist_curitem++, freelist_curitem_size <<= 1)
		{
			append_freelist_item(_memmgr, freelist_array, freelist_curitem, oversize_memory_current, freelist_curitem_size);
			oversize_memory_current += freelist_curitem_size * sizeof(std::uintptr_t);
		}

		append_freelist_item(_memmgr, freelist_array, freelist_idx, oversize_memory, freelist_item_size);

	}

	static bool supply_freelist_memory(memory_mgr* _memmgr, std::uintptr_t* freelist_array, size_t freelist_idx, size_t freelist_count)
	{
		size_t actual_size_power = 0;
		foliage::utils::byte_t* oversize_memory = request_buddy_memory(_memmgr, freelist_idx, freelist_count - 1, &actual_size_power);
		if (!oversize_memory) return false;
		append_freelist_item(_memmgr, freelist_array, actual_size_power, oversize_memory, foliage::utils::leftbitshift<size_t>(1, actual_size_power));
		split_freelist_large_memory(_memmgr, freelist_array, freelist_idx, actual_size_power);
		return true;
	}

	static foliage::utils::byte_t* request_freelist_memory(memory_mgr* _memmgr, size_t request_size_power)
	{
		size_t freelist_count = _memmgr->slotvalue<memory_mgr_stategy_embedded::private_slot_freelist_size>();
		size_t freelist_idx = request_size_power;
		assert(freelist_idx > 0);

		if (freelist_idx >= freelist_count)
			return request_non_freelist_memory(_memmgr, freelist_idx);

		std::uintptr_t* freelist_array = 
			reinterpret_cast<std::uintptr_t*>(_memmgr->slotvalue<memory_mgr_stategy_embedded::private_slot_memory_block>()) + 
			_memmgr->slotvalue<memory_mgr_stategy_embedded::private_slot_freelist_startpos>();

		if (!freelist_array[freelist_idx])
		{
			for (size_t freelist_checkitem = freelist_idx + 1; freelist_checkitem < freelist_count; freelist_checkitem++)
			{
				if (freelist_array[freelist_checkitem])
				{
					split_freelist_large_memory(_memmgr, freelist_array, freelist_idx, freelist_checkitem);
					goto freelist_item_ready;
				}
			}

			if (!supply_freelist_memory(_memmgr, freelist_array, freelist_idx, freelist_count))
				return nullptr;
		}
freelist_item_ready:
		std::uintptr_t* requested_memory = reinterpret_cast<std::uintptr_t*>(freelist_array[freelist_idx]);
		freelist_array[freelist_idx] = *requested_memory;
		return reinterpret_cast<foliage::utils::byte_t*>(requested_memory);
	}

	static size_t detect_size(memory_mgr* _memmgr, void* _ptr)
	{
		return 0;
	}

	/* static */
	void* memory_mgr_stategy_embedded::alloc(memory_mgr* _memmgr, size_t _size)
	{
		if (_size == 0) _size = 1;
		// following are using cell as units.
		size_t actual_size = foliage::utils::ceildiv(_size + sizeof(std::uintptr_t), sizeof(std::uintptr_t));
		size_t request_size = foliage::utils::ceiltopowerof2(actual_size);
		size_t request_size_power = foliage::utils::counttrailingzero(request_size);
		foliage::utils::byte_t* raw_chunk = request_freelist_memory(_memmgr, request_size_power);
		if (!raw_chunk) return nullptr;
		if (actual_size < request_size)
		{
			foliage::utils::byte_t* memory_chunk = reinterpret_cast<foliage::utils::byte_t*>(_memmgr->slotvalue<memory_mgr_stategy_embedded::private_slot_memory_block>());
			size_t bitmap_startpos = _memmgr->slotvalue<memory_mgr_stategy_embedded::private_slot_bitmap_startpos>();
			foliage::utils::byte_t* bitmap = reinterpret_cast<foliage::utils::byte_t*>(memory_chunk)+bitmap_startpos * sizeof(std::uintptr_t);
			size_t freelist_count = _memmgr->slotvalue<memory_mgr_stategy_embedded::private_slot_freelist_size>();
			std::uintptr_t* freelist_array = reinterpret_cast<std::uintptr_t*>(memory_chunk) + _memmgr->slotvalue<memory_mgr_stategy_embedded::private_slot_freelist_startpos>();
			size_t allocated_offset = reinterpret_cast<std::uintptr_t*>(raw_chunk)-reinterpret_cast<std::uintptr_t*>(memory_chunk);
			size_t allocated_size = actual_size;
			for (size_t item_size = 1, item_size_idx = 0; allocated_size < request_size; item_size <<= 1, item_size_idx++)
			{
				if ((allocated_size & item_size) == 0) continue;

				if (item_size_idx > 0 && item_size_idx < freelist_count)
				{
					append_freelist_item(_memmgr, freelist_array, item_size_idx, 
						reinterpret_cast<foliage::utils::byte_t*>(reinterpret_cast<std::uintptr_t*>(raw_chunk)+allocated_size), item_size);
				}
				else
				{
					foliage::utils::bitmap_fill_n<0>(bitmap, allocated_offset + allocated_size, item_size);
				}

				allocated_size += item_size;
			}
			assert(allocated_size == request_size);
			reinterpret_cast<std::uintptr_t*>(raw_chunk)[actual_size - 1] = actual_size;
		}
  		return raw_chunk;
	}

	/* static */
	void memory_mgr_stategy_embedded::dealloc(memory_mgr* _memmgr, void* _ptr, size_t _size)
	{
		if (!_ptr) return;
		if (_size == 0) _size = 1;
		size_t actual_size = (_size != (size_t)-1)
			? foliage::utils::ceiltomultiple(_size + sizeof(std::uintptr_t), sizeof(std::uintptr_t))
			: detect_size(_memmgr, _ptr);

		assert(reinterpret_cast<std::uintptr_t*>(reinterpret_cast<foliage::utils::byte_t*>(_ptr)+actual_size)[-1] == actual_size);

		return;
	}

	/* static */
	void memory_mgr_stategy_embedded::initialize(memory_mgr_impl<memory_mgr_stategy_embedded>& _memmgr, void* _memory_block, size_t _memory_size)
	{
		const size_t freelist_min_count = 3;
		const size_t _memory_cell_count = _memory_size / sizeof(std::uintptr_t);
		size_t extpart_start = foliage::utils::ceildiv<size_t>(
			reinterpret_cast<foliage::utils::byte_t*>(&_memmgr + 1) - reinterpret_cast<foliage::utils::byte_t*>(_memory_block),
			sizeof(std::uintptr_t));
		size_t freelist_start = extpart_start;
		size_t bitmap_start_min = foliage::utils::ceiltomultiple<size_t>(freelist_start + freelist_min_count, sizeof(std::uintptr_t));
		size_t bitmap_size = foliage::utils::ceildiv<size_t>(_memory_cell_count, CHAR_BIT * sizeof(std::uintptr_t));
		const size_t header_length_cell_length = 1;
		size_t arena_start = foliage::utils::ceiltomultiple<size_t>(bitmap_start_min + bitmap_size + header_length_cell_length, CHAR_BIT * sizeof(std::uintptr_t));
		assert(_memory_cell_count > arena_start);
		size_t arena_size = _memory_cell_count - arena_start;
		size_t bitmap_start = foliage::utils::floortomultiple<size_t>(arena_start - bitmap_size - header_length_cell_length, sizeof(std::uintptr_t));
		size_t freelist_size = bitmap_start - freelist_start;
		size_t extpart_size = arena_start - freelist_start;
		size_t extpart_end = arena_start;
		size_t arena_end = _memory_cell_count;

		_memmgr.slotvalue<memory_mgr::slot_fnptr_allocate>() = cast_slot_value(alloc);
		_memmgr.slotvalue<memory_mgr::slot_fnptr_deallocate>() = cast_slot_value(dealloc);

		_memmgr.slotvalue<private_slot_memory_block>() = cast_slot_value(_memory_block);
		_memmgr.slotvalue<private_slot_memory_size>() = cast_slot_value(_memory_size);
		_memmgr.slotvalue<private_slot_freelist_startpos>() = cast_slot_value(freelist_start);
		_memmgr.slotvalue<private_slot_freelist_size>() = cast_slot_value(freelist_size);
		_memmgr.slotvalue<private_slot_bitmap_startpos>() = cast_slot_value(bitmap_start);
		_memmgr.slotvalue<private_slot_bitmap_size>() = cast_slot_value(bitmap_size);
		_memmgr.slotvalue<private_slot_arena_startpos>() = cast_slot_value(arena_start);
		_memmgr.slotvalue<private_slot_arena_size>() = cast_slot_value(arena_size);

		std::uninitialized_fill_n(reinterpret_cast<std::uintptr_t*>(_memory_block)+extpart_start, extpart_size, (std::uintptr_t)0U);

		reinterpret_cast<std::uintptr_t*>(_memory_block)[extpart_end - 1] = extpart_end;
		foliage::utils::byte_t* bitmap = reinterpret_cast<foliage::utils::byte_t*>
			(reinterpret_cast<std::uintptr_t*>(_memory_block)+bitmap_start);
		foliage::utils::bitmap_fill<1>(bitmap, 0, extpart_end);
		foliage::utils::bitmap_fill<1>(bitmap, arena_end, bitmap_size * CHAR_BIT * sizeof(std::uintptr_t));
	}

}
}