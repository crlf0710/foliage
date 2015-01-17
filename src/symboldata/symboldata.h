#include <climits>
#include <deque>
#include <algorithm>
#include "../utils/varsizeint.h"

namespace foliage {
namespace symboldata {
	enum
	{
		FOLIAGE_SYMBOLDATA_SECTORSIZE = 1024,

	};

	typedef std::uintptr_t symboldatagroup_t;
	typedef std::uintptr_t symboldataref_t;

	struct symboldatabuffer
	{
		symboldatagroup_t buffer[FOLIAGE_SYMBOLDATA_SECTORSIZE];
	};
	typedef symboldatabuffer symboldatabuffer_t;
	
	
	const symboldataref_t symboldataref_invalid = UINTPTR_MAX;
	class symboldatachain
	{
	public:
		symboldatachain()
			: cursor(FOLIAGE_SYMBOLDATA_SECTORSIZE)
		{

		}

		symboldataref_t intern(const std::string& prefix, symboldataref_t suffix = symboldataref_invalid)
		{
			symboldataref_t ref = query(prefix, suffix);
			if (ref == symboldataref_invalid) ref = createref(prefix, suffix);

			return ref;
		}

	protected:
		void _addnodeifneeded();
		void _appendbuffer(const void* buffer, size_t groupcount);

		symboldataref_t query(const std::string& prefix, symboldataref_t suffix = symboldataref_invalid);
		symboldataref_t createref(const std::string& prefix, symboldataref_t suffix = symboldataref_invalid);

		std::deque < std::unique_ptr<symboldatabuffer_t> > nodes;
		std::uintptr_t cursor;
	};

	typedef symboldatachain symboldatachain_t;



}
}