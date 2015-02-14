#include "unittest.h"

#include "../src/runtime/runtime.h"
#include "../src/runtime/memmgr_embedded.h"

void diag_memmgr_dump_freelist(foliage::runtime::object* memmgr_obj)
{
	foliage::utils::byte_t* memory_chunk = reinterpret_cast<foliage::utils::byte_t*>(memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_memory_block]);
	size_t freelist_count = memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_freelist_size];
	std::uintptr_t* freelist_array = reinterpret_cast<std::uintptr_t*>(memory_chunk)+memmgr_obj->slotvalue<foliage::runtime::memory_mgr_stategy_embedded::private_slot_freelist_startpos>();
	printf("  freelist content:\n");
	for (size_t freelist_idx = 0; freelist_idx < freelist_count; freelist_idx++)
	{
		std::uintptr_t* next_node = reinterpret_cast<std::uintptr_t*>(freelist_array[freelist_idx]);
		printf("fl#%02d: %p", freelist_idx, next_node);
		if (next_node)
		{
			size_t start_cell = next_node - reinterpret_cast<std::uintptr_t*>(memory_chunk);
			size_t length_cell = foliage::utils::leftbitshift<size_t>(1, freelist_idx);
			printf("(#0x%x-#0x%x, %d cells)", start_cell, start_cell + length_cell, length_cell);
		}

		if (next_node)
		{
			do
			{
				next_node = next_node ? *reinterpret_cast<std::uintptr_t**>(next_node) : nullptr;
				printf(" -> %p", next_node);
				if (next_node)
				{
					size_t start_cell = next_node - reinterpret_cast<std::uintptr_t*>(memory_chunk);
					size_t length_cell = foliage::utils::leftbitshift<size_t>(1, freelist_idx);
					printf("(#0x%x-#0x%x, %d cells)", start_cell, start_cell + length_cell, length_cell);
				}

			}
			while (next_node);
		}
		printf("\n");
	}

}

void diag_memmgr_dump_bitmap(foliage::runtime::object* memmgr_obj)
{
	foliage::utils::byte_t* memory_chunk = reinterpret_cast<foliage::utils::byte_t*>(memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_memory_block]);
	size_t bitmap_size = memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_bitmap_size];
	foliage::utils::byte_t* bitmap = memory_chunk +
		memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_bitmap_startpos] * sizeof(std::uintptr_t);
	size_t bitmap_byte_size = bitmap_size * sizeof(std::uintptr_t);
	printf("  bitmap content:\n");
	const size_t column_count = 4;
	for (size_t bitmap_byte_idx = 0; bitmap_byte_idx < bitmap_byte_size; bitmap_byte_idx++)
	{
		if (bitmap_byte_idx % column_count == 0)
		{
			printf("%08X:", bitmap_byte_idx * CHAR_BIT);
		}

		printf(" ");
		for (size_t bit_idx = 0; bit_idx < CHAR_BIT; bit_idx++)
		{
			printf("%c", (bitmap[bitmap_byte_idx] & (1 << bit_idx)) == 0 ? '.' : '@');
		}

		if (bitmap_byte_idx % column_count == column_count - 1)
		{
			printf("\n");
		}
	}
}

void diag_memmgr_test_alloc(foliage::runtime::object* memmgr_obj)
{
	foliage::utils::memory_allocator < foliage::utils::byte_t, foliage::runtime::memmgr_t >
		byte_allocator(foliage::runtime::memmgr_t{ static_cast<foliage::runtime::memory_mgr*>(memmgr_obj) });

	foliage::utils::byte_t* memory_chunk =
		reinterpret_cast<foliage::utils::byte_t*>(memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_memory_block]);

	const size_t test_count = 5;
	const bool use_test_cases = false;
	const size_t test_cases[] = { 2, 8, 5, 1, 10, 5, 9, 0, 0, 0 };
	const auto test_case_generator = [](int case_idx){ return rand() % 16 * (1 << (4 * (rand() % 5))); };
	for (int i = 0; i < test_count; i++)
	{
		size_t alloc_size = use_test_cases ? test_cases[i] : test_case_generator(i);
		foliage::utils::byte_t* alloc_ptr = byte_allocator.allocate(alloc_size * sizeof(std::uintptr_t));

		printf("Alloc #%d: ", i);
		if (alloc_ptr)
		{
			size_t start_cell = foliage::utils::floordiv<size_t>(alloc_ptr - memory_chunk, sizeof(std::uintptr_t));
			size_t end_cell = foliage::utils::ceildiv<size_t>(alloc_ptr + alloc_size - memory_chunk, sizeof(std::uintptr_t));

			printf("%p(%d bytes), cells visible #0x%x - 0x%x(%d cells)", alloc_ptr, alloc_size, start_cell, end_cell, end_cell - start_cell);
			std::uninitialized_fill_n(alloc_ptr, alloc_size, 0x11 * (i % 16));
		}
		else
		{
			printf("failed (%d bytes)", alloc_size);
		}
		printf("\n");


		diag_memmgr_dump_freelist(memmgr_obj);
		diag_memmgr_dump_bitmap(memmgr_obj);
	}
}

void diag_memmgr_impl(foliage::runtime::object* runtime_obj)
{
	printf("Foliage runtime object: %p\n", runtime_obj);
	ASSERT_NE(nullptr, runtime_obj);
	auto memmgr_obj = reinterpret_cast<foliage::runtime::object*>(runtime_obj->slots[foliage::runtime::runtime::slot_memorymgr]);
	printf("Foliage memory manager object: %p\n", memmgr_obj);
	printf("  alloc: %p, dealloc: %p\n", 
		memmgr_obj->slots[foliage::runtime::memory_mgr::slot_fnptr_allocate],
		memmgr_obj->slots[foliage::runtime::memory_mgr::slot_fnptr_deallocate]);

	if (1)
	{
		printf("  managed memory: %d cells(%p-%p, %u bytes)\n",
			memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_memory_size] / sizeof(std::uintptr_t),
			memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_memory_block],
			memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_memory_block]
			+ memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_memory_size],
			memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_memory_size]);
		printf("      free lists: %d cells(cell #%u-%u)\n",
			memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_freelist_size],
			memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_freelist_startpos],
			memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_freelist_startpos] 
			+ memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_freelist_size]);
		printf("          bitmap: %d cells(cell #%u-%u)\n",
			memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_bitmap_size],
			memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_bitmap_startpos],
			memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_bitmap_startpos]
			+ memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_bitmap_size]);
		printf("           arena: %d cells(cell #%u-%u)\n",
			memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_arena_size],
			memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_arena_startpos],
			memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_arena_startpos]
			+ memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_arena_size]);

		diag_memmgr_dump_freelist(memmgr_obj);
		diag_memmgr_dump_bitmap(memmgr_obj);

		diag_memmgr_test_alloc(memmgr_obj);
//		memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_memory_block]
//			memmgr_obj->slots[foliage::runtime::memory_mgr_stategy_embedded::private_slot_memory_size]
	}
		// currently only embbeded memory manager is supported;

}

void diag_memmgr(foliage::runtime::object* runtime_obj)
{
	printf("Foliage memory manager diagnosis begin.\n");

	diag_memmgr_impl(runtime_obj);

	printf("Foliage memory manager diagnosis end.\n");
}