#pragma once

#include <cstdint>
#include <vector>

#include "fm/patch.hpp"

namespace MID3SMPS {
class gyb {
	std::int8_t default_LFO_speed{};
	std::vector<fm::patch> patches;
};
}