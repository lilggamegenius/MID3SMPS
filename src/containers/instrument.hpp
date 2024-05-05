#pragma once

#include <string>

namespace MID3SMPS {
	struct instrument {
		std::string name{};

		constexpr instrument()          = default;
		constexpr virtual ~instrument() = default;
	};
}
