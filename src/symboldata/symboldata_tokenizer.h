#pragma once

#include "symboldata.h"

namespace foliage {
namespace symboldata {

	enum class symbol_char_category
	{
		normal,
		breakable,
		forcebreak,
	};

	static inline symbol_char_category get_symbol_char_category(const char _chr)
	{
		switch (_chr)
		{
		case 'a': case 'e': case 'i':
		case 'o': case 'u':
			return symbol_char_category::breakable;
		case '.':
			return symbol_char_category::forcebreak;
		default:
			return symbol_char_category::normal;
		}
	}

	class symboldata_token_reviterator
	{
	protected:
		const std::string& databuf;
		size_t endpos;
		size_t charcount;

	public:
		symboldata_token_reviterator(const std::string& _str, size_t _endpos) 
			: databuf(_str), endpos(_endpos) 
		{
			_updatecharcount();
		}

		bool operator!=(const symboldata_token_reviterator& end)
		{
			assert(&this->databuf == &end.databuf);

			return this->endpos != end.endpos;
		}

		symboldata_token_reviterator& operator ++ ()
		{
			endpos -= charcount;
			_updatecharcount();
			return *this;
		}

		std::string operator*()
		{
			assert(endpos != 0);
			return databuf.substr(endpos - charcount, charcount);
		}

		void _updatecharcount()
		{
			charcount = 0;

			auto start_iter = databuf.crbegin() + databuf.length() - endpos;
			auto limit_iter = databuf.crend();
			if (start_iter == limit_iter)
				return;

			for (auto end_iter = start_iter; end_iter != limit_iter; end_iter++)
			{
				auto category = get_symbol_char_category(*end_iter);
				size_t curlen = end_iter - start_iter + 1;
				if (curlen >= sizeof(symboldatagroup_t))
				{
					if (charcount != 0) return;

					if (category != symbol_char_category::normal)
					{
						charcount = curlen;
						return;
					}
				}
				else
				{
					if (category != symbol_char_category::normal)
					{
						charcount = curlen;
						if (category == symbol_char_category::forcebreak) return;
					}
				}
			}
			if (charcount == 0) charcount = limit_iter - start_iter;
		}
	};

	class symboldata_reverse_tokenizer
	{
	protected:
		const std::string& databuf;

	public:
		symboldata_reverse_tokenizer(const std::string& _str) : databuf(_str) {}

		symboldata_token_reviterator begin()
		{
			return symboldata_token_reviterator(databuf, databuf.length());
		}

		symboldata_token_reviterator end()
		{
			return symboldata_token_reviterator(databuf, 0);
		}
	};
}
}
