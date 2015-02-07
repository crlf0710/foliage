#pragma once

#include <new>
#include "object.h"
#include "../utils/mathutils.h"

namespace foliage {
namespace runtime {
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

		typedef void* (*fnptr_allocate)(memory_mgr*, size_t);
		typedef void (*fnptr_deallocate)(memory_mgr*, void*, size_t);

		fnptr_allocate allocate_fn() const { return reinterpret_cast<fnptr_allocate>(slotvalue<slot_fnptr_allocate>()); }
		fnptr_deallocate deallocate_fn() const { return reinterpret_cast<fnptr_deallocate>(slotvalue<slot_fnptr_deallocate>()); }

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

		operator memory_mgr*()
		{
			return reinterpret_cast<memory_mgr*>(&static_cast<object_n<memory_mgr::slot_count>&>(*this));
		}

	};

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

	class memory_mgr_stategy_embedded
	{
	public:
		enum
		{
			private_slot_count = 2,
			private_slot_memory_block = memory_mgr::slot_offset_strategydata + 0,
			private_slot_memory_size  = memory_mgr::slot_offset_strategydata + 1,
		};

	public:
		static void* allocself(size_t _size, foliage::utils::byte_t* _place)
		{
			return ::operator new(_size, _place);
		}

		static void deallocself(void* _ptr, foliage::utils::byte_t* _place)
		{
			::operator delete(_ptr, _place);
		}

		static void* alloc(memory_mgr*, size_t _size)
		{
			return NULL;
		}

		static void dealloc(memory_mgr*, void* _ptr, size_t _size)
		{
			return;
		}

		static void initialize(memory_mgr_impl<memory_mgr_stategy_embedded>& _memmgr, void* _memory_block, size_t _memory_size)
		{
			_memmgr.slotvalue<memory_mgr::slot_fnptr_allocate>() = cast_slot_value(alloc);
			_memmgr.slotvalue<memory_mgr::slot_fnptr_deallocate>() = cast_slot_value(dealloc);

			_memmgr.slotvalue<private_slot_memory_block>() = cast_slot_value(_memory_block);
			_memmgr.slotvalue<private_slot_memory_size>() = cast_slot_value(_memory_size);
		}

	};

#define _OLD_GCC_COMPATIBILITY_MODE

#ifdef _OLD_GCC_COMPATIBILITY_MODE
#define memory_allocator memory_allocator_base
#endif

	template <class _type>
	class memory_allocator
	{
	public:
		typedef _type value_type;
		typedef value_type* pointer;
	public:
		memory_allocator(memory_mgr* _memmgr)
			: memmgr(_memmgr)
		{
		}

		template <class _type2>
		memory_allocator(const memory_allocator<_type2>& _other)
		{
			memmgr = _other.memmgr;
		}

		pointer allocate(size_t _count)
		{
			size_t _destcount = foliage::utils::ceildiv(sizeof(value_type) * _count, sizeof(std::uintptr_t));
			return static_cast<pointer>(memmgr->allocate_fn()(memmgr, _destcount));
		}

		void deallocate(void* _ptr, size_t _count)
		{
			size_t _destcount = foliage::utils::ceildiv(sizeof(value_type) * _count, sizeof(std::uintptr_t));
			return memmgr->deallocate_fn()(memmgr, _ptr, _destcount);
		}

	protected:
		memory_mgr* memmgr;
		
		template<typename _type2>
		friend class memory_allocator;
	};

#ifdef _OLD_GCC_COMPATIBILITY_MODE
#undef memory_allocator

	// not actually needed, only for compatibility.
	template <class _type>
	class memory_allocator : public memory_allocator_base<_type>
	{
	public:
		typedef memory_allocator_base<_type> _base_type;
		typedef typename _base_type::value_type value_type;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;

		template <class _type2>
		struct rebind
		{
			typedef memory_allocator<_type2> other;
		};

		memory_allocator(memory_mgr* _memmgr) : _base_type(_memmgr) {}

		template <class _type2>
		memory_allocator(const memory_allocator<_type2>& _other) : _base_type(_other) {}
	};

	template <>
	class memory_allocator<void> : public memory_allocator_base < void >
	{
	public:
		typedef memory_allocator_base< void > _base_type;
		typedef const void* const_pointer;

		template <class _type2>
		struct rebind
		{
			typedef memory_allocator<_type2> other;
		};

		memory_allocator(memory_mgr* _memmgr) : _base_type(_memmgr) {}

		template <class _type2>
		memory_allocator(const memory_allocator<_type2>& _other) : _base_type(_other) {}
	};

#endif


}
}
