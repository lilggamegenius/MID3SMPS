#pragma once

#include "containers/files/gyb.hpp"
#include "containers/files/fm/operators.hpp"
#include <string_view>

namespace MID3SMPS {
	using namespace std::string_view_literals;
	class ym2612_edit : public window<ym2612_edit>{
		static constexpr std::array num_formats = {
			"%i"sv,
			"%X"sv
		};
		friend class main_window;
		enum class editor_mode : std::uint8_t {
			digital,
			analog
		} mode_{};
		bool open_ = false;
		bool hex_format = false;
		[[nodiscard]] constexpr auto num_format() const {
			if(hex_format) {
				return num_formats[1];
			}
			return num_formats[0];
		}

		gyb gyb_{};

		void render_menu_bar();
		void render_patch_selection();
		void render_editor_digital();
		void render_editor_analog();

		void render_lfo();

		void render_detune(fm::operators &op, const fm::operators::op_id &op_id);
		void render_multiple(fm::operators &op, const fm::operators::op_id &op_id);
		void render_total_level(fm::operators &op, const fm::operators::op_id &op_id);
		void render_rate_scaling(fm::operators &op, const fm::operators::op_id &op_id);
		void render_attack_rate(fm::operators &op, const fm::operators::op_id &op_id);
		void render_amplitude_modulation(fm::operators &op, const fm::operators::op_id &op_id);
		void render_decay_rate(fm::operators &op, const fm::operators::op_id &op_id);
		void render_sustain_rate(fm::operators &op, const fm::operators::op_id &op_id);
		void render_sustain_level(fm::operators &op, const fm::operators::op_id &op_id);
		void render_release_rate(fm::operators &op, const fm::operators::op_id &op_id);
		void render_ssgeg(fm::operators &op, const fm::operators::op_id &op_id);

		void render_ams(fm::operators &op);
		void render_fms(fm::operators &op);
		void render_feedback(fm::operators &op);
		void render_algorithm(fm::operators &op);
		void render_transposition(fm::operators &op);
		void render_registers(/*const*/ fm::operators &op, std::uint_fast8_t current_row) const;

		enum class scroll_wheel_direction {
			positive,
			neutral,
			negative
		};

		[[nodiscard]] static scroll_wheel_direction handle_scroll();
		template<std::size_t max_value = 0, std::integral T, std::enable_if_t<!std::is_enum_v<T>, bool> = true>
		[[nodiscard]] static std::optional<T> handle_combo_scroll(const T &value, bool invert = false);
		template<typename T, std::enable_if_t<std::is_enum_v<T>, bool> = true>
		[[nodiscard]] static std::optional<T> handle_combo_scroll(const T &enumeration);
		[[nodiscard]] static ImVec2 calculate_child_size();

		friend class window;
		void render_impl();
		void render_children_impl();
		void on_close_impl();
		[[nodiscard]] bool keep_impl() const;
		[[nodiscard]] static constexpr const char* window_title_impl(){
			return "YM2612 Edit";
		}
	};
} // MID3SMPS