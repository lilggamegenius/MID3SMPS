#include "instrument_bank.hpp"

namespace MID3SMPS{
	bank_key_t instrument_bank::add_bank(const bank_container_t::mapped_type &new_bank_name) {
		for(const auto &[bank_id, bank_name] : banks) {
			constexpr auto case_insensitive_compare = [](const unsigned char lhs, const unsigned char rhs) noexcept{
				return std::tolower(lhs) == std::tolower(rhs);
			};
			if(std::ranges::equal(bank_name, new_bank_name, case_insensitive_compare)) {
				return bank_id;
			}
		}
		const auto id = new_unique_bank_id();
		auto [_, inserted] = banks.try_emplace(id, new_bank_name);
		if(!inserted) {
			throw std::runtime_error(fmt::format("Failed to add bank with ID {}", id));
		}
		bank_order.push_back(id);
		return id;
	}

	instrument& instrument_bank::add_instrument(ins_container_t::mapped_type &&instrument, const bank_key_t &selected_bank) {
		const auto id = new_unique_ins_id();
		auto [iter, inserted] = instruments.emplace(id, std::move(instrument));
		if(!inserted) {
			throw std::runtime_error(fmt::format("Failed to add instrument with ID {}", id));
		}
		instruments_order[selected_bank].emplace_back(id);
		return *iter->second;
	}
}
