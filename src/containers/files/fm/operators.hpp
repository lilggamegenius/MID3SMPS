#pragma once

#include <array>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <fmt/core.h>

namespace MID3SMPS::fm {
	using namespace std::string_view_literals;
	struct operators {
		using register_t                                     = std::uint8_t;
		static constexpr register_t instrument_register_size = 0x1E;
		std::array<register_t, instrument_register_size> registers{};

		enum class op_id : register_t {
			op1 = 0,
			op2 = 2,
			op3 = 1,
			op4 = 3
		};

		[[nodiscard, gnu::pure]] constexpr const auto &reg(const op_id &op, const register_t &upper_nibble) const {
			return registers[std::to_underlying(op) + (upper_nibble * 4u)];
		}

		[[nodiscard, gnu::pure]] constexpr auto &reg(const op_id &op, const register_t &upper_nibble) {
			return registers[std::to_underlying(op) + (upper_nibble * 4u)];
		}

		enum class detune_mode : register_t {
			// ReSharper disable CppInconsistentNaming
			no_change_1,
			plus_e,
			plus_2e,
			plus_3e,
			no_change_2,
			minus_e,
			minus_2e,
			minus_3e,
			// ReSharper restore CppInconsistentNaming
		};
		enum class rate_scaling_mode : register_t {
			kc8,
			kc4,
			kc2,
			kc1,
		};
		enum class ssgeg_mode : register_t {
			disabled,
			mode0 = 0b1000,
			mode1,
			mode2,
			mode3,
			mode4,
			mode5,
			mode6,
			mode7,
		};
		enum class feedback_mode : register_t {
			off,
			pi_div_16,
			pi_div_8,
			pi_div_4,
			pi_div_2,
			pi,
			pi_mul_2,
			pi_mul_4,
		};
		enum class algorithm_mode : register_t {
			mode0,
			mode1,
			mode2,
			mode3,
			mode4,
			mode5,
			mode6,
			mode7
		};

		struct masks {
			static constexpr register_t detune               = 0xF0;
			static constexpr register_t multiple             = 0x0F;
			static constexpr register_t total_level          = 0x7F;
			static constexpr register_t rate_scaling         = 0xC0;
			static constexpr register_t attack_rate          = 0x1F;
			static constexpr register_t amplitude_modulation = 0x80;
			static constexpr register_t decay_rate           = 0x1F;
			static constexpr register_t sustain_rate         = 0x1F;
			static constexpr register_t sustain_level        = 0xF0;
			static constexpr register_t release_rate         = 0x0F;
			static constexpr register_t ssgeg_mode           = 0x0F;
			static constexpr register_t ams                  = 0x30;
			static constexpr register_t fms                  = 0x07;
			static constexpr register_t feedback             = 0x38;
			static constexpr register_t algorithm            = 0x07;

			masks() = delete;

			static_assert(detune == static_cast<register_t>(~multiple));
			static_assert((rate_scaling | attack_rate) == 0b11011111);
			static_assert((amplitude_modulation | decay_rate) == 0b10011111);
			static_assert(sustain_level == static_cast<register_t>(~release_rate));
			static_assert((ams | fms) == 0b00110111);
			static_assert((feedback | algorithm) == 0b00111111);
		};

		[[nodiscard, gnu::pure]] constexpr detune_mode detune(const op_id &op) const {
			auto value = reg(op, 0);
			value >>= 4;
			return static_cast<detune_mode>(value);
		}

		constexpr void detune(const op_id &op, const detune_mode mode) {
			auto value = std::to_underlying(mode);
			value <<= 4;
			auto &detune_reg = reg(op, 0);
			detune_reg &= masks::multiple;
			detune_reg |= value;
		}

		[[nodiscard, gnu::pure]] constexpr register_t multiple(const op_id &op) const {
			const auto &value = reg(op, 0);
			return value & masks::multiple;
		}

		constexpr void multiple(const op_id &op, const register_t new_multiple) {
			const auto &value  = std::min(new_multiple, masks::multiple); //value &= masks::multiple;
			auto &multiple_reg = reg(op, 0);
			multiple_reg &= masks::detune;
			multiple_reg |= value;
		}

		[[nodiscard, gnu::pure]] constexpr register_t total_level(const op_id &op) const {
			const auto &value = reg(op, 1);
			return value & masks::total_level;
		}

		constexpr void total_level(const op_id &op, const register_t new_total_level) {
			const auto &value = std::min(new_total_level, masks::total_level); //value &= masks::total_level;
			reg(op, 1)        = value;
		}

		[[nodiscard, gnu::pure]] constexpr rate_scaling_mode rate_scaling(const op_id &op) const {
			auto value = reg(op, 2);
			value >>= 6;
			return static_cast<rate_scaling_mode>(value);
		}

		constexpr void rate_scaling(const op_id &op, const rate_scaling_mode mode) {
			auto value = std::to_underlying(mode);
			value <<= 6;
			auto &rs_reg = reg(op, 2);
			rs_reg &= masks::attack_rate;
			rs_reg |= value;
		}

		[[nodiscard, gnu::pure]] constexpr register_t attack_rate(const op_id &op) const {
			const auto &value = reg(op, 2);
			return value & masks::attack_rate;
		}

		constexpr void attack_rate(const op_id &op, const register_t new_attack_rate) {
			const auto &value = std::min(new_attack_rate, masks::attack_rate); //value &= masks::attack_rate;
			auto &ar_reg      = reg(op, 2);
			ar_reg &= masks::rate_scaling;
			ar_reg |= value;
		}

		[[nodiscard, gnu::pure]] constexpr bool amplitude_modulation(const op_id &op) const {
			auto value = reg(op, 3);
			value &= masks::amplitude_modulation;
			return value == masks::amplitude_modulation;
		}

		constexpr void amplitude_modulation(const op_id &op, const bool enabled) {
			auto &am_reg = reg(op, 3);
			if(enabled) {
				am_reg |= masks::amplitude_modulation;
			} else {
				am_reg &= masks::amplitude_modulation;
				//reg &= 0b10011111; // ANDing off the last 3 bits instead of only the last 1 because 2 bits go unused
			}
		}

		[[nodiscard, gnu::pure]] constexpr register_t decay_rate(const op_id &op) const {
			const auto &dr_reg = reg(op, 3);
			return dr_reg & masks::decay_rate;
		}

		constexpr void decay_rate(const op_id &op, const register_t new_decay_rate) {
			const auto &value                  = std::min(new_decay_rate, masks::decay_rate); //value &= masks::decay_rate;
			auto &dr_reg                       = reg(op, 3);
			static constexpr auto flipped_mask = static_cast<register_t>(~masks::decay_rate); // Bypass integer promotion
			dr_reg &= flipped_mask;
			dr_reg |= value;
		}

		[[nodiscard, gnu::pure]] constexpr register_t sustain_rate(const op_id &op) const {
			const auto &sr_reg = reg(op, 4);
			return sr_reg & masks::sustain_rate;
		}

		constexpr void sustain_rate(const op_id &op, const register_t new_sustain_rate) {
			auto &sr_reg = reg(op, 4);
			sr_reg       = std::min(new_sustain_rate, masks::sustain_rate);
		}

		[[nodiscard, gnu::pure]] constexpr register_t sustain_level(const op_id &op) const {
			const auto &sl_reg = reg(op, 5);
			return sl_reg >> 4;
		}

		constexpr void sustain_level(const op_id &op, const register_t new_sustain_level) {
			auto value = std::min(new_sustain_level, static_cast<register_t>(0x0F));
			value <<= 4;
			auto &sl_reg = reg(op, 5);
			sl_reg &= masks::release_rate;
			sl_reg |= value;
		}

		[[nodiscard, gnu::pure]] constexpr register_t release_rate(const op_id &op) const {
			const auto &rr_reg = reg(op, 5);
			return rr_reg & masks::release_rate;
		}

		constexpr void release_rate(const op_id &op, const register_t new_release_rate) {
			const auto &value = std::min(new_release_rate, masks::release_rate);
			//value &= masks::release_rate;
			auto &rr_reg = reg(op, 5);
			rr_reg &= masks::sustain_level;
			rr_reg |= value;
		}

		[[nodiscard, gnu::pure]] constexpr ssgeg_mode ssgeg(const op_id &op) const {
			auto value = reg(op, 6);
			[[assume(value <= masks::ssgeg_mode)]];
			//value &= masks::ssgeg_mode;
			return static_cast<ssgeg_mode>(value);
		}

		constexpr void ssgeg(const op_id &op, const ssgeg_mode mode) {
			auto &ssgeg_reg = reg(op, 6);
			ssgeg_reg       = std::to_underlying(mode);
		}

		static constexpr std::array ams_values = {
			0.,
			1.4,
			5.9,
			11.8
		};

		[[nodiscard, gnu::pure]] constexpr register_t ams() const {
			const auto &lfo_sensitivity_reg = reg(op_id::op3, 7);
			std::uint8_t val                        = lfo_sensitivity_reg & masks::ams;
			val >>= 4;
			return val;
		}

		constexpr void ams(const register_t new_ams_val) {
			auto val = std::min(new_ams_val, static_cast<register_t>(3));
			val <<= 4;
			auto &lfo_sensitivity_reg = reg(op_id::op3, 7);
			lfo_sensitivity_reg &= masks::fms;
			lfo_sensitivity_reg |= val;
		}

		static constexpr std::array fms_values = {
			0.,
			3.4,
			6.7,
			10.,
			14.,
			20.,
			40.,
			80.
		};

		[[nodiscard, gnu::pure]] constexpr register_t fms() const {
			const auto &lfo_sensitivity_reg = reg(op_id::op3, 7);
			return lfo_sensitivity_reg & masks::fms;
		}

		constexpr void fms(const register_t new_fms_val) {
			const auto &val           = std::min(new_fms_val, masks::fms);
			auto &lfo_sensitivity_reg = reg(op_id::op3, 7);
			lfo_sensitivity_reg &= masks::ams;
			lfo_sensitivity_reg |= val;
		}

		[[nodiscard, gnu::pure]] constexpr feedback_mode feedback() const {
			const auto &feedback_algo_reg = reg(op_id::op1, 7);
			return static_cast<feedback_mode>(feedback_algo_reg >> 3);
		}

		constexpr void feedback(const feedback_mode new_feedback_mode) {
			auto val = std::to_underlying(new_feedback_mode);
			val <<= 3;
			val &= masks::feedback;
			auto &feedback_algo_reg = reg(op_id::op1, 7);
			feedback_algo_reg &= masks::algorithm;
			feedback_algo_reg |= val;
		}

		[[nodiscard, gnu::pure]] constexpr algorithm_mode algorithm() const {
			const auto &feedback_algo_reg = reg(op_id::op1, 7);
			return static_cast<algorithm_mode>(feedback_algo_reg & masks::algorithm);
		}

		constexpr void algorithm(const algorithm_mode new_algorithm_mode) {
			auto val = std::to_underlying(new_algorithm_mode);
			val &= masks::algorithm;
			auto &feedback_algo_reg = reg(op_id::op1, 7);
			feedback_algo_reg &= masks::feedback;
			feedback_algo_reg |= val;
		}

		[[nodiscard, gnu::const]] static constexpr auto string(const op_id &id) {
			using enum op_id;
			switch(id) {
				case op1: return "Operator 1"sv;
				case op2: return "Operator 2"sv;
				case op3: return "Operator 3"sv;
				case op4: return "Operator 4"sv;
				default: throw std::logic_error(fmt::format("Invalid value for op_id, got {}", std::to_underlying(id)));
			}
		}

		[[nodiscard, gnu::const]] static constexpr auto string(const detune_mode &mode) {
			using namespace std::string_view_literals;
			using enum detune_mode;
			switch(mode) {
				case no_change_1:
				case no_change_2: return "No Change"sv;
				case plus_e: return "+1 × E"sv;
				case plus_2e: return "+2 × E"sv;
				case plus_3e: return "+3 × E"sv;
				case minus_e: return "-1 × E"sv;
				case minus_2e: return "-2 × E"sv;
				case minus_3e: return "-3 × E"sv;
				default: throw std::logic_error(fmt::format("Invalid value for detune_mode, got {}", std::to_underlying(mode)));
			}
		}

		[[nodiscard, gnu::const]] static constexpr auto string(const rate_scaling_mode &mode) {
			using namespace std::string_view_literals;
			using enum rate_scaling_mode;
			switch(mode) {
				case kc8: return "(KC/8) + (2×Rate)"sv;
				case kc4: return "(KC/4) + (2×Rate)"sv;
				case kc2: return "(KC/2) + (2×Rate)"sv;
				case kc1: return "(KC/1) + (2×Rate)"sv;
				default: throw std::logic_error(fmt::format("Invalid value for rate_scaling_mode, got {}", std::to_underlying(mode)));
			}
		}

		[[nodiscard, gnu::const]] static constexpr auto string(const ssgeg_mode &mode) {
			using namespace std::string_view_literals;
			using enum ssgeg_mode;
			switch(mode) {
				case disabled: return "Disabled"sv;
				case mode0: return R"(\\\\)"sv;
				case mode1: return R"(\___)"sv;
				case mode2: return R"(\/\/)"sv;
				case mode3: return R"(\¯¯¯)"sv;
				case mode4: return R"(////)"sv;
				case mode5: return R"(/¯¯¯)"sv;
				case mode6: return R"(/\/\)"sv;
				case mode7: return R"(/___)"sv;
				default: throw std::logic_error(fmt::format("Invalid value for ssgeg_mode, got {}", std::to_underlying(mode)));
			}
		}

		[[nodiscard, gnu::const]] static constexpr auto string(const feedback_mode &mode) {
			using namespace std::string_view_literals;
			using enum feedback_mode;
			switch(mode) {
				case off: return "Off"sv;
				case pi_div_16: return "Pi / 16"sv;
				case pi_div_8: return "Pi / 8"sv;
				case pi_div_4: return "Pi / 4"sv;
				case pi_div_2: return "Pi / 2"sv;
				case pi: return "Pi"sv;
				case pi_mul_2: return "2 * Pi"sv;
				case pi_mul_4: return "4 * Pi"sv;
				default: throw std::logic_error(fmt::format("Invalid value for feedback_mode, got {}", std::to_underlying(mode)));
			}
		}

		[[nodiscard, gnu::const]] static constexpr auto string(const algorithm_mode &mode) {
			using namespace std::string_view_literals;
			using enum algorithm_mode;
			switch(mode) {
				case mode0: return "0 (Out 4)"sv;
				case mode1: return "1 (Out 4)"sv;
				case mode2: return "2 (Out 4)"sv;
				case mode3: return "3 (Out 4)"sv;
				case mode4: return "4 (Out 2-4)"sv;
				case mode5: return "5 (Out 2-3-4)"sv;
				case mode6: return "6 (Out 2-3-4)"sv;
				case mode7: return "7 (Out 1-2-3-4)"sv;
				default: throw std::logic_error(fmt::format("Invalid value for algorithm_mode, got {}", std::to_underlying(mode)));
			}
		}

		[[nodiscard, gnu::const]] static constexpr std::size_t operator_to_index(const op_id &op) {
			if(op > op_id::op4) {
				throw std::logic_error(fmt::format("Invalid index passed to operator_to_index. {} outside of range of 0-3", std::to_underlying(op)));
			}
			return std::to_underlying(op);
		}
	};
	static_assert(sizeof(operators) == operators::instrument_register_size);

	template<typename T>
	struct list_helper {
		using type = std::span<const T>;
		[[nodiscard, gnu::const]] static constexpr auto list() -> type = delete;
	};

	template<>
	struct list_helper<operators::op_id> {
		using type = std::span<const operators::op_id, 4>;

		[[nodiscard, gnu::const]] static constexpr type list() {
			static constexpr std::array values = {
				operators::op_id::op1,
				operators::op_id::op2,
				operators::op_id::op3,
				operators::op_id::op4,
			};
			return values;
		}
	};

	template<>
	struct list_helper<operators::detune_mode> {
		using type = std::span<const operators::detune_mode, 8>;

		[[nodiscard, gnu::const]] static constexpr type list() {
			static constexpr std::array values = {
				operators::detune_mode::no_change_1,
				operators::detune_mode::plus_e,
				operators::detune_mode::plus_2e,
				operators::detune_mode::plus_3e,
				operators::detune_mode::no_change_2,
				operators::detune_mode::minus_e,
				operators::detune_mode::minus_2e,
				operators::detune_mode::minus_3e
			};
			return values;
		}
	};

	template<>
	struct list_helper<operators::rate_scaling_mode> {
		using type = std::span<const operators::rate_scaling_mode, 4>;

		[[nodiscard, gnu::const]] static constexpr type list() {
			static constexpr std::array values = {
				operators::rate_scaling_mode::kc8,
				operators::rate_scaling_mode::kc4,
				operators::rate_scaling_mode::kc2,
				operators::rate_scaling_mode::kc1,
			};
			return values;
		}
	};

	template<>
	struct list_helper<operators::ssgeg_mode> {
		using type = std::span<const operators::ssgeg_mode, 9>;

		[[nodiscard, gnu::const]] static constexpr type list() {
			static constexpr std::array values = {
				operators::ssgeg_mode::disabled,
				operators::ssgeg_mode::mode0,
				operators::ssgeg_mode::mode1,
				operators::ssgeg_mode::mode2,
				operators::ssgeg_mode::mode3,
				operators::ssgeg_mode::mode4,
				operators::ssgeg_mode::mode5,
				operators::ssgeg_mode::mode6,
				operators::ssgeg_mode::mode7,
			};
			return values;
		}
	};

	template<>
	struct list_helper<operators::feedback_mode> {
		using type = std::span<const operators::feedback_mode, 8>;

		[[nodiscard, gnu::const]] static constexpr type list() {
			static constexpr std::array values = {
				operators::feedback_mode::off,
				operators::feedback_mode::pi_div_16,
				operators::feedback_mode::pi_div_8,
				operators::feedback_mode::pi_div_4,
				operators::feedback_mode::pi_div_2,
				operators::feedback_mode::pi,
				operators::feedback_mode::pi_mul_2,
				operators::feedback_mode::pi_mul_4,
			};
			return values;
		}
	};

	template<>
	struct list_helper<operators::algorithm_mode> {
		using type = std::span<const operators::algorithm_mode, 8>;

		[[nodiscard, gnu::const]] static constexpr type list() {
			static constexpr std::array values = {
				operators::algorithm_mode::mode0,
				operators::algorithm_mode::mode1,
				operators::algorithm_mode::mode2,
				operators::algorithm_mode::mode3,
				operators::algorithm_mode::mode4,
				operators::algorithm_mode::mode5,
				operators::algorithm_mode::mode6,
				operators::algorithm_mode::mode7
			};
			return values;
		}
	};

	template<typename T>
	static constexpr auto list() {
		return list_helper<T>::list();
	}
} // namespace MID3SMPS::fm
