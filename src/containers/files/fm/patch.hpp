#pragma once

#include <cstdint>
#include <span>

#include "operators.hpp"

namespace MID3SMPS::fm {
	struct options {
		bool chord_notes: 1;
	};

	struct chords {
		std::uint8_t note_count;
		std::span<std::int8_t> relative_notes;
	};

	struct patch {
	private:
		std::string name_{};
		std::uint16_t total_size_{};
		operators operators_{};
		std::int8_t instrument_transposition_{};
		options options_{};
		chords chord_notes_{};
	};
}
