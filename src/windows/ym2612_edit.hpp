#pragma once

#include <string_view>

#include "windows/window.hpp"
#include "containers/files/gyb.hpp"
#include "containers/files/fm/operators.hpp"

namespace MID3SMPS {
	using namespace std::string_view_literals;
	class ym2612_edit : public window<ym2612_edit>{
		static constexpr std::array num_formats = {
			"%i"sv,
			"%X"sv
		};
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
		float last_space_remaining = 0;

		gyb gyb_{};
		bool diry_ = false;

		void render_menu_bar();
		void render_patch_selection();
		void render_editor_digital();
		void render_editor_analog();

		std::optional<std::pair<gyb::bank, ins_count_t>> selected_id = std::nullopt;
		[[nodiscard]] constexpr bool has_selected_patch() const {
			return selected_id.has_value();
		}
		[[nodiscard]] constexpr std::optional<gyb::bank> selected_bank_id() const {
			if(!selected_id) {
				return std::nullopt;
			}
			return selected_id->first;
		}
		[[nodiscard]] constexpr std::optional<ins_count_t> selected_patch_id() const {
			if(!selected_id) {
				return std::nullopt;
			}
			return selected_id->second;
		}
		[[nodiscard]] gyb::patch_order_t& selected_bank();
		[[nodiscard]] const gyb::patch_order_t& selected_bank() const;
		[[nodiscard]] fm::patch& selected_patch();
		[[nodiscard]] const fm::patch& selected_patch() const;

		void render_patch_selector();
		void render_patch_mappings();
		void render_oscilloscope();

		void render_lfo();
		void render_operator_headers();

		void render_detune					(const fm::operators::op_id &op_id);
		void render_multiple				(const fm::operators::op_id &op_id);
		void render_total_level				(const fm::operators::op_id &op_id);
		void render_rate_scaling			(const fm::operators::op_id &op_id);
		void render_attack_rate				(const fm::operators::op_id &op_id);
		void render_amplitude_modulation	(const fm::operators::op_id &op_id);
		void render_decay_rate				(const fm::operators::op_id &op_id);
		void render_sustain_rate			(const fm::operators::op_id &op_id);
		void render_sustain_level			(const fm::operators::op_id &op_id);
		void render_release_rate			(const fm::operators::op_id &op_id);
		void render_ssgeg					(const fm::operators::op_id &op_id);

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
		[[nodiscard]] static std::optional<T> handle_combo_scroll(const T &value, bool invert = false);
		template<typename T, std::enable_if_t<std::is_enum_v<T>, bool> = true>
		[[nodiscard]] static std::optional<T> handle_combo_scroll(const T &enumeration);

		friend class main_window;
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