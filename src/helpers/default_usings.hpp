#pragma once

#include "safe_int.hpp"

namespace MID3SMPS {
	using bit_8			= safe_int<std::uint8_t>;
	using bit_16		= safe_int<std::uint16_t>;
	using bit_32		= safe_int<std::uint32_t>;
	using bit_64		= safe_int<std::uint64_t>;
	//using bit_128	= safe_int<unsigned __int128>;

	using bit_8s		= safe_int<std::int8_t>;
	using bit_16s		= safe_int<std::int16_t>;
	using bit_32s		= safe_int<std::int32_t>;
	using bit_64s		= safe_int<std::int64_t>;
	//using bit_128s	= safe_int<__int128>;

	using bit_ptr		= safe_int<std::uintptr_t>;
}