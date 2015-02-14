#pragma once

#include <cstdint>
#include "../utils/mathutils.h"

namespace foliage {
namespace utils {

	inline foliage::utils::byte_t helper_bitmap_bytemask(std::size_t startbit, std::size_t endbit)
	{
		return ((1 << endbit) - 1) - ((1 << startbit) - 1);
	}

	template<std::uint8_t bit_value>
	inline void helper_bitmap_byteset(foliage::utils::byte_t* bitmap, std::size_t startpos, std::size_t endpos)
	{
		std::size_t byte_offset = foliage::utils::floordiv<size_t>(startpos, CHAR_BIT);
		foliage::utils::byte_t& dest_byte = bitmap[byte_offset];
		if (bit_value)
			dest_byte |= helper_bitmap_bytemask(startpos - byte_offset * CHAR_BIT, endpos - byte_offset * CHAR_BIT);
		else
			dest_byte &= ~helper_bitmap_bytemask(startpos - byte_offset * CHAR_BIT, endpos - byte_offset * CHAR_BIT);
	}

	template<std::uint8_t bit_value>
	inline void bitmap_fill(foliage::utils::byte_t* bitmap, std::size_t startpos, std::size_t endpos)
	{
		size_t whole_byte_startpos = foliage::utils::ceiltomultiple<size_t>(startpos, CHAR_BIT);
		size_t whole_byte_endpos = foliage::utils::floortomultiple<size_t>(endpos, CHAR_BIT);

		if (whole_byte_startpos <= whole_byte_endpos)
		{
			helper_bitmap_byteset<bit_value>(bitmap, startpos, whole_byte_startpos);
			std::uninitialized_fill(reinterpret_cast<foliage::utils::byte_t*>(bitmap)+whole_byte_startpos / CHAR_BIT,
				reinterpret_cast<foliage::utils::byte_t*>(bitmap)+whole_byte_endpos / CHAR_BIT,
				(foliage::utils::byte_t)(bit_value ? ~0U : 0U));
			helper_bitmap_byteset<bit_value>(bitmap, whole_byte_endpos, endpos);
		}
		else
		{
			helper_bitmap_byteset<bit_value>(bitmap, startpos, endpos);
		}
	}

	template<std::uint8_t bit_value>
	inline void bitmap_fill_n(foliage::utils::byte_t* bitmap, std::size_t startpos, std::size_t length)
	{
		return bitmap_fill<bit_value>(bitmap, startpos, startpos + length);
	}

	inline size_t bitmap_search_aligned_gap(foliage::utils::byte_t* bitmap, std::size_t start, std::size_t end, std::size_t gap_size_power)
	{
		if (gap_size_power < CHAR_BIT * sizeof(std::uintptr_t) || start % (CHAR_BIT * sizeof(std::uintptr_t)) != 0)
		{
			foliage::utils::byte_t* bitmap_ptr = bitmap;
			foliage::utils::byte_t* bitmap_endptr = bitmap_ptr + end / CHAR_BIT;

			if (gap_size_power < CHAR_BIT)
			{
				foliage::utils::byte_t bitmask_item = foliage::utils::leftbitshift<std::uintptr_t>(1, gap_size_power) - 1;
				std::size_t bitmask_bytereciprocal = CHAR_BIT / gap_size_power;
				foliage::utils::byte_t* bitmap_startptr = bitmap_ptr + start / CHAR_BIT;
				for (foliage::utils::byte_t* bitmap_checkptr = bitmap_startptr;
					bitmap_checkptr < bitmap_endptr;
					bitmap_checkptr++)
				{
					foliage::utils::byte_t bitmask_cur = bitmask_item;
					for (std::size_t offset = 0; offset < bitmask_bytereciprocal; offset++, bitmask_cur <<= gap_size_power)
					{
						if ((*bitmap_checkptr & bitmask_cur) == 0)
						{
							return (bitmap_checkptr - bitmap_ptr) * CHAR_BIT + offset * gap_size_power;
						}
					}
				}
			}
			else
			{
				std::size_t bytemaskcount = foliage::utils::leftbitshift<std::size_t>(1, gap_size_power) / CHAR_BIT;
				foliage::utils::byte_t* bitmap_startptr = bitmap_ptr + foliage::utils::ceiltomultiple(start / CHAR_BIT, bytemaskcount);
				for (foliage::utils::byte_t* bitmap_checkptr = bitmap_startptr;
					bitmap_checkptr + bytemaskcount - 1 < bitmap_endptr;
					bitmap_checkptr += bytemaskcount)
				{
					for (std::size_t offset = 0; offset < bytemaskcount; offset++)
					{
						if (bitmap_checkptr[offset] != 0) goto byte_check_fail;
					}
					return (bitmap_checkptr - bitmap_ptr) * CHAR_BIT;
				byte_check_fail:
					/*go on*/;
				}
			}
		}
		else // faster check
		{
			std::uintptr_t* bitmap_ptr = reinterpret_cast<std::uintptr_t*>(bitmap);
			std::uintptr_t* bitmap_endptr = bitmap_ptr + end / (CHAR_BIT * sizeof(std::uintptr_t));

			std::size_t wordmaskcount = foliage::utils::leftbitshift<std::size_t>(1, gap_size_power) / (CHAR_BIT * sizeof(std::uintptr_t));
			std::uintptr_t* bitmap_startptr = bitmap_ptr + foliage::utils::ceiltomultiple(start / (CHAR_BIT * sizeof(std::uintptr_t)), wordmaskcount);

			for (std::uintptr_t* bitmap_checkptr = bitmap_startptr;
				bitmap_checkptr + wordmaskcount - 1 < bitmap_endptr;
				bitmap_checkptr += wordmaskcount)
			{
				for (std::size_t offset = 0; offset < wordmaskcount; offset++)
				{
					if (bitmap_checkptr[offset] != 0) goto word_check_fail;
				}
				return (bitmap_checkptr - bitmap_ptr) * CHAR_BIT * sizeof(std::uintptr_t);
			word_check_fail:
				/*go on*/;
			}
		}

		return (size_t)-1;
	}

	inline size_t bitmap_find_largest_gap(foliage::utils::byte_t* bitmap, size_t begin_pos, size_t size_power_min, size_t size_power_max)
	{
		size_t size_power = size_power_min + 1;
		bool word_check_unavailable = begin_pos % (CHAR_BIT * sizeof(std::uintptr_t)) != 0;
		for (; size_power <= size_power_max; size_power++)
		{
			if (word_check_unavailable || size_power < CHAR_BIT * sizeof(std::uintptr_t))
			{
				if (size_power < CHAR_BIT)
				{
					foliage::utils::byte_t bitmask_item = 
						(foliage::utils::leftbitshift<std::uintptr_t>(1, size_power + 1) - 1) << (begin_pos % CHAR_BIT);
					if ((bitmap[begin_pos / CHAR_BIT] & bitmask_item) != 0) goto check_done;
				}
				else
				{
					std::size_t bytemaskcount = foliage::utils::rightbitshift<std::size_t>(size_power, CHAR_BIT);
					foliage::utils::byte_t* bitmap_bytes = bitmap + (begin_pos / CHAR_BIT);
					for (std::size_t idx = 0; idx < bytemaskcount; idx++)
						if (bitmap_bytes[idx] != 0) goto check_done;
				}
			}
			else
			{
				std::size_t wordmaskcount = foliage::utils::rightbitshift(size_power, CHAR_BIT * sizeof(std::uintptr_t));
				std::uintptr_t* bitmap_words = reinterpret_cast<std::uintptr_t*>(bitmap) 
					+ (begin_pos / (CHAR_BIT * sizeof(std::uintptr_t)));
				for (std::size_t idx = 0; idx < wordmaskcount; idx++)
					if (bitmap_words[idx] != 0) goto check_done;
			}
		}
check_done:
		return size_power - 1;
	}

}
}
