#pragma once

#include <cstdint>
#include <cstddef>
#include <limits>
#include <new>
#include "../symboldata/symboldata.h"
#include "../utils/sysutils.h"
#include "../utils/handle.h"


namespace foliage {
namespace runtime {
	// forward declaration
	class object;
	template <size_t _n> class object_n;

	class type_info;

	typedef class { std::uintptr_t unused; } *type_token_t;

	typedef std::uintptr_t objectref_t;
	static_assert(std::numeric_limits<objectref_t>::is_signed == false, "objectref_t is always unsigned");
	static const objectref_t objectref_typemask = 1;

	// foliage_object

	class object
	{
	public:
		std::uintptr_t slots[1];
	public:
		void* operator new(std::size_t oneslot, std::size_t slotcount)
		{
			assert(slotcount > 0);
			return ::operator new(oneslot * slotcount);
		}

		inline void* operator new(std::size_t oneslot, type_info* type_info);

	public:
		void* operator new(std::size_t) = delete;
		void* operator new[](std::size_t) = delete;
	public:
		template <size_t _n>
		static inline std::uintptr_t* slotptr(object* object)
		{
			return object ? object->slots + _n : nullptr;
		}

		template <size_t _n>
		inline std::uintptr_t* slotptr()
		{
			return slotptr<_n>(this);
		}

		template <size_t _n>
		static inline std::uintptr_t& slotvalue(object* object)
		{
			assert(object);
			return object->slots[_n];
		}

		template <size_t _n>
		static inline std::uintptr_t slotvalue(const object* object)
		{
			assert(object);
			return object->slots[_n];
		}

		template <size_t _n>
		inline std::uintptr_t& slotvalue()
		{
			return slotvalue<_n>(this);
		}
		
		template <size_t _n>
		inline std::uintptr_t slotvalue() const
		{
			return slotvalue<_n>(this);
		}


		typedef std::uintptr_t& (&fnptr_slotref)(object*);

		template <size_t _n>
		static inline object* from_slotptr(std::uintptr_t* slotptr)
		{
			return slotptr ?
				reinterpret_cast<object*>(
				reinterpret_cast<foliage::utils::byte_t*>(slotptr)-offsetof(object, slots[_n])) :
				nullptr;
		}
	};

	// object_n
	
	template <size_t _n> class object_n : public object
	{
	protected:
		std::uintptr_t extra_slots[_n - 1];
	public:
		template <size_t _n2>
		operator object_n<_n2>&()
		{
			static_assert(_n2 < _n && _n2 > 0, "foliage_object_n conversion to specified type is not safe.");
			return reinterpret_cast<object_n < _n2 >&> (*this);
		}
	};
	template <> class object_n<1> : public object{};

	template <class T>
	std::enable_if_t<std::is_integral<T>::value, std::uintptr_t>
		inline cast_slot_value(T _value)
	{
		return static_cast<std::uintptr_t>(_value);
	}
	
	template <class T>
	std::enable_if_t<std::is_pointer<T>::value, std::uintptr_t>
		inline cast_slot_value(T _value)
	{
		return reinterpret_cast<std::uintptr_t>(_value);
	}

	template <class T>
	std::uintptr_t
		inline cast_slot_value(foliage::utils::handle<T> _value)
	{
		return reinterpret_cast<std::uintptr_t>(_value.pointer);
	}

	class type_info : public object_n<2>
	{
	public:
		enum
		{
			slot_typetoken = 0,
			slot_slotcount = 1,

			slot_typeinfotoken_offset = 0
		};
	public:
		static inline type_info* type_info_from_token(type_token_t token)
		{
			return static_cast<type_info*>(object::from_slotptr<slot_typeinfotoken_offset>(reinterpret_cast<std::uintptr_t*>(token)));
		}
	public:
		static inline type_token_t type_token(type_info* type_info)
		{
			return reinterpret_cast<type_token_t>(object::slotptr<slot_typeinfotoken_offset>(type_info));
		}

		static const object::fnptr_slotref& slot_count;
	};

	inline void* object::operator new(std::size_t oneslot, type_info* type_info)
	{
		assert(type_info);
		return operator new(oneslot, type_info::slot_count(type_info));
	}

#ifdef FOLIAGE_OBJECT_IMPL
	const object::fnptr_slotref& type_info::slot_count = object::slotvalue < slot_slotcount > ;
#endif

}
}
