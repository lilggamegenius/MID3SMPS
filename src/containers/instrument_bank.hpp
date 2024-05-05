#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <fmt/core.h>

#include "instrument.hpp"
#include "chips/ym2612/operators.hpp"

namespace MID3SMPS {
	using ins_key_t = std::uint16_t;
	using bank_key_t = std::uint16_t;

	struct instrument_bank {
		using ins_container_t = std::unordered_map<ins_key_t, std::unique_ptr<instrument>>;
		using bank_container_t = std::unordered_map<bank_key_t, std::string>;
		using ins_order_t = std::vector<ins_key_t>;
		using bank_order_t = std::vector<bank_key_t>;
		using ins_bank_t = std::unordered_map<bank_key_t, ins_order_t>;

		ins_container_t instruments{};
		bank_container_t banks{};

		ins_bank_t instruments_order{};
		bank_order_t bank_order{};

	protected:
		mutable ins_key_t current_ins_id = 0;
		mutable bank_key_t current_bank_id = 0;

		[[nodiscard]] ins_key_t new_unique_ins_id() const {
			while(instruments.contains(current_ins_id)) {
				++current_ins_id;
			}
			return current_ins_id;
		}

		[[nodiscard]] bank_key_t new_unique_bank_id() const {
			while(banks.contains(current_bank_id)) {
				++current_bank_id;
			}
			return current_bank_id;
		}

	public:
		[[nodiscard]] bank_key_t add_bank(const bank_container_t::mapped_type& new_bank_name);
		instrument& add_instrument(ins_container_t::mapped_type &&instrument, const bank_key_t &selected_bank);
		instrument_bank()			= default;
		//virtual ~instrument_bank()	= default; // Causes an error with patch_container_t about std::construct_at(__p, std::forward<_Args>(__args)...);

		[[nodiscard, gnu::const]] static constexpr auto string(const ym2612::lfo &mode) {
			using namespace std::string_view_literals;
			using enum ym2612::lfo;
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
