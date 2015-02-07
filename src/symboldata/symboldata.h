#pragma once 

#include <string>
#include <memory>
#include <cassert>
#include <cstring>
#include <climits>
#include <deque>
#include <algorithm>
#include "../utils/varsizeint.h"
#include "../utils/sysutils.h"


namespace foliage {
namespace symboldata {
	enum
	{
		symboldata_sectorsize = 1024,

	};

	typedef std::uint32_t symboldatagroup_t, symboldataref_t, symboldataoffset_t;

	struct symboldatabuffer
	{
		symboldatagroup_t buffer[symboldata_sectorsize];
	};
	typedef symboldatabuffer symboldatabuffer_t;
	
	const symboldataref_t symboldataref_invalid = std::numeric_limits<symboldataref_t>::max();

	template <class _alloc = std::allocator<void>>
	class symboldatachain
	{
	public:
		typedef std::unique_ptr<symboldatabuffer_t> _chainnodetype;
		typedef typename std::allocator_traits<_alloc>::template rebind_alloc<_chainnodetype> _chainnodealloc;
		typedef std::deque < std::unique_ptr<symboldatabuffer_t>, _chainnodealloc> _chaintype;

		class group_iterator;
	public:
		symboldatachain() : symboldatachain(_alloc()) {}
		symboldatachain(const _alloc& _allocator)
			: allocator(_allocator)
			, nodes(_chainnodealloc(_allocator))
			, cursor(symboldata_sectorsize)
		{

		}

		size_t get_capacity();

		symboldataref_t intern(const std::string& prefix, symboldataref_t suffix = symboldataref_invalid);

	protected:
		void _addnodeifneeded();
		void _appendbuffer(const void* buffer, size_t groupcount);

		//bool _verify(const std::string& prefix, symboldataref_t suffix, const group_iterator& iter);
		bool _verify_shallow(const std::string& prefix, symboldataref_t suffix, const group_iterator& iter);

		symboldataref_t query(const std::string& prefix, symboldataref_t suffix = symboldataref_invalid);
		symboldataref_t createref(const std::string& prefix, symboldataref_t suffix = symboldataref_invalid);

		_alloc				allocator;
		_chaintype			nodes;
		symboldataoffset_t	cursor;
	};

	typedef symboldatachain<> symboldatachain_t;



}
}


