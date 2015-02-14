#pragma once

#include <new>
#include "object.h"
#include "../utils/mathutils.h"
#include "../utils/handle.h"
#include "../utils/allocator.h"

namespace foliage {
namespace runtime {
	class memory_mgr;

	typedef foliage::utils::handle<memory_mgr> memmgr_t;

	class memory_mgr : public object_n < 3 >
	{
	public:
		enum
		{
			slot_type_token = 0,
			slot_fnptr_allocate = 1,
			slot_fnptr_deallocate = 2,

			slot_count = 3,
			slot_offset_strategydata = 3,
		};

		typedef foliage::utils::memory_allocator<void, memmgr_t> allocator_type;

		typedef void* (*fnptr_allocate)(memory_mgr*, size_t);
		typedef void (*fnptr_deallocate)(memory_mgr*, void*, size_t);

		fnptr_allocate allocate_fn() const { return reinterpret_cast<fnptr_allocate>(slotvalue<slot_fnptr_allocate>()); }
		fnptr_deallocate deallocate_fn() const { return reinterpret_cast<fnptr_deallocate>(slotvalue<slot_fnptr_deallocate>()); }

		memmgr_t handle()
		{
			return{ this };
		}

	protected:
		memory_mgr() = delete;
		memory_mgr(const memory_mgr&) = delete;

	};

	template <class _strategy>
	class memory_mgr_impl : public object_n < 3 + _strategy::private_slot_count >
	{
	public:
		template <typename... _types>
		memory_mgr_impl(_types&&... args)
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

		memmgr_t handle()
		{
			return { reinterpret_cast<memory_mgr*>(&static_cast<object_n<memory_mgr::slot_count>&>(*this)) };
		}

	};

}
}

inline void* operator new (size_t _size, foliage::runtime::memmgr_t _memmgr)
{
	size_t _destsize = foliage::utils::ceildiv(_size, sizeof(std::uintptr_t)) * sizeof(std::uintptr_t);
	return (_memmgr->allocate_fn()(_memmgr.pointer, _destsize));
}

inline void operator delete (void* _ptr, foliage::runtime::memmgr_t _memmgr)
{
	return _memmgr->deallocate_fn()(_memmgr.pointer, _ptr, -1);
}

inline void operator delete (void* _ptr, size_t _size, foliage::runtime::memmgr_t _memmgr)
{
	size_t _destsize = foliage::utils::ceildiv(_size, sizeof(std::uintptr_t)) * sizeof(std::uintptr_t);
	return _memmgr->deallocate_fn()(_memmgr.pointer, _ptr, _destsize);
}

