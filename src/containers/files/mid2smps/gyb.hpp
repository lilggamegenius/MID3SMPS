#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <span>

#include "helpers/list_helper.hpp"
#include "fm/patch.hpp"

namespace MID3SMPS::M2S {
	using ins_key_t = std::uint16_t;
	using bank_key_t = std::uint16_t;
	namespace fs = std::filesystem;
	struct gyb {
		enum class lfo : std::uint8_t{
			off,
			mode0 = 0b1000,
			mode1,
			mode2,
			mode3,
			mode4,
			mode5,
			mode6,
			mode7,
		};
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
		enum bank : bank_key_t{
			melodic,
			drum
		};
		lfo default_LFO_speed{};
		using patch_container_t = std::unordered_map<ins_key_t, fm::patch>;
		using patch_order_t = std::vector<ins_key_t>;
		patch_container_t patches;
		std::unordered_map<bank, std::pair<std::string, patch_order_t>> patches_order;

	private:
		mutable ins_key_t current_id = 0;

		[[nodiscard]] ins_key_t new_unique_id() const {
			while(patches.contains(current_id)) {
				++current_id;
			}
			return current_id;
		}

	public:
		template<typename... Args>
		fm::patch& add_patch(bank selected_bank, Args&&... args) {
			const auto id = new_unique_id();
			fm::patch patch(args...);
			auto [iter, inserted] = patches.emplace(id, std::move(patch));
			if(!inserted) {
				throw std::runtime_error(fmt::format("Failed to add fm::patch with ID {}", id));
			}
			patches_order[selected_bank].second.emplace_back(id);
			return iter->second;
		}

		gyb()								 = default;
		gyb(const gyb &other)                = default;
		gyb(gyb &&other) noexcept            = default;
		gyb &operator=(const gyb &other)     = default;
		gyb &operator=(gyb &&other) noexcept = default;

		explicit gyb(const fs::path &path);

	private:
		void load_v1(std::span<const std::uint8_t> data);
		void load_v2(std::span<const std::uint8_t> data);
		void load_v3(std::span<const std::uint8_t> data);

	public:
		[[nodiscard, gnu::const]] static constexpr auto string(const lfo &mode) {
			using namespace std::string_view_literals;
			using enum lfo;
			switch(mode) {
				case off: return "Off"sv;
				case mode0: return "3.98 Hz"sv;
				case mode1: return "5.56 Hz"sv;
				case mode2: return "6.02 Hz"sv;
				case mode3: return "6.37 Hz"sv;
				case mode4: return "6.88 Hz"sv;
				case mode5: return "9.63 Hz"sv;
				case mode6: return "48.1 Hz"sv;
				case mode7: return "72.2 Hz"sv;
				default: throw std::logic_error(fmt::format("Invalid value for lfo, got {}", std::to_underlying(mode)));
			}
		}
	};
}

template<>
struct MID3SMPS::list_helper<MID3SMPS::M2S::gyb::lfo> {
	using type = std::span<const M2S::gyb::lfo, 9>;
	static constexpr std::array values = {
		M2S::gyb::lfo::off,
		M2S::gyb::lfo::mode0,
		M2S::gyb::lfo::mode1,
		M2S::gyb::lfo::mode2,
		M2S::gyb::lfo::mode3,
		M2S::gyb::lfo::mode4,
		M2S::gyb::lfo::mode5,
		M2S::gyb::lfo::mode6,
		M2S::gyb::lfo::mode7
	};

	[[nodiscard, gnu::const]] static constexpr type list() {
		return values;
	}
};

/*
GYB Version 1/2
===============

Header
------
Pos	Len	Description
--------------------------------
00	02	Signature: 26 12 (both decimal)
02	01	Version
03	01	Instrument Count: Melody Bank
04	01	Instrument Count: Drum Bank
05	100	Instrument Map (Melody
		-> FM instrument IDs for GM instruments/drums
		-> Melody/Drum Bank interleaved (melody 0, drum 0, melody 1, drum 1, ...)
105	01	[GYB v2 only] default LFO Speed
??	??	Instrument Data [Melody Bank]
??	??	Instrument Data [Drum Bank]
??	??	Instrument Names [Melody Bank]
??	??	Instrument Names [Drum Bank]
??	04	Checksum (calculated over whole file minus the checksum itself)
EOF

Instrument Data [v1]
---------------
00	1D	YM2612 Registers
		Order:	30 34 38 3C 40 44 48 4C 50 54 58 5C 60 64 68 6C 70 74 78 7C 80 84 88 8C 90 94 98 9C B0
1D	01	Instrument Transposition (signed 8-bit)
		Drum Bank: default drum note (unsigned 8-bit)

Instrument Data [v2]
---------------
00	1E	YM2612 Registers
		Order:	30 34 38 3C 40 44 48 4C 50 54 58 5C 60 64 68 6C 70 74 78 7C 80 84 88 8C 90 94 98 9C B0 B4
1E	01	Instrument Transposition (same as v1)
1F	01	Padding (set to 0)

Instrument Name
---------------
00	01	String Length n
01	n	String



GYB Version 3
=============

Header
------
Pos	Len	Description
--------------------------------
00	02	Signature: 26 12 (both decimal)
02	01	Version
03	01	default LFO Speed
04	04	File Size
08	04	File Offset: Instrument Banks
0C	04	File Offset: Instrument Maps


Instrument Bank
---------------
00	02	Instrument Count
02	??	Instrument Data

Instrument Data
---------------
00	02	Bytes used by this instrument (includes this value)
02	1E	YM2612 Registers
		Order:	30 34 38 3C 40 44 48 4C 50 54 58 5C 60 64 68 6C 70 74 78 7C 80 84 88 8C 90 94 98 9C B0 B4
20	01	Instrument Transposition (signed 8-bit)
		Drum Bank: default drum note (unsigned 8-bit)
21	01	Bitfield: Additional Data
		Bit 0: Chord notes (additional notes that sound by playing this instrument)
22	??	additional data
??	01	String Length n
??	n	String


Chord notes
-----------
00	01	Number of Notes n
01	01*n	Notes (8-bit signed, relative to initial note)



Instrument Maps
---------------
00	??	Instrument Map: Melody [GM instrument -> FM instrument]
??	??	Instrument Map: Drum [GM drum kit sound -> FM instrument]

Instrument Map
--------------
00	??*80	128 (dec) Map Entries
		[Melody] each entry corresponds to one GM instrument
		[Drum] each entry corresponds to one GM instrument

Instrument Map Entry
--------------------
00	02	Map Sub-Entry Count n
02	04*n	Map Sub-Entries

00	01	Bank MSB (00 = default, FF = all) [Melody] / Drum Kit [Drum]
01	01	Bank LSB (FF = all) [Melody] / unused [Drum]
02	02	FM Instrument (Bit 15 = Drum Bank)

*/
