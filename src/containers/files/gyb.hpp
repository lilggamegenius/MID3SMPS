#pragma once

#include <cstdint>
#include <vector>

#include "fm/patch.hpp"

namespace MID3SMPS {
struct gyb {
	static constexpr std::array lfo_values = { // Hz
		0., // Off
		3.98,
		5.56,
		6.02,
		6.37,
		6.88,
		9.63,
		48.1,
		72.2
	};
	std::uint8_t default_LFO_speed{}; // Index into lfo_values
	std::vector<fm::patch> patches;
};
}