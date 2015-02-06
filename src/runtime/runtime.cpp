#define FOLIAGE_RUNTIME_IMPL
#include "runtime.h"
#include "memmgr.h"

namespace foliage {
namespace runtime {
	using foliage::utils::byte_t;

	/* static */
	std::unique_ptr<runtime, foliage::utils::placement_delete<runtime> >
		runtime::create(std::unique_ptr<foliage::utils::byte_t[]> memory_block, std::size_t memory_block_size)
	{
		std::unique_ptr<runtime, foliage::utils::placement_delete<runtime> > rt(new (memory_block.get())runtime);
		rt->slotvalue<runtime::slot_memorymgr>() = 
			(std::uintptr_t)new (memory_block.get() + sizeof(runtime)) memory_mgr<memory_mgr_stategy_embedded>();
		//runtime::slotvalue()
		
		// this memory will leak for now. it will be managed and released by memory mgr when it is ready.
		auto ptr = memory_block.release();
		return std::move(rt);
	}

	/* static */
	std::unique_ptr<runtime> runtime::create()
	{
		std::unique_ptr<runtime> rt(new runtime);
		return std::move(rt);
	}

	runtime::~runtime()
	{

	}

}
}
