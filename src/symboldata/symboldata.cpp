#include <string>
#include <memory>
#include <cassert>
#include <cstring>
#include "symboldata.h"
#include "../utils/mathutils.h"

namespace foliage {
namespace symboldata {
	using std::size_t;
	using std::uint8_t;
	using std::string;
	using std::copy_n;
	using std::make_unique;
	using std::min;
	using std::strncmp;
	using foliage::utils::last_byte_of;

	class symboldatachain::group_iterator
	{
	public:
		symboldatachain*		chain;
		_chaintype::iterator	iter;
		symboldataoffset_t		offset;

	public:
		group_iterator(symboldatachain* _chain, symboldataref_t _ref)
			: chain(_chain), iter(_chain->nodes.begin()), offset(symboldataref_invalid)
		{
			assert(_chain != nullptr);
			_set(_ref);
		}

		auto& operator=(symboldataref_t _ref)
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

		auto& operator++()
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

		auto& operator--()
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

		auto operator++(int) { auto tmp(*this); operator++(); return tmp; }
		auto operator--(int) { auto tmp(*this); operator--(); return tmp; }
	};

	size_t symboldatachain::get_capacity()
	{
		return (nodes.size() * symboldata_sectorsize) - (symboldata_sectorsize - cursor); 
	}

	bool symboldatachain::_verify_shallow(const string& prefix, symboldataref_t suffix, const group_iterator& iter)
	{
		group_iterator viter = iter;
		size_t len = prefix.length();
		const char* prefixbuf = prefix.c_str();
		size_t offset = 0;
		while (offset < len && iter.is_valid())
		{
			symboldatagroup_t curgroup = *viter;
			switch (last_byte_of(curgroup))
			{
			case 0xff:
				{
					symboldatagroup_t* nextgroup = (viter + 1);
					if (suffix == symboldataref_invalid)
					{
						if (nextgroup != nullptr) return false;
					}
					else
					{
						if (*nextgroup != suffix) return false;
					}
				}
				//fall through
			case 0x00:
				return strncmp(prefixbuf + offset, reinterpret_cast<const char*>(&curgroup), sizeof(symboldatagroup_t) - 1) == 0;
			default:
				if (strncmp(prefixbuf + offset, reinterpret_cast<const char*>(&curgroup), sizeof(symboldatagroup_t)) != 0)
					return false;
				offset += sizeof(symboldatagroup_t);
				viter++;
			}
		}
		return false;
	}

	symboldataref_t symboldatachain::query(const string& prefix, symboldataref_t suffix /* = symboldataref_invalid */)
	{
		for (group_iterator it(this, 0); it.is_valid(); it++)
		{
			if (_verify_shallow(prefix, suffix, it))
				return it.get_ref();
		}
		return symboldataref_invalid;
	}

	void symboldatachain::_addnodeifneeded()
	{
		if (cursor == symboldata_sectorsize)
		{
			nodes.push_back(make_unique<symboldatabuffer_t>());
			cursor = 0;
		}
	}
	
	void symboldatachain::_appendbuffer(const void* buffer, size_t groupcount)
	{
		assert(cursor + groupcount <= symboldata_sectorsize);
		copy_n(reinterpret_cast<const symboldatagroup_t*>(buffer),
			groupcount, (reinterpret_cast<symboldatagroup_t*>((**nodes.rbegin()).buffer) + cursor));
	}

	symboldataref_t symboldatachain::createref(const string& prefix, symboldataref_t suffix /* = symboldataref_invalid */)
	{
		const size_t groupsize = sizeof(symboldatagroup_t);
		size_t prefixlen = prefix.length();
		size_t wholegroupcount = prefixlen / groupsize;
		size_t wholegrouplen = wholegroupcount * groupsize;

		symboldataref_t retvalue = cursor + (nodes.size() * symboldata_sectorsize) - symboldata_sectorsize;

		size_t groupoffset = 0;
		while (groupoffset < wholegroupcount)
		{
			_addnodeifneeded();
			size_t blksize = min(symboldata_sectorsize - cursor, wholegroupcount - groupoffset);
			_appendbuffer(prefix.c_str() + groupoffset * groupsize, blksize);
			groupoffset += blksize;
		}

		symboldatagroup_t restgroupdata = 0;
		copy_n(prefix.c_str() + wholegrouplen, prefixlen - wholegrouplen, reinterpret_cast<uint8_t*>(&restgroupdata));
		last_byte_of(restgroupdata) = suffix != 0 ? 0xFF : 0;
		_addnodeifneeded();
		_appendbuffer(&restgroupdata, 1);

		if (suffix)
		{
			static_assert(sizeof(suffix) == groupsize, "symboldatagroup_t should be the same size with symboldataref_t.");
			_addnodeifneeded();
			_appendbuffer(&suffix, 1);
		}

		return retvalue;
	}
}
}
