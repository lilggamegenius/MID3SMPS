#include "ym2612_edit.hpp"

#include <imguiwrap.dear.h>
#include <imgui.h>
#include <execution>
#include <imgui_internal.h>
#include <gui/backend/window_handler.hpp>

#include "containers/files/mid2smps/gyb.hpp"
#include "containers/files/mid2smps/fm/patch.hpp"

static constexpr auto default_hover_flags = ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_ForTooltip;

namespace MID3SMPS {
	using namespace ym2612;
	using namespace M2S;
	template<std::size_t max_value, std::integral T, std::enable_if_t<!std::is_enum_v<T>, bool>>
	std::optional<T> ym2612_edit::handle_combo_scroll(const safe_int<T> &value, bool invert) {
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
		using underlying_t = std::underlying_type_t<T>;
		const auto new_enum = handle_combo_scroll<list<T>().size(), underlying_t>(underlying);
		if(new_enum) {
			return static_cast<T>(*new_enum);
		}
		return std::nullopt;
	}

	template<>
	std::optional<operators::ssgeg_mode> ym2612_edit::handle_combo_scroll(const ym2612::operators::ssgeg_mode &enumeration) {
		using T = operators::ssgeg_mode;
		static constexpr auto size = std::to_underlying(T::mode7)+1;
		const auto underlying = std::to_underlying(enumeration);
		using underlying_t = decltype(underlying);
		if(const auto new_enum = handle_combo_scroll<size, underlying_t>(underlying)) {
			auto value = *new_enum;
			if(value == std::to_underlying(T::disabled)+1) {
				return T::mode0;
			}
			if(value == std::to_underlying(T::mode0)-1) {
				return T::disabled;
			}
			return static_cast<T>(value);
		}
		return std::nullopt;
	}

	template<>
	std::optional<lfo> ym2612_edit::handle_combo_scroll(const lfo &enumeration) {
		using T = lfo;
		static constexpr auto size = std::to_underlying(T::mode7)+1;
		const auto underlying = std::to_underlying(enumeration);
		using underlying_t = decltype(underlying);
		const auto new_enum = handle_combo_scroll<size, underlying_t>(underlying);
		if(new_enum) {
			auto value = *new_enum;
			if(value == std::to_underlying(T::off)+1) {
				return T::mode0;
			}
			if(value == std::to_underlying(T::mode0)-1) {
				return T::off;
			}
			return static_cast<T>(value);
		}
		return std::nullopt;
	}

	void ym2612_edit::render() {
		dear::Begin{window_title(), &stay_open_, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse} && [this] {
			//const auto dock_node = ImGui::GetWindowDockNode();
			render_menu_bar();
			render_patch_selection();
			dear::TabBar{"Editor tabs"} && [this] {
				dear::TabItem{"Digital"} && [this] {
					dear::Disabled(!has_selected_patch()) && [this] {
						render_editor_digital();
					};
				};
				dear::TabItem{"Analog"} && [this] {
					dear::Disabled(!has_selected_patch()) && [this] {
						render_editor_analog();
					};
				};
			};
			scale_window();
		};
	}

	void ym2612_edit::render_menu_bar() {
		dear::WithStyleVar(ImGuiStyleVar_ItemSpacing, {8, 0}) && [] {
			dear::MenuBar{} && [] {
				if(ImGui::MenuItem("Open new bank")) {}
				if(ImGui::MenuItem("Save bank")) {}
				if(ImGui::MenuItem("Bank switch")) {}
				if(ImGui::MenuItem("Import from file")) {}
				if(ImGui::MenuItem("About")) {}
			};
		};
	}

	void ym2612_edit::render_patch_selection() {
		dear::WithStyleVar style(ImGuiStyleVar_WindowPadding, {0, 0});
		auto child_size = ImGui::GetContentRegionAvail();
		child_size.y *= .45f;
		dear::Child{"Patch Info", child_size, ImGuiChildFlags_ResizeY} && [this, &child_size] {
			dear::Child("Patch selector", {child_size.x / 4, -1}, ImGuiChildFlags_Border) && [this] {
				render_patch_selector();
			};
			ImGui::SameLine();
			dear::Child("Mapping selector", {child_size.x / 4, -1}, ImGuiChildFlags_Border) && [this] {
				render_patch_mappings();
			};
			ImGui::SameLine();
			dear::Child("Oscilloscope", {child_size.x / 2, -1}, ImGuiChildFlags_Border) && [this] {
				render_oscilloscope();
			};
		};
	}

	void ym2612_edit::render_patch_selector() {
		static constexpr ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		for(const auto &bank : gyb_.bank_order) {
			auto category_flags = base_flags;
			if(selected_bank_id() == bank) {
				category_flags |= ImGuiTreeNodeFlags_Selected;
			}
			dear::TreeNodeEx(gyb_.banks[bank].c_str(), category_flags) && [this, &bank] {
				ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
				for(const auto &id : gyb_.instruments_order[bank]) {
					const auto &patch = gyb_.instruments[id];
					auto patch_flags = base_flags;
					if(selected_patch_id() == id) {
						patch_flags |= ImGuiTreeNodeFlags_Selected;
					}
					patch_flags |= ImGuiTreeNodeFlags_Leaf;
					dear::TreeNodeEx(patch->name.c_str(), patch_flags) && [this, &bank, &id, &patch] {
						if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
							selected_id = {bank, id};
						}
						dear::ItemTooltip() && [&patch] {
							ImGui::TextUnformatted(patch->name.c_str());
						};
					};
				}
				ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
			};
		}
	}
	void ym2612_edit::render_patch_mappings() {}

	template<typename T, typename R = float>
	constexpr R normalize(const T &val, const R &max) requires std::floating_point<R> && (std::integral<T> || std::floating_point<R>){
		return static_cast<R>(val) / max;
	}

	template<typename T>
	constexpr void grow_and_insert(std::vector<T> &vector, const std::size_t &idx, const T &value) { // todo: forward value type
		while(vector.size() <= idx) {
			vector.emplace_back();
		}
		vector[idx] = value;
	}

	void ym2612_edit::render_oscilloscope() {
		static std::vector<float> adsr_data = {};
		if(!has_selected_patch()) {
			std::fill(std::execution::par_unseq, adsr_data.begin(), adsr_data.end(), 0);
		} else {
			const auto &operators = selected_patch().operators;
			constexpr auto id     = operators::op_id::op4;
			if(operators.attack_rate(id) == 0) {
				goto skip_render;
			}
			float level = 0;
			{
				const float tl = normalize(127 - operators.total_level(id), 127.f);
				float step = normalize(operators.attack_rate(id), 31.f);
				step *= tl;
				for(std::size_t idx = 0; level < tl; idx++) {
					level = std::min(level + step, tl);
					grow_and_insert(adsr_data, idx, level);
				}
			}
		}

	skip_render:
		ImGui::PlotLines("##Envelope", adsr_data.data(), static_cast<int>(adsr_data.size()), 0, nullptr, 0.f, 1.f, {-1, -1});
	}

	void ym2612_edit::render_editor_digital() {
		static constexpr auto table_flags = ImGuiTableFlags_Borders;
		auto child_size = ImGui::GetContentRegionAvail();
		child_size.y = 0;
		child_size.x *= 5.f / 7.f;
		dear::Table{"Patch Editor Table 1", 5, table_flags, child_size} && [this] {
			static constexpr auto row_count = 12;
			//const auto availible_space = ImGui::GetContentRegionAvail();
			//const auto cell_space = availible_space.y / row_count;
			const auto column_0_width = ImGui::CalcTextSize("Amplitude Modulation");
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, column_0_width.x);
			for(std::uint_fast8_t row = 0; row < row_count; row++) {
				switch(row) {
					case 0: {
						ImGui::TableNextColumn();
						render_lfo();
						render_operator_headers();
						break;
					}
					case 1: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Detune");
						for(const auto &op_id : list<operators::op_id>()) {
							ImGui::TableNextColumn();
							render_detune(op_id);
						}
						break;
					}
					case 2: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Multiple");
						for(const auto &op_id : list<operators::op_id>()) {
							ImGui::TableNextColumn();
							render_multiple(op_id);
						}
						break;
					}
					case 3: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Total Level");
						for(const auto &op_id : list<operators::op_id>()) {
							ImGui::TableNextColumn();
							render_total_level(op_id);
						}
						break;
					}
					case 4: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Rate Scaling");
						for(const auto &op_id : list<operators::op_id>()) {
							ImGui::TableNextColumn();
							render_rate_scaling(op_id);
						}
						break;
					}
					case 5: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Attack Rate");
						for(const auto &op_id : list<operators::op_id>()) {
							ImGui::TableNextColumn();
							render_attack_rate(op_id);
						}
						break;
					}
					case 6: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Amplitude Modulation");
						for(const auto &op_id : list<operators::op_id>()) {
							ImGui::TableNextColumn();
							render_amplitude_modulation(op_id);
						}
						break;
					}
					case 7: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Decay Rate");
						for(const auto &op_id : list<operators::op_id>()) {
							ImGui::TableNextColumn();
							render_decay_rate(op_id);
						}
						break;
					}
					case 8: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Sustain Rate");
						for(const auto &op_id : list<operators::op_id>()) {
							ImGui::TableNextColumn();
							render_sustain_rate(op_id);
						}
						break;
					}
					case 9: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Sustain Level");
						for(const auto &op_id : list<operators::op_id>()) {
							ImGui::TableNextColumn();
							render_sustain_level(op_id);
						}
						break;
					}
					case 10: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("Release Rate");
						for(const auto &op_id : list<operators::op_id>()) {
							ImGui::TableNextColumn();
							render_release_rate(op_id);
						}
						break;
					}
					case 11: {
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("SSG-EG");
						for(const auto &op_id : list<operators::op_id>()) {
							ImGui::TableNextColumn();
							render_ssgeg(op_id);
						}
						break;
					}
					default: break;
				}
			}
		};
		ImGui::SameLine();
		dear::Table{"Patch Editor Table 2", 2, table_flags} && [this] {
			static constexpr auto row_count = 12;
			for(std::uint_fast8_t row = 0; row < row_count; row++) {
				switch(row) {
					case 0:
						ImGui::TableNextColumn();
						render_ams();
						ImGui::TableNextColumn();
						render_fms();
						break;
					case 1:
						ImGui::TableNextColumn();
						render_feedback();
						break;
					case 2:
						ImGui::TableNextColumn();
						render_algorithm();
						break;
					case 3:
						ImGui::TableNextColumn();
						render_transposition();
						break;
					case 4:
					case 6:
					case 8:
					case 10: {
						static constexpr std::array headers = {
							"\t3X"sv,
							"\t4X"sv,
							"\t5X"sv,
							"\t6X"sv,
							"\t7X"sv,
							"\t8X"sv,
							"\t9X"sv,
							"\tBX"sv,
						};
						ImGui::TableNextColumn();
						const std::uint_fast8_t current_row = (row/2)-2;
						const std::uint_fast8_t offset = current_row*2;
						ImGui::TextUnformatted(headers[offset].data());
						ImGui::TableNextColumn();
						ImGui::TextUnformatted(headers[offset+1].data());
						break;
					}
					case 5:
					case 7:
					case 9:
					case 11: {
						ImGui::TableNextColumn();
						const std::uint_fast8_t current_row = (row/2)-2;
						render_registers(current_row);
						break;
					}
					default: break;
				}
			}
		};
	}

	void ym2612_edit::scale_window() {
		const auto cursor_y = ImGui::GetCursorPosY();
		const auto window_height = ImGui::GetWindowHeight();
		last_space_remaining = window_height-cursor_y;
		auto *current_window = ImGui::GetCurrentWindow();
		auto &window_scale = current_window->FontWindowScale;
		static constexpr float free_space_range = 17.f;
		static constexpr float scale_diff = 0.04f;
		// ReSharper disable once CppEntityAssignedButNoRead
		static std::optional<FpsIdling::override> override = std::nullopt;
		if(last_space_remaining > free_space_range) {
			window_scale += scale_diff;
		} else if(last_space_remaining < 0) {
			window_scale -= scale_diff;
		} else {
			override = std::nullopt;
			return;
		}
		override = handler.idling().get_override(); // Set the override temporarily so the screen can scale
		//ImGui::DebugLog("Cursor Y: %f WindowHeight: %f Available space: %f\n", cursor_y, window_height, last_space_remaining);
	}


	void ym2612_edit::render_editor_analog() {
		ImGui::TextUnformatted("Not done yet, go away");
	}

	fm::patch& ym2612_edit::selected_patch() {
		if(selected_id) {
			return *dynamic_cast<fm::patch*>(gyb_.instruments[selected_id->second].get());
		}
		return fm::empty_patch; // Patch editing is disabled if there is no patch selected
	}
	const fm::patch& ym2612_edit::selected_patch() const{
		if(selected_id) {
			return *dynamic_cast<fm::patch*>(gyb_.instruments.at(selected_id->second).get());
		}
		return fm::empty_patch;
	}

	void ym2612_edit::render_lfo() {
		ImGui::TextUnformatted("LFO  ");
		bool hovered = false;
		if (ImGui::IsItemHovered(default_hover_flags)){
			hovered = true;
		}
		ImGui::SameLine();
		auto &val = gyb_.default_LFO_speed;
		ImGui::SetNextItemWidth(-1);
		dear::Combo{"##LFO Value", gyb::string(val).data()} && [this, &val] {
			for (const auto &current : list<lfo>()){
				const bool is_selected = gyb_.default_LFO_speed == current;
				if (ImGui::Selectable(gyb::string(current).data(), is_selected)) {
					val = current;
				}

				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		};
		if (ImGui::IsItemHovered(default_hover_flags)){
			hovered = true;
		}
		dear::Tooltip{hovered} && [] {
			ImGui::TextUnformatted("Used to enable FMS and AMS to work for some simple vibrato and tremolo-like effects.");
		};
		if(const auto new_val = handle_combo_scroll(val)) {
			val = *new_val;
		}
	}

	void ym2612_edit::render_operator_headers() {
		for(const auto &op_id : list<operators::op_id>()) {
			ImGui::TableNextColumn();
			const auto str = operators::string(op_id);
			auto iter = str.data();
			if(ImGui::GetWindowWidth() < 600.f) { // Todo: Make this relative to available space
				std::advance(iter, 9); // Only show the operator number if window is too small
			}
			const auto cend = str.data() + str.size(); // Windows STL in debug mode doesn't use a ptr for iterators; no way to get the ptr without dereferencing it
			ImGui::TextUnformatted(iter, cend);
			dear::ItemTooltip{} && [this, &str] {
				ImGui::Text("Imagine an envelope preview here for %s", str.data());
				[[maybe_unused]] const auto &patch = selected_patch();
			};
		}
	}

	void ym2612_edit::render_detune(const operators::op_id &op_id) {
		using oper = operators;
		dear::WithID(&op_id) && [this, &op_id] {
			ImGui::SetNextItemWidth(-1);
			auto &op = selected_patch().operators;
			const auto val = op.detune(op_id);
			dear::Combo{"##detune", oper::string(val).data()} && [&op, &op_id, &val] {
				for (const auto &current : list<oper::detune_mode>()){
					const bool is_selected = val == current;
					if (ImGui::Selectable(oper::string(current).data(), is_selected)) {
						op.detune(op_id, current);
					}

					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
			};
			if(const auto new_val = handle_combo_scroll(val)) {
				op.detune(op_id, *new_val);
			}
		};
	}
	void ym2612_edit::render_multiple(const operators::op_id &op_id) {
		static constexpr std::uint8_t step = 0x1, step_fast = 16/4;
		dear::WithID(&op_id) && [this, &op_id] {
			auto &op = selected_patch().operators;
			auto val = op.multiple(op_id);
			ImGui::SetNextItemWidth(-1);
			if(ImGui::InputScalar("##multiple", ImGuiDataType_U8, &val, &step, &step_fast, num_format().data())) {
				op.multiple(op_id, val);
			}
			if(const auto new_val = handle_combo_scroll(val, true)) {
				op.multiple(op_id, *new_val);
			}
		};
	}
	void ym2612_edit::render_total_level(const operators::op_id &op_id) {
		static constexpr std::uint8_t step = 0x1, step_fast = 128/4;
		dear::WithID(&op_id) && [this, &op_id] {
			auto &op = selected_patch().operators;
			auto val = op.total_level(op_id);
			ImGui::SetNextItemWidth(-1);
			if(ImGui::InputScalar("##total_level", ImGuiDataType_U8, &val, &step, &step_fast, num_format().data())) {
				op.total_level(op_id, val);
			}
			if(const auto new_val = handle_combo_scroll(val, true)) {
				op.total_level(op_id, *new_val);
			}
		};
	}
	void ym2612_edit::render_rate_scaling(const operators::op_id &op_id) {
		using oper = operators;
		dear::WithID(&op_id) && [this, &op_id] {
			auto &op = selected_patch().operators;
			const auto val = op.rate_scaling(op_id);
			ImGui::SetNextItemWidth(-1);
			dear::Combo{"##rate_scaling",  oper::string(val).data()} && [&op, &op_id, &val] {
				for (const auto &current : list<oper::rate_scaling_mode>()){
					const bool is_selected = val == current;
					if (ImGui::Selectable(oper::string(current).data(), is_selected)) {
						op.rate_scaling(op_id, current);
					}

					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
			};
			if(const auto new_val = handle_combo_scroll(val)) {
				op.rate_scaling(op_id, *new_val);
			}
		};
	}
	void ym2612_edit::render_attack_rate(const operators::op_id &op_id) {
		static constexpr std::uint8_t step = 0x1, step_fast = 32/4;
		dear::WithID(&op_id) && [this, &op_id] {
			auto &op = selected_patch().operators;
			auto val = op.attack_rate(op_id);
			ImGui::SetNextItemWidth(-1);
			if(ImGui::InputScalar("##attack_rate", ImGuiDataType_U8, &val, &step, &step_fast, num_format().data())) {
				op.attack_rate(op_id, val);
			}
			if(const auto new_val = handle_combo_scroll(val, true)) {
				op.attack_rate(op_id, *new_val);
			}
		};
	}
	void ym2612_edit::render_amplitude_modulation(const operators::op_id &op_id) {
		static constexpr std::array values = {
			"Disabled",
			"Enabled"
		};
		dear::WithID(&op_id) && [this, &op_id] {
			auto &op = selected_patch().operators;
			const auto val = op.amplitude_modulation(op_id);
			ImGui::SetNextItemWidth(-1);
			dear::Combo{"##amplitude_modulation", val ? values[1] : values[0]} && [&op, &op_id, &val] {
				for (const auto &current : values){
					const bool current_bool = current == values[1];
					const bool is_selected = val == current_bool;
					if (ImGui::Selectable(current, is_selected)) {
						op.amplitude_modulation(op_id, current_bool);
					}

					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
			};
			using enum scroll_wheel_direction;
			if(const auto new_val = handle_scroll(); new_val != neutral) {
				op.amplitude_modulation(op_id, !val);
			}
		};
	}
	void ym2612_edit::render_decay_rate(const operators::op_id &op_id) {
		static constexpr std::uint8_t step = 0x1, step_fast = 32/4;
		dear::WithID(&op_id) && [this, &op_id] {
			auto &op = selected_patch().operators;
			auto val = op.decay_rate(op_id);
			ImGui::SetNextItemWidth(-1);
			if(ImGui::InputScalar("##decay_rate", ImGuiDataType_U8, &val, &step, &step_fast, num_format().data())) {
				op.decay_rate(op_id, val);
			}
			if(const auto new_val = handle_combo_scroll(val, true)) {
				op.decay_rate(op_id, *new_val);
			}
		};
	}
	void ym2612_edit::render_sustain_rate(const operators::op_id &op_id) {
		static constexpr std::uint8_t step = 0x1, step_fast = 32/4;
		dear::WithID(&op_id) && [this, &op_id] {
			auto &op = selected_patch().operators;
			auto val = op.sustain_rate(op_id);
			ImGui::SetNextItemWidth(-1);
			if(ImGui::InputScalar("##sustain_rate", ImGuiDataType_U8, &val, &step, &step_fast, num_format().data())) {
				op.sustain_rate(op_id, val);
			}
			if(const auto new_val = handle_combo_scroll(val, true)) {
				op.sustain_rate(op_id, *new_val);
			}
		};
	}
	void ym2612_edit::render_sustain_level(const operators::op_id &op_id) {
		static constexpr std::uint8_t step = 0x1, step_fast = 32/4;
		dear::WithID(&op_id) && [this, &op_id] {
			auto &op = selected_patch().operators;
			auto val = op.sustain_level(op_id);
			ImGui::SetNextItemWidth(-1);
			if(ImGui::InputScalar("##sustain_level", ImGuiDataType_U8, &val, &step, &step_fast, num_format().data())) {
				op.sustain_level(op_id, val);
			}
			if(const auto new_val = handle_combo_scroll(val, true)) {
				op.sustain_level(op_id, *new_val);
			}
		};
	}
	void ym2612_edit::render_release_rate(const operators::op_id &op_id) {
		static constexpr std::uint8_t step = 0x1, step_fast = 32/4;
		dear::WithID(&op_id) && [this, &op_id] {
			auto &op = selected_patch().operators;
			auto val = op.release_rate(op_id);
			ImGui::SetNextItemWidth(-1);
			if(ImGui::InputScalar("##release_rate", ImGuiDataType_U8, &val, &step, &step_fast, num_format().data())) {
				op.release_rate(op_id, val);
			}
			if(const auto new_val = handle_combo_scroll(val, true)) {
				op.release_rate(op_id, *new_val);
			}
		};
	}
	void ym2612_edit::render_ssgeg(const operators::op_id &op_id) {
		using oper = operators;
		dear::WithID(&op_id) && [this, &op_id] {
			auto &op = selected_patch().operators;
			const auto val = op.ssgeg(op_id);
			ImGui::SetNextItemWidth(-1);
			dear::Combo{"##ssgeg", oper::string(val).data()} && [&op, &op_id, &val] {
				for (const auto &current : list<oper::ssgeg_mode>()){
					const bool is_selected = val == current;
					if (ImGui::Selectable(oper::string(current).data(), is_selected)) {
						op.ssgeg(op_id, current);
					}

					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
			};
			if(const auto new_val = handle_combo_scroll(val)) {
				op.ssgeg(op_id, *new_val);
			}
		};
	}

	void ym2612_edit::render_ams() {
		ImGui::TextUnformatted("AMS  ");
		bool hovered = false;
		if (ImGui::IsItemHovered(default_hover_flags)){
			hovered = true;
		}
		ImGui::SameLine();
		using oper = operators;
		static constexpr std::array strings = {
			"0"sv,
			"1.4"sv,
			"5.9"sv,
			"11.8"sv
		};
		auto &op = selected_patch().operators;
		const auto val = op.ams();
		ImGui::SetNextItemWidth(-1);
		dear::Combo{"##AMS value", strings[val].data()} && [&op] {
			for (std::size_t i = 0; i < oper::ams_values.size(); i++){
				const bool is_selected = op.ams() == i;
				if (ImGui::Selectable(strings[i].data(), is_selected)) {
					op.ams(static_cast<std::uint8_t>(i));
				}

				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		};
		if (ImGui::IsItemHovered(default_hover_flags)){
			hovered = true;
		}
		dear::Tooltip{hovered} && [] {
			ImGui::TextUnformatted("Amplitude modulation sensivity");
		};
		if(const auto new_val = handle_combo_scroll<oper::ams_values.size()>(op.ams())) {
			op.ams(*new_val);
		}
	}
	void ym2612_edit::render_fms() {
		ImGui::TextUnformatted("FMS  %");
		bool hovered = false;
		if (ImGui::IsItemHovered(default_hover_flags)){
			hovered = true;
		}
		ImGui::SameLine();
		using oper = operators;
		static constexpr std::array strings = {
			"0"sv,
			"\u00b13.4"sv,
			"\u00b16.7"sv,
			"\u00b110"sv,
			"\u00b114"sv,
			"\u00b120"sv,
			"\u00b140"sv,
			"\u00b180"sv,
		};
		auto &op = selected_patch().operators;
		const auto val = op.fms();
		ImGui::SetNextItemWidth(-1);
		dear::Combo{"##FMS value", strings[val].data()} && [&op] {
			for (std::size_t i = 0; i < oper::fms_values.size(); i++){
				const bool is_selected = op.fms() == i;
				if (ImGui::Selectable(strings[i].data(), is_selected)) {
					op.fms(static_cast<std::uint8_t>(i));
				}

				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		};
		if (ImGui::IsItemHovered(default_hover_flags)){
			hovered = true;
		}
		dear::Tooltip{hovered} && [] {
			ImGui::TextUnformatted("Frequency modulation sensivity");
		};
		if(const auto new_val = handle_combo_scroll<oper::fms_values.size()>(op.fms())) {
			op.fms(*new_val);
		}
	}
	void ym2612_edit::render_feedback() {
		ImGui::TextUnformatted("Feedback");
		ImGui::TableNextColumn();
		auto &op = selected_patch().operators;
		const auto val = op.feedback();
		using oper = operators;
		ImGui::SetNextItemWidth(-1);
		dear::Combo{"##Feedback value", oper::string(val).data()} && [&op, &val] {
			for (const auto &current : list<oper::feedback_mode>()){
				const bool is_selected = val == current;
				if (ImGui::Selectable(oper::string(current).data(), is_selected)) {
					op.feedback(current);
				}

				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		};
		if(const auto new_val = handle_combo_scroll(val)) {
			op.feedback(*new_val);
		}
	}

	void ym2612_edit::render_algorithm() {
		ImGui::TextUnformatted("Algorithm");
		ImGui::TableNextColumn();
		auto &op = selected_patch().operators;
		const auto val = op.algorithm();
		using oper = operators;
		ImGui::SetNextItemWidth(-1);
		dear::Combo{"##Algorithm value", oper::string(val).data()} && [&op, &val] {
			for (const auto &current : list<oper::algorithm_mode>()){
				const bool is_selected = val == current;
				if (ImGui::Selectable(oper::string(current).data(), is_selected)) {
					op.algorithm(current);
				}

				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		};
		if(const auto new_val = handle_combo_scroll(val)) {
			op.algorithm(*new_val);
		}
	}

	void ym2612_edit::render_transposition() {
		ImGui::TextUnformatted("Transposition");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(-1);
		dear::Combo{"##Transposition value", "0 (Default)"} && [] {
			// Todo
		};
	}

	void ym2612_edit::render_registers(const std::uint_fast8_t current_row) const {
		const std::uint_fast8_t offset = current_row * 8;
		const std::uint_fast8_t max = current_row == 3 ? 6 : 8;
		const auto width = ImGui::GetContentRegionAvail().x / 4;
		auto &op = const_cast<operators &>(selected_patch().operators); // InputScalar takes non-const pointer but can't actually write in this case
		for(std::uint_fast8_t i = 0; i < max; i++) {
			if(i == 4) {
				ImGui::TableNextColumn();
			} else if(i != 0) {
				ImGui::SameLine();
			}
			ImGui::SetNextItemWidth(width);
			ImGui::InputScalar("##", ImGuiDataType_U8, &op.registers[offset + i], nullptr, nullptr, "%02X");
		}
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

	void ym2612_edit::on_close() {}
} // MID3SMPS
