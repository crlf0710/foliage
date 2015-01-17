#include <string>
#include <memory>
#include <cassert>
#include "symboldata.h"
#include "../utils/mathutils.h"

namespace foliage {
namespace symboldata {
	using std::size_t;
	using std::uint8_t;

	symboldataref_t symboldatachain::query(const std::string& prefix, symboldataref_t suffix /* = symboldataref_invalid */)
	{
		//xx
		return symboldataref_invalid;
	}

	void symboldatachain::_addnodeifneeded()
	{
		if (cursor == FOLIAGE_SYMBOLDATA_SECTORSIZE)
		{
			nodes.push_back(std::make_unique<symboldatabuffer_t>());
			cursor = 0;
		}
	}
	
	void symboldatachain::_appendbuffer(const void* buffer, size_t groupcount)
	{
		assert(cursor + groupcount <= FOLIAGE_SYMBOLDATA_SECTORSIZE);
		std::copy_n(reinterpret_cast<const symboldatagroup_t*>(buffer),
			groupcount, (reinterpret_cast<symboldatagroup_t*>((**nodes.rbegin()).buffer) + cursor));
	}

	symboldataref_t symboldatachain::createref(const std::string& prefix, symboldataref_t suffix /* = symboldataref_invalid */)
	{
		const size_t groupsize = sizeof(symboldatagroup_t);
		size_t prefixlen = prefix.length();
		size_t wholegroupcount = prefixlen / groupsize;
		size_t wholegrouplen = wholegroupcount * groupsize;

		symboldataref_t retvalue = cursor + (nodes.size() * FOLIAGE_SYMBOLDATA_SECTORSIZE) - FOLIAGE_SYMBOLDATA_SECTORSIZE;

		size_t groupoffset = 0;
		while (groupoffset < wholegroupcount)
		{
			_addnodeifneeded();
			size_t blksize = std::min(FOLIAGE_SYMBOLDATA_SECTORSIZE - cursor, wholegroupcount - groupoffset);
			_appendbuffer(prefix.c_str() + groupoffset * groupsize, blksize);
			groupoffset += blksize;
		}

		symboldatagroup_t restgroupdata = 0;
		std::copy_n(prefix.c_str() + wholegrouplen, prefixlen - wholegrouplen, reinterpret_cast<uint8_t*>(&restgroupdata));
		restgroupdata |= suffix != 0 ? 0xFF : 0;
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
