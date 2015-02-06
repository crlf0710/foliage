#pragma once

#include <new>
#include "object.h"
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
	protected:
		void* operator new(std::size_t _size){ return ::operator new(_size); }
		void* operator new(std::size_t _size, void* _place) { return ::operator new(_size, _place); }

	public:
		~runtime();
	public:
		static std::unique_ptr<runtime> create();
		static std::unique_ptr<runtime, foliage::utils::placement_delete<runtime> > 
			create(std::unique_ptr<foliage::utils::byte_t[]> memory_block, std::size_t memory_block_size);
	};

	
}
}