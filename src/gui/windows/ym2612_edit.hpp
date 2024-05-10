#pragma once

#include "gui/windows/window.hpp"
#include "containers/files/mid2smps/gyb.hpp"
#include "containers/chips/ym2612/operators.hpp"

namespace MID3SMPS {
	using namespace std::string_view_literals;
	class ym2612_edit : public window{
		static constexpr std::array num_formats = {
			"%i"sv,
			"%X"sv
		};
		enum class editor_mode : std::uint8_t {
			digital,
			analog
		} mode_{};
		bool hex_format = false;
		[[nodiscard]] constexpr auto num_format() const {
			if(hex_format) {
				return num_formats[1];
			}
			return num_formats[0];
		}
		float last_space_remaining = 0;

		M2S::gyb gyb_{};
		bool dirty_ = false;

		void render_menu_bar();
		void render_instrument_selection();
		void render_editor_digital();
		void render_editor_analog();

		std::optional<std::pair<bank_key_t, ins_key_t>> selected_id = std::nullopt;
		[[nodiscard]] constexpr bool has_selected_instrument() const {
			return selected_id.has_value();
		}
		[[nodiscard]] constexpr std::optional<bank_key_t> selected_bank_id() const {
			if(!selected_id) {
				return std::nullopt;
			}
			return selected_id->first;
		}
		[[nodiscard]] constexpr std::optional<ins_key_t> selected_instrument_id() const {
			if(!selected_id) {
				return std::nullopt;
			}
			return selected_id->second;
		}
		[[nodiscard]] instrument_bank::ins_order_t& selected_bank();
		[[nodiscard]] const instrument_bank::ins_order_t& selected_bank() const;
		[[nodiscard]] fm_instrument& selected_instrument();
		[[nodiscard]] const fm_instrument& selected_instrument() const;

		void render_instrument_selector();
		void render_instrument_mappings();
		void render_oscilloscope();

		void render_lfo();
		void render_operator_headers();

		void render_detune					(const ym2612::operators::op_id &op_id);
		void render_multiple				(const ym2612::operators::op_id &op_id);
		void render_total_level				(const ym2612::operators::op_id &op_id);
		void render_rate_scaling			(const ym2612::operators::op_id &op_id);
		void render_attack_rate				(const ym2612::operators::op_id &op_id);
		void render_amplitude_modulation	(const ym2612::operators::op_id &op_id);
		void render_decay_rate				(const ym2612::operators::op_id &op_id);
		void render_sustain_rate			(const ym2612::operators::op_id &op_id);
		void render_sustain_level			(const ym2612::operators::op_id &op_id);
		void render_release_rate			(const ym2612::operators::op_id &op_id);
		void render_ssgeg					(const ym2612::operators::op_id &op_id);

		void render_ams();
		void render_fms();
		void render_feedback();
		void render_algorithm();
		void render_transposition();
		void render_registers(std::uint_fast8_t current_row) const;

		void scale_window();

		enum class scroll_wheel_direction {
			positive,
			neutral,
			negative
		};

		[[nodiscard]] static scroll_wheel_direction handle_scroll();
		template<std::size_t max_value = 0, std::integral T, std::enable_if_t<!std::is_enum_v<T>, bool> = true>
		[[nodiscard]] static std::optional<T> handle_combo_scroll(const safe_int<T> &value, bool invert = false);
		template<typename T, std::enable_if_t<std::is_enum_v<T>, bool> = true>
		[[nodiscard]] static std::optional<T> handle_combo_scroll(const T &enumeration);

		friend class main_window;
	public:
		void render() override;
		void on_close() override;
		[[nodiscard]] constexpr const char* window_title() const override{
			return "YM2612 Edit";
		}
	};
} // MID3SMPS