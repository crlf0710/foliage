#define FOLIAGE_RUNTIME_IMPL
#include "runtime.h"
#include "memmgr.h"
#include "memmgr_embedded.h"
#include "memmgr_system.h"
#include "../symboldata/symboldata.h"

namespace foliage {
namespace runtime {
	typedef std::unique_ptr<runtime> runtime_ptr;
	typedef std::unique_ptr<runtime, foliage::utils::placement_delete<runtime> > embedded_runtime_ptr;
	typedef memory_mgr_impl<memory_mgr_stategy_system> system_memory_mgr;
	typedef memory_mgr_impl<memory_mgr_stategy_embedded> embedded_memory_mgr;

	template class memory_mgr_impl<memory_mgr_stategy_system>;

	/* static */
	runtime_ptr runtime::create()
	{
		runtime_ptr rt(new runtime);
		auto const _memory_mgr = (new system_memory_mgr())->handle();
		rt->slotvalue<runtime::slot_memorymgr>() = cast_slot_value(_memory_mgr);

		auto* const symboldatachain = new(_memory_mgr)runtime::symbol_data_chain_type(_memory_mgr);
		rt->slotvalue<runtime::slot_symboldatachain>() = cast_slot_value(symboldatachain);
		return std::move(rt);
	}

	/* static */
	embedded_runtime_ptr runtime::create(void* memory_block, std::size_t memory_block_size)
	{
		foliage::utils::byte_t* const memory_startpos = reinterpret_cast<foliage::utils::byte_t*>(memory_block);
		foliage::utils::byte_t* const runtime_startpos = memory_startpos;
		embedded_runtime_ptr rt(new (runtime_startpos)runtime);
		foliage::utils::byte_t* const memory_mgr_startpos = runtime_startpos + sizeof(runtime);
		auto const _memory_mgr = (new (memory_mgr_startpos)embedded_memory_mgr(memory_startpos, memory_block_size))->handle();
		rt->slotvalue<runtime::slot_memorymgr>() = cast_slot_value(_memory_mgr);

		//auto* const symboldatachain = new(_memory_mgr)runtime::symbol_data_chain_type(_memory_mgr);
		//rt->slotvalue<runtime::slot_symboldatachain>() = cast_slot_value(symboldatachain);

		return std::move(rt);
	}

	runtime::~runtime()
	{

	}

}
}
