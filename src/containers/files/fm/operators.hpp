#pragma once

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <fmt/core.h>

namespace MID3SMPS::fm {
	struct operators {
		static constexpr std::uint8_t instrument_register_size = 0x1E;
		std::array<std::uint8_t, instrument_register_size> registers{};

		enum op_id : std::uint8_t {
			op1 = 0,
			op2 = 2,
			op3 = 1,
			op4 = 3
		};
		enum class detune_mode : std::uint8_t {
			// ReSharper disable CppInconsistentNaming
			no_change_1,
			plus_e,
			plus_2e,
			plus_3e,
			no_change_2,
			minus_e,
			minus_2e,
			minus_3e
			// ReSharper restore CppInconsistentNaming
		};
		enum class rate_scaling_mode : std::uint8_t {
			kc8,
			kc4,
			kc2,
			kc1
		};
		enum class ssgeg_mode : std::uint8_t {
			disabled,
			mode0 = 0b1000,
			mode1,
			mode2,
			mode3,
			mode4,
			mode5,
			mode6,
			mode7
		};

		struct masks {
			static constexpr std::uint8_t detune = 0xF0;
			static constexpr std::uint8_t multiple = 0x0F;
			static constexpr std::uint8_t total_level = 0x7F;
			static constexpr std::uint8_t rate_scaling = 0xC0;
			static constexpr std::uint8_t attack_rate = 0x3F;
			static constexpr std::uint8_t amplitude_modulation = 0x80;
			static constexpr std::uint8_t decay_rate = 0x1F;
			static constexpr std::uint8_t sustain_rate = 0x1F;
			static constexpr std::uint8_t sustain_level = 0xF0;
			static constexpr std::uint8_t release_rate = 0x0F;
			static constexpr std::uint8_t ssgeg_mode = 0x0F;

			masks() = delete;

			static_assert(detune == static_cast<std::uint8_t>(~multiple));
			static_assert(rate_scaling == static_cast<std::uint8_t>(~attack_rate));
			//static_assert(amplitude_modulation == static_cast<std::uint8_t>(~decay_rate)); // 2 bits unused, so not equal
		};

		[[nodiscard]] constexpr detune_mode detune(const op_id &op) const {
			auto value = registers[op + (0 * 4)];
			value >>= 4;
			return static_cast<detune_mode>(value);
		}
		constexpr void detune(const op_id &op, const detune_mode mode) {
			auto value = std::to_underlying(mode);
			value <<= 4;
			auto &reg = registers[op + (0 * 4)];
			reg &= masks::multiple;
			reg |= value;
		}

		[[nodiscard]] constexpr std::uint8_t multiple(const op_id &op) const {
			const auto &value = registers[op + (0 * 4)];
			return value & masks::multiple;
		}
		constexpr void multiple(const op_id &op, const std::uint8_t new_multiple) {
			auto value = new_multiple;
			value &= masks::multiple;
			auto &reg = registers[op + (0 * 4)];
			reg &= masks::detune;
			reg |= value;
		}

		[[nodiscard]] constexpr std::uint8_t total_level(const op_id &op) const {
			const auto &value = registers[op + (1 * 4)];
			return value & masks::total_level;
		}
		constexpr void total_level(const op_id &op, const std::uint8_t new_total_level) {
			auto value = new_total_level;
			value &= masks::total_level;
			registers[op + (1 * 4)] = value;
		}

		[[nodiscard]] constexpr rate_scaling_mode rate_scaling(const op_id &op) const {
			auto value = registers[op + (2 * 4)];
			value >>= 6;
			return static_cast<rate_scaling_mode>(value);
		}
		constexpr void rate_scaling(const op_id &op, const rate_scaling_mode mode) {
			auto value = std::to_underlying(mode);
			value <<= 6;
			auto &reg = registers[op + (2 * 4)];
			reg &= masks::attack_rate;
			reg |= value;
		}

		[[nodiscard]] constexpr std::uint8_t attack_rate(const op_id &op) const {
			const auto &value = registers[op + (2 * 4)];
			return value & masks::attack_rate;
		}
		constexpr void attack_rate(const op_id &op, const std::uint8_t new_attack_rate) {
			auto value = new_attack_rate;
			value &= masks::attack_rate;
			auto &reg = registers[op + (2 * 4)];
			reg &= masks::rate_scaling;
			reg |= value;
		}

		[[nodiscard]] constexpr bool amplitude_modulation(const op_id &op) const {
			auto value = registers[op + (3 * 4)];
			value &= masks::amplitude_modulation;
			return value == masks::amplitude_modulation;
		}
		constexpr void amplitude_modulation(const op_id &op, bool enabled) {
			auto &reg = registers[op + (3 * 4)];
			if(enabled) {
				reg |= masks::amplitude_modulation;
			} else {
				reg &= masks::amplitude_modulation;
				//reg &= 0b10011111; // ANDing off the last 3 bits instead of only the last 1 because 2 bits go unused
			}
		}

		[[nodiscard]] constexpr std::uint8_t decay_rate(const op_id &op) const {
			const auto &reg = registers[op + (3 * 4)];
			return reg & masks::decay_rate;
		}
		constexpr void decay_rate(const op_id &op, const std::uint8_t new_decay_rate) {
			auto value = new_decay_rate;
			value &= masks::decay_rate;
			auto &reg = registers[op + (3 * 4)];
			static constexpr auto flipped_mask = static_cast<std::uint8_t>(~masks::decay_rate);
			reg &= flipped_mask;
			reg |= value;
		}

		[[nodiscard]] constexpr std::uint8_t sustain_rate(const op_id &op) const {
			const auto &reg = registers[op + (4 * 4)];
			return reg & masks::sustain_rate;
		}
		constexpr void sustain_rate(const op_id &op, const std::uint8_t new_sustain_rate) {
			auto &reg = registers[op + (4 * 4)];
			reg = new_sustain_rate & masks::sustain_rate;
		}

		[[nodiscard]] constexpr std::uint8_t sustain_level(const op_id &op) const {
			const auto &reg = registers[op + (5 * 4)];
			return reg >> 4;
		}
		constexpr void sustain_level(const op_id &op, const std::uint8_t new_sustain_level) {
			auto value = new_sustain_level;
			value <<= 4;
			auto &reg = registers[op + (5 * 4)];
			reg &= masks::release_rate;
			reg |= value;
		}

		[[nodiscard]] constexpr std::uint8_t release_rate(const op_id &op) const {
			const auto &reg = registers[op + (5 * 4)];
			return reg & masks::release_rate;
		}
		constexpr void release_rate(const op_id &op, const std::uint8_t new_release_rate) {
			auto value = new_release_rate;
			value &= masks::release_rate;
			auto &reg = registers[op + (5 * 4)];
			reg &= masks::sustain_level;
			reg |= value;
		}

		[[nodiscard]] constexpr ssgeg_mode ssgeg(const op_id &op) const {
			auto value = registers[op + (6 * 4)];
			[[assume(value <= masks::ssgeg_mode)]];
			//value &= masks::ssgeg_mode;
			return static_cast<ssgeg_mode>(value);
		}
		constexpr void ssgeg(const op_id &op, const ssgeg_mode mode) {
			auto &reg = registers[op + (6 * 4)];
			reg = std::to_underlying(mode);
		}

		// Todo: add AMS, FMS, feedback, and algorithm fields. lives in last 2 bytes

		[[nodiscard]] static constexpr auto string(const op_id &id) {
			using namespace std::string_view_literals;
			switch(id) {
				case op1: return "Operator 1"sv;
				case op2: return "Operator 2"sv;
				case op3: return "Operator 3"sv;
				case op4: return "Operator 4"sv;
				default: return ""sv;
			}
		}

		[[nodiscard]] static constexpr auto string(const detune_mode &mode) {
			using namespace std::string_view_literals;
			using enum detune_mode;
			switch(mode) {
				case no_change_1:
				case no_change_2: return "No Change"sv;
				case plus_e: return "× (1+e)"sv;
				case plus_2e: return "× (1+2e)"sv;
				case plus_3e: return "× (1+3e)"sv;
				case minus_e: return "× (1-e)"sv;
				case minus_2e: return "× (1-2e)"sv;
				case minus_3e: return "× (1-3e)"sv;
				default: return ""sv;
			}
		}

		[[nodiscard]] static constexpr auto string(const rate_scaling_mode &mode) {
			using namespace std::string_view_literals;
			using enum rate_scaling_mode;
			switch(mode) {
				case kc8: return "2 × Rate + (KC/8)"sv;
				case kc4: return "2 × Rate + (KC/4)"sv;
				case kc2: return "2 × Rate + (KC/2)"sv;
				case kc1: return "2 × Rate + (KC/1)"sv;
				default: return ""sv;
			}
		}

		[[nodiscard]] static constexpr auto string(const ssgeg_mode &mode) {
			using namespace std::string_view_literals;
			using enum ssgeg_mode;
			switch(mode) {
				case disabled: return "Disabled"sv;
				case mode0: return R"(\\\\)"sv;
				case mode1: return R"(\___)"sv;
				case mode2: return R"(\/\/)"sv;
				case mode3: return R"(\‾‾‾)"sv;
				case mode4: return R"(////)"sv;
				case mode5: return R"(/‾‾‾)"sv;
				case mode6: return R"(/\/\)"sv;
				case mode7: return R"(/___)"sv;
				default: return ""sv;
			}
		}

		[[nodiscard]] static constexpr std::size_t operator_to_index(const op_id &op) {
			if(op > op4) {
				throw std::logic_error(fmt::format("Invalid index passed to operator_to_index. {} outside of range of 0-3", string(op)));
			}
			return op;
		}
	};
	static_assert(sizeof(operators) == operators::instrument_register_size);
}
