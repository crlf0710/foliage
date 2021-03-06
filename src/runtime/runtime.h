#pragma once

#include <new>
#include "object.h"
#include "memmgr.h"
#include "../utils/sysutils.h"
#include "../utils/placement_delete.h"

namespace foliage {
namespace runtime {

	class runtime : public object_n<4>
	{
	public:
		enum
		{
			slot_typetoken       = 0,
			slot_memorymgr       = 1,
			slot_symboldatachain = 2,
			slot_typehierarchy   = 3,
		};
		typedef memory_mgr::allocator_type alloc_type;

		typedef foliage::symboldata::symboldatachain<alloc_type> symbol_data_chain_type;

	public:
		~runtime();
		template < typename T >
		foliage::utils::memory_allocator<T, memmgr_t> get_allocator()
		{
			return foliage::utils::memory_allocator<T, memmgr_t>(
				reinterpret_cast<foliage::runtime::memory_mgr*>(slotvalue<runtime::slot_memorymgr>())->handle());
		}

	protected:
		void* operator new(std::size_t _size){ return ::operator new(_size); }
		void* operator new(std::size_t _size, void* _place){ return ::operator new(_size, _place); }
	public:
		void operator delete(void* _ptr) { return ::operator delete(_ptr); }
		void operator delete(void* _ptr, void* _place) { return ::operator delete(_ptr, _place); }

	public:
		static std::unique_ptr<runtime> create();
		static std::unique_ptr<runtime, foliage::utils::placement_delete<runtime> > 
			create(void* memory_block, std::size_t memory_block_size);
	};

	
}
}
