#pragma once

#include "symboldata.h"

namespace foliage {
namespace symboldata {

	template<class _alloc>
	class symboldatachain<_alloc>::group_iterator
	{
	public:
		symboldatachain*				chain;
		typename _chaintype::iterator	iter;
		symboldataoffset_t				offset;

	public:
		group_iterator(symboldatachain* _chain, symboldataref_t _ref)
			: chain(_chain), iter(_chain->nodes.begin()), offset(symboldataref_invalid)
		{
			assert(_chain != nullptr);
			_set(_ref);
		}

		group_iterator& operator=(symboldataref_t _ref)
		{
			_set(_ref);
			return *this;
		}

		void _set(symboldataref_t _ref)
		{
			if (_ref != symboldataref_invalid && _ref < chain->get_capacity())
			{
				iter += _ref / symboldata_sectorsize;
				offset = _ref % symboldata_sectorsize;
			}
			else
			{
				offset = symboldataref_invalid;
			}
		}

		symboldataref_t _get()
		{
			return offset == symboldataref_invalid
				? symboldataref_invalid
				: (iter - chain->nodes.begin()) * symboldata_sectorsize + offset;
		}

		bool is_invalid() const
		{
			return offset == symboldataref_invalid;
		}

		bool is_valid() const
		{
			return !is_invalid();
		}

		operator symboldatagroup_t* () const
		{
			return is_invalid() ? nullptr : (*iter)->buffer + offset;
		}

		symboldataref_t get_ref()
		{
			return _get();
		}

		group_iterator& operator++()
		{
			if (is_invalid())
			{
				if (chain->get_capacity())
					_set(0);
			}
			else
			{
				bool last_node = iter + 1 == chain->nodes.end();
				if (offset < (last_node ? chain->cursor : symboldata_sectorsize))
				{
					offset++;
				}
				else if (!last_node)
				{
					iter++;
					offset = 0;
				}
				else
				{
					_set(symboldataref_invalid);
				}
			}
			return *this;
		}

		group_iterator& operator--()
		{
			// actual decrement takes place here
			if (is_invalid())
			{
				size_t capacity = chain->get_capacity();
				if (capacity)
					_set(capacity - 1);
			}
			else
			{
				bool first_node = iter == chain->nodes.begin();
				if (offset > 0)
				{
					offset--;
				}
				else if (!first_node)
				{
					iter--;
					offset = symboldata_sectorsize - 1;
				}
				else
				{
					_set(symboldataref_invalid);
				}
			}
			return *this;
		}

		group_iterator operator++(int) { auto tmp(*this); operator++(); return tmp; }
		group_iterator operator--(int) { auto tmp(*this); operator--(); return tmp; }
	};

}
}