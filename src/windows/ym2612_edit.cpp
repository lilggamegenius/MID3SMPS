#include "ym2612_edit.hpp"

#include <imguiwrap.dear.h>
#include <imgui.h>
#include <fmt/compile.h>

#include "containers/files/gyb.hpp"
#include "containers/files/fm/patch.hpp"

namespace MID3SMPS {
	void ym2612_edit::render_impl() {
		dear::Begin(window_title(), &open_, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse) && [this] {
			render_menu_bar();
			render_patch_selection();
			dear::TabBar("Editor tabs") && [this] {
				dear::TabItem("Digital") && [this] {
					render_editor_digital();
				};
				dear::TabItem("Analog") && [this] {
					render_editor_analog();
				};
			};
		};
	}

	void ym2612_edit::render_menu_bar() {
		fmt::print("");
		dear::MenuBar() && [this] {
			if(ImGui::MenuItem("Open new bank")) {}
			if(ImGui::MenuItem("Save bank")) {}
			if(ImGui::MenuItem("Bank switch")) {}
			if(ImGui::MenuItem("Import from file")) {}
			if(ImGui::MenuItem("About")) {}
		};
	}

	ImVec2 ym2612_edit::calculate_child_size() {
		const auto win_size    = ImGui::GetWindowSize();
		const auto &padding    = ImGui::GetStyle().WindowPadding;
		const auto dock_height = ImGui::GetFrameHeight();
		ImVec2 child_size(win_size);
		child_size.x -= padding.x * 2;
		child_size.y /= 2;
		child_size.y -= dock_height;
		child_size.y -= padding.y;
		return child_size;
	}

	void ym2612_edit::render_patch_selection() {
		const auto &padding = ImGui::GetStyle().WindowPadding;
		auto child_size     = calculate_child_size();
		child_size.y -= padding.y;
		dear::Child("Patch Info", child_size) && [this] {
			ImGui::TextUnformatted("Placeholder space");
		};
	}

	void ym2612_edit::render_editor_digital() {
		static constexpr auto table_flags = ImGuiTableFlags_Borders;
		auto child_size = calculate_child_size();
		//child_size.y = 0;
		child_size.x *= 5.f / 7.f;
		static fm::patch test{
			.name = "Test patch",
			.operators = {
				.registers = {
					0x00, 0x00, 0x00, 0x00,
					0x7F, 0x7F, 0x7F, 0x7F,
					0x1F, 0x1F, 0x1F, 0x1F,
					0x1F, 0x1F, 0x1F, 0x1F,
					0x1F, 0x1F, 0x1F, 0x1F,
					0x0F, 0x0F, 0x0F, 0x0F,
					0x00, 0x00, 0x00, 0x00,
					0x00, 0x00
				}
			}
		};
		dear::Table("Patch Editor Table 1", 5, table_flags, child_size) && [this] {
			static constexpr auto row_count = 12;
			ImGui::BeginDisabled(false);
			//const auto availible_space = ImGui::GetContentRegionAvail();
			//const auto cell_space = availible_space.y / row_count;
			for(std::uint_fast8_t row = 0; row < row_count; row++) {
				switch(row) {
					case 0: {
						ImGui::TableNextColumn();
						render_lfo();
						for(const auto &op_id : fm::list<fm::operators::op_id>()) {
							ImGui::TableNextColumn();
							ImGui::TextUnformatted(fm::operators::string(op_id));
						}
						break;
					}
					case 1: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Detune");
						for(const auto &op_id : fm::list<fm::operators::op_id>()) {
							ImGui::TableNextColumn();
							render_detune(test.operators, op_id);
						}
						break;
					}
					case 2: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Multiple");
						for(const auto &op_id : fm::list<fm::operators::op_id>()) {
							ImGui::TableNextColumn();
							render_multiple(test.operators, op_id);
						}
						break;
					}
					case 3: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Total Level");
						for(const auto &op_id : fm::list<fm::operators::op_id>()) {
							ImGui::TableNextColumn();
							render_total_level(test.operators, op_id);
						}
						break;
					}
					case 4: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Rate Scaling");
						for(const auto &op_id : fm::list<fm::operators::op_id>()) {
							ImGui::TableNextColumn();
							render_rate_scaling(test.operators, op_id);
						}
						break;
					}
					case 5: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Attack Rate");
						for(const auto &op_id : fm::list<fm::operators::op_id>()) {
							ImGui::TableNextColumn();
							render_attack_rate(test.operators, op_id);
						}
						break;
					}
					case 6: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Amplitude Modulation");
						for(const auto &op_id : fm::list<fm::operators::op_id>()) {
							ImGui::TableNextColumn();
							render_amplitude_modulation(test.operators, op_id);
						}
						break;
					}
					case 7: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Decay Rate");
						for(const auto &op_id : fm::list<fm::operators::op_id>()) {
							ImGui::TableNextColumn();
							render_decay_rate(test.operators, op_id);
						}
						break;
					}
					case 8: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Sustain Rate");
						for(const auto &op_id : fm::list<fm::operators::op_id>()) {
							ImGui::TableNextColumn();
							render_sustain_rate(test.operators, op_id);
						}
						break;
					}
					case 9: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Sustain Level");
						for(const auto &op_id : fm::list<fm::operators::op_id>()) {
							ImGui::TableNextColumn();
							render_sustain_level(test.operators, op_id);
						}
						break;
					}
					case 10: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Release Rate");
						for(const auto &op_id : fm::list<fm::operators::op_id>()) {
							ImGui::TableNextColumn();
							render_release_rate(test.operators, op_id);
						}
						break;
					}
					case 11: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("SSG-EG");
						for(const auto &op_id : fm::list<fm::operators::op_id>()) {
							ImGui::TableNextColumn();
							render_ssgeg(test.operators, op_id);
						}
						break;
					}
					default: break;
				}
			}
			ImGui::EndDisabled();
		};
		ImGui::SameLine();
		dear::Table("Patch Editor Table 2", 2, table_flags) && [this] {
			static constexpr auto row_count = 8;
			for(std::uint_fast8_t row = 0; row < row_count; row++) {
				switch(row) {
					case 0:
						ImGui::TableNextColumn();
						render_ams(test.operators);
						ImGui::TableNextColumn();
						render_fms(test.operators);
						break;
					case 1:
						ImGui::TableNextColumn();
						render_feedback(test.operators);
						break;
					case 2:
						ImGui::TableNextColumn();
						render_algorithm(test.operators);
						break;
					case 3:
						ImGui::TableNextColumn();
						render_transposition(test.operators);
						break;
					case 4:
					case 5:
					case 6:
					case 7: {
						ImGui::TableNextColumn();
						const std::uint_fast8_t current_row = row-4;
						const std::uint_fast8_t offset = current_row * 8;
						const std::uint_fast8_t max = row == 7 ? 6 : 8;
						const auto width = ImGui::CalcTextSize("FF0").x;
						for(std::uint_fast8_t i = 0; i < max; i++) {
							if(i == 4) {
								ImGui::TableNextColumn();
							} else if(i != 0) {
								ImGui::SameLine();
							}
							ImGui::PushItemWidth(width);
							ImGui::InputScalar("##", ImGuiDataType_U8, &test.operators.registers[offset + i], nullptr, nullptr, "%02X");
						}
						break;
					}
					default: break;
				}
			}
		};
	}

	void ym2612_edit::render_editor_analog() {
		ImGui::TextUnformatted("Not done yet, go away");
	}

	void ym2612_edit::render_lfo() {
		ImGui::TextUnformatted("LFO");
		ImGui::SameLine();
		static std::string preview = "Off";
		dear::Combo("##LFO Value", preview.c_str()) && [this] {
			static std::unordered_map<std::uint8_t, std::string> string_map;
			for (std::size_t i = 0; i < gyb::lfo_values.size(); i++){
				const bool is_selected = gyb_.default_LFO_speed == i;
				if(!string_map.contains(i)) {
					std::string new_val;
					if(i == 0) {
						new_val = "Off";
					} else {
						new_val = fmt::format(FMT_COMPILE("{}"), gyb::lfo_values[i]);
					}
					string_map.emplace(i, std::move(new_val));
				}
				if (ImGui::Selectable(string_map[i].c_str(), is_selected)) {
					gyb_.default_LFO_speed = static_cast<std::uint8_t>(i);
					preview = string_map[i];
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
		};
	}

	void ym2612_edit::render_detune(fm::operators &op, const fm::operators::op_id &op_id) {
		using oper = fm::operators;
		ImGui::PushID(&op_id);
		const auto val = op.detune(op_id);
		dear::Combo("##detune", oper::string(val)) && [this, &op, &op_id, &val] {
			for (const auto &current : fm::list<oper::detune_mode>()){
				const bool is_selected = val == current;
				if (ImGui::Selectable(oper::string(current), is_selected)) {
					op.detune(op_id, current);
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		};
		if(const auto new_val = handle_combo_scroll(val)) {
			op.detune(op_id, *new_val);
		}
		ImGui::PopID();
	}
	void ym2612_edit::render_multiple(fm::operators &op, const fm::operators::op_id &op_id) {
		static constexpr std::uint8_t step = 0x1, step_fast = 16/4;
		auto val = op.multiple(op_id);
		ImGui::PushID(&op_id);
		if(ImGui::InputScalar("##multiple", ImGuiDataType_U8, &val, &step, &step_fast)) {
			op.multiple(op_id, val);
		}
		if(const auto new_val = handle_combo_scroll(val, true)) {
			op.multiple(op_id, *new_val);
		}
		ImGui::PopID();
	}
	void ym2612_edit::render_total_level(fm::operators &op, const fm::operators::op_id &op_id) {
		static constexpr std::uint8_t step = 0x1, step_fast = 128/4;
		auto val = op.total_level(op_id);
		ImGui::PushID(&op_id);
		if(ImGui::InputScalar("##total_level", ImGuiDataType_U8, &val, &step, &step_fast)) {
			op.total_level(op_id, val);
		}
		if(const auto new_val = handle_combo_scroll(val, true)) {
			op.total_level(op_id, *new_val);
		}
		ImGui::PopID();
	}
	void ym2612_edit::render_rate_scaling(fm::operators &op, const fm::operators::op_id &op_id) {
		using oper = fm::operators;
		const auto val = op.rate_scaling(op_id);
		ImGui::PushID(&op_id);
		dear::Combo("##rate_scaling", oper::string(val)) && [this, &op, &op_id, &val] {
			for (const auto &current : fm::list<oper::rate_scaling_mode>()){
				const bool is_selected = val == current;
				if (ImGui::Selectable(oper::string(current), is_selected)) {
					op.rate_scaling(op_id, current);
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		};
		if(const auto new_val = handle_combo_scroll(val)) {
			op.rate_scaling(op_id, *new_val);
		}
		ImGui::PopID();
	}
	void ym2612_edit::render_attack_rate(fm::operators &op, const fm::operators::op_id &op_id) {
		static constexpr std::uint8_t step = 0x1, step_fast = 32/4;
		auto val = op.attack_rate(op_id);
		ImGui::PushID(&op_id);
		if(ImGui::InputScalar("##attack_rate", ImGuiDataType_U8, &val, &step, &step_fast)) {
			op.attack_rate(op_id, val);
		}
		if(const auto new_val = handle_combo_scroll(val, true)) {
			op.attack_rate(op_id, *new_val);
		}
		ImGui::PopID();
	}
	void ym2612_edit::render_amplitude_modulation(fm::operators &op, const fm::operators::op_id &op_id) {
		static constexpr std::array values = {
			"Disabled",
			"Enabled"
		};
		const auto val = op.amplitude_modulation(op_id);
		ImGui::PushID(&op_id);
		dear::Combo("##amplitude_modulation", val ? values[1] : values[0]) && [this, &op, &op_id, &val] {
			for (const auto &current : values){
				const bool current_bool = current == values[1];
				const bool is_selected = val == current_bool;
				if (ImGui::Selectable(current, is_selected)) {
					op.amplitude_modulation(op_id, current_bool);
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		};
		using enum scroll_wheel_direction;
		if(const auto new_val = handle_scroll(); new_val != neutral) {
			op.amplitude_modulation(op_id, new_val == positive );
		}
		ImGui::PopID();
	}
	void ym2612_edit::render_decay_rate(fm::operators &op, const fm::operators::op_id &op_id) {
		static constexpr std::uint8_t step = 0x1, step_fast = 32/4;
		auto val = op.decay_rate(op_id);
		ImGui::PushID(&op_id);
		if(ImGui::InputScalar("##decay_rate", ImGuiDataType_U8, &val, &step, &step_fast)) {
			op.decay_rate(op_id, val);
		}
		if(const auto new_val = handle_combo_scroll(val, true)) {
			op.decay_rate(op_id, *new_val);
		}
		ImGui::PopID();
	}
	void ym2612_edit::render_sustain_rate(fm::operators &op, const fm::operators::op_id &op_id) {
		static constexpr std::uint8_t step = 0x1, step_fast = 32/4;
		auto val = op.sustain_rate(op_id);
		ImGui::PushID(&op_id);
		if(ImGui::InputScalar("##sustain_rate", ImGuiDataType_U8, &val, &step, &step_fast)) {
			op.sustain_rate(op_id, val);
		}
		if(const auto new_val = handle_combo_scroll(val, true)) {
			op.sustain_rate(op_id, *new_val);
		}
		ImGui::PopID();
	}
	void ym2612_edit::render_sustain_level(fm::operators &op, const fm::operators::op_id &op_id) {
		static constexpr std::uint8_t step = 0x1, step_fast = 32/4;
		auto val = op.sustain_level(op_id);
		ImGui::PushID(&op_id);
		if(ImGui::InputScalar("##sustain_level", ImGuiDataType_U8, &val, &step, &step_fast)) {
			op.sustain_level(op_id, val);
		}
		if(const auto new_val = handle_combo_scroll(val, true)) {
			op.sustain_level(op_id, *new_val);
		}
		ImGui::PopID();
	}
	void ym2612_edit::render_release_rate(fm::operators &op, const fm::operators::op_id &op_id) {
		static constexpr std::uint8_t step = 0x1, step_fast = 32/4;
		auto val = op.release_rate(op_id);
		ImGui::PushID(&op_id);
		if(ImGui::InputScalar("##release_rate", ImGuiDataType_U8, &val, &step, &step_fast)) {
			op.release_rate(op_id, val);
		}
		if(const auto new_val = handle_combo_scroll(val, true)) {
			op.release_rate(op_id, *new_val);
		}
		ImGui::PopID();
	}
	void ym2612_edit::render_ssgeg(fm::operators &op, const fm::operators::op_id &op_id) {
		using oper = fm::operators;
		const auto val = op.ssgeg(op_id);
		ImGui::PushID(&op_id);
		dear::Combo("##ssgeg", oper::string(val)) && [this, &op, &op_id, &val] {
			for (const auto &current : fm::list<oper::ssgeg_mode>()){
				const bool is_selected = val == current;
				if (ImGui::Selectable(oper::string(current), is_selected)) {
					op.ssgeg(op_id, current);
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		};
		//if(const auto new_val = handle_combo_scroll(val)) {
		//	op.ssgeg(op_id, *new_val);
		//}
		ImGui::PopID();
	}

	void ym2612_edit::render_ams(fm::operators &op) {
		ImGui::TextUnformatted("AMS");
		ImGui::SameLine();
		using oper = fm::operators;
		static std::string preview = fmt::format(FMT_COMPILE("{}"), oper::ams_values[0]);
		static std::unordered_map<std::uint8_t, std::string> string_map;
		dear::Combo("##AMS value", preview.c_str()) && [this, &op] {
			for (std::size_t i = 0; i < oper::ams_values.size(); i++){
				const bool is_selected = op.ams() == i;
				if(!string_map.contains(i)) {
					string_map.emplace(i, fmt::format(FMT_COMPILE("{}"), oper::ams_values[i]));
				}
				if (ImGui::Selectable(string_map[i].c_str(), is_selected)) {
					op.ams(static_cast<std::uint8_t>(i));
					preview = string_map[i];
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
		};
		if(const auto new_val = handle_combo_scroll<oper::ams_values.size()>(op.ams())) {
			op.ams(*new_val);
			preview = string_map[*new_val];
		}
	}
	void ym2612_edit::render_fms(fm::operators &op) {
		ImGui::TextUnformatted("FMS %");
		ImGui::SameLine();
		using oper = fm::operators;
		static std::string preview = fmt::format(FMT_COMPILE("{}"), oper::fms_values[0]);
		static std::unordered_map<std::uint8_t, std::string> string_map;
		dear::Combo("##FMS value", preview.c_str()) && [this, &op] {
			for (std::size_t i = 0; i < oper::fms_values.size(); i++){
				const bool is_selected = op.fms() == i;
				if(!string_map.contains(i)) {
					string_map.emplace(i, fmt::format(FMT_COMPILE("{}"), oper::fms_values[i]));
				}
				if (ImGui::Selectable(string_map[i].c_str(), is_selected)) {
					op.fms(static_cast<std::uint8_t>(i));
					preview = string_map[i];
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
		};
		if(const auto new_val = handle_combo_scroll<oper::fms_values.size()>(op.fms())) {
			op.fms(*new_val);
			preview = string_map[*new_val];
		}
	}
	void ym2612_edit::render_feedback(fm::operators &op) {
		ImGui::TextUnformatted("Feedback");
		ImGui::TableNextColumn();
		const auto val = op.feedback();
		using oper = fm::operators;
		dear::Combo("##Feedback value", oper::string(val)) && [this, &op, &val] {
			for (const auto &current : fm::list<oper::feedback_mode>()){
				const bool is_selected = val == current;
				if (ImGui::Selectable(oper::string(current), is_selected)) {
					op.feedback(current);
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		};
		if(const auto new_val = handle_combo_scroll(val)) {
			op.feedback(*new_val);
		}
	}
	void ym2612_edit::render_algorithm(fm::operators &op) {
		ImGui::TextUnformatted("Algorithm");
		ImGui::TableNextColumn();
		const auto val = op.algorithm();
		using oper = fm::operators;
		dear::Combo("##Algorithm value", oper::string(val)) && [this, &op, &val] {
			for (const auto &current : fm::list<oper::algorithm_mode>()){
				const bool is_selected = val == current;
				if (ImGui::Selectable(oper::string(current), is_selected)) {
					op.algorithm(current);
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		};
		if(const auto new_val = handle_combo_scroll(val)) {
			op.algorithm(*new_val);
		}
	}

	void ym2612_edit::render_transposition(fm::operators &op) {
		ImGui::TextUnformatted("Transposition");
		ImGui::TableNextColumn();
		dear::Combo("##Transposition value", "0 (Default)") && [this, &op] {
			(void)op;
		};
	}

	ym2612_edit::scroll_wheel_direction ym2612_edit::handle_scroll() {
		using enum scroll_wheel_direction;
		ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
		if (!ImGui::IsItemHovered()) {
			return neutral;
		}
		const float wheel = ImGui::GetIO().MouseWheel;
		if (wheel == 0.f) {
			return neutral;
		}
		if (ImGui::IsItemActive()){
			ImGui::ClearActiveID();
		} else {
			if(wheel > 0.f) {
				return positive;
			}
			return negative;
		}
		return neutral;
	}

	template<std::size_t max_value, std::integral T, std::enable_if_t<!std::is_enum_v<T>, bool>>
	std::optional<T> ym2612_edit::handle_combo_scroll(const T &value, bool invert) {
		auto new_val = value;
		auto scroll = handle_scroll();
		using enum scroll_wheel_direction;
		if(invert) {
			switch(scroll) {
				case negative: scroll = positive;
					break;
				case positive: scroll = negative;
					break;
				default:
					break;
			}
		}
		switch(scroll) {
			case negative:
				if constexpr (max_value != 0) {
					if(new_val == max_value-1) {
						return std::nullopt;
					}
				}
			++new_val;
			break;
			case positive:
				if(new_val == 0) {
					return std::nullopt;
				}
			--new_val;
			break;
			case neutral:
				default: return std::nullopt;
		}
		return new_val;
	}

	template<typename T, std::enable_if_t<std::is_enum_v<T>, bool>>
	std::optional<T> ym2612_edit::handle_combo_scroll(const T &enumeration) {
		const auto underlying = std::to_underlying(enumeration);
		const auto new_enum = handle_combo_scroll<fm::list<T>().size(), std::underlying_type_t<T>>(underlying);
		if(new_enum) {
			return static_cast<T>(*new_enum);
		}
		return std::nullopt;
	}


	void ym2612_edit::render_children_impl() {}

	void ym2612_edit::on_close_impl() {}

	bool ym2612_edit::keep_impl() const {
		return open_;
	}
} // MID3SMPS
