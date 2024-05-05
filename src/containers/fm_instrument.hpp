#pragma once

#include "instrument.hpp"
#include "chips/ym2612/operators.hpp"

namespace MID3SMPS {
	struct fm_instrument : instrument {
		ym2612::operators operators{};

		constexpr fm_instrument()           = default;
		constexpr ~fm_instrument() override = default;
	};
}
