#define FOLIAGE_RUNTIME_IMPL
#include "runtime.h"
#include "memmgr.h"
#include "../symboldata/symboldata.h"

namespace foliage {
namespace runtime {
	typedef std::unique_ptr<runtime> runtime_ptr;
	typedef std::unique_ptr<runtime, foliage::utils::placement_delete<runtime> > embedded_runtime_ptr;
	typedef memory_mgr_impl<memory_mgr_stategy_system> system_memory_mgr;
	typedef memory_mgr_impl<memory_mgr_stategy_embedded> embedded_memory_mgr;

	/* static */
	runtime_ptr runtime::create()
	{
		runtime_ptr rt(new runtime);
		auto const _memory_mgr = static_cast<memory_mgr*>(*new system_memory_mgr());
		rt->slotvalue<runtime::slot_memorymgr>() = cast_slot_value(_memory_mgr);

		typedef memory_allocator<void> alloc_type;
		auto* const symboldatachain = new(_memory_mgr)foliage::symboldata::symboldatachain<alloc_type>(alloc_type(_memory_mgr));
		rt->slotvalue<runtime::slot_symboldatachain>() = cast_slot_value(symboldatachain);
		return std::move(rt);
	}

	/* static */
	embedded_runtime_ptr runtime::create(std::unique_ptr<foliage::utils::byte_t[]> memory_block, std::size_t memory_block_size)
	{
		embedded_runtime_ptr rt(new (memory_block.get())runtime);
		
		foliage::utils::byte_t* const runtime_startpos = memory_block.get();
		foliage::utils::byte_t* const memory_mgr_startpos = runtime_startpos + sizeof(runtime);
		
		auto const _memory_mgr = static_cast<memory_mgr*>(*new (memory_mgr_startpos)embedded_memory_mgr(memory_block.release(), memory_block_size));
		rt->slotvalue<runtime::slot_memorymgr>() = cast_slot_value(_memory_mgr);

		typedef memory_allocator<void> alloc_type;
		auto* const symboldatachain = new(_memory_mgr)foliage::symboldata::symboldatachain<alloc_type>(alloc_type(_memory_mgr));
		rt->slotvalue<runtime::slot_symboldatachain>() = cast_slot_value(symboldatachain);

		return std::move(rt);
	}


	runtime::~runtime()
	{

	}

}
}
