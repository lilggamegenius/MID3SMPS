#include <ImGuiFileDialog.h>
#include <imgui.h>
#include <imguiwrap.dear.h>
#include <ranges>
#include <thread>
#include <fmt/core.h>

#include "main_window.hpp"

#include "gui/backend/window_handler.hpp"

#include "containers/program_persistence.hpp"
#include "containers/files/mid2smps/mapping.hpp"

static const IGFD::FileDialogConfig default_file_dialog_config{
	.path = "",
	.fileName = "",
	.filePathName = ".",
	.flags = ImGuiFileDialogFlags_ConfirmOverwrite,
	.sidePane = nullptr,
	.userFileAttributes = nullptr
};

namespace MID3SMPS {
	void main_window::cached_wrap_text(const std::string &label, cached_key_t key, const std::string &emptyText) {
		const auto &cache = cached_strings.insert({key, emptyText}).first->second; // Sets the cache to the default text if it doesn't already exist

		const auto textWidth   = ImGui::CalcTextSize(cache.c_str()).x;
		const auto labelLength = ImGui::CalcTextSize(label.c_str()).x;

		const auto windowWidth = ImGui::GetWindowSize().x;

		dear::TextUnformatted(label);
		if(const auto combined = textWidth + labelLength + 30; combined <= windowWidth) {
			ImGui::SameLine();
		}
		dear::TextUnformatted(cache);
	}

	void main_window::render() {
		static bool first_frame_completed = false;
		if(!first_frame_completed) [[unlikely]] {
			if(!persistence->empty()) {
				auto map(persistence->last_config_);
				open_mapping(std::move(map), false);
			}
		}
		dear::Begin{window_title(), &stay_open_, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse} && [this] {
			show_menu_bar();

			constexpr float minWidth  = 200;
			constexpr float minHeight = 200;

			const auto windowWidth  = ImGui::GetWindowSize().x;
			const auto windowHeight = ImGui::GetWindowSize().y;

			const bool meets_min_width  = windowWidth > minWidth;
			const bool meets_min_height = windowHeight > minHeight;

			using namespace std::literals;
			cached_wrap_text("Loaded MIDI: "s, &midi_path_, "No MIDI loaded"s);
			if(meets_min_height) {
				ImGui::NewLine();
			}

			cached_wrap_text("Loaded Bank: "s, &map_.gyb(), "No bank loaded"s);
			if(meets_min_height) {
				ImGui::NewLine();
			}

			ImGui::PushItemWidth(40);
			if(ImGui::InputInt("Ticks/Quarter", &ticks_per_quarter_, 0, 0)) {
				ticks_per_quarter_ = std::clamp(ticks_per_quarter_, 0, 999);
			}
			if(meets_min_width && windowWidth > minWidth + 50) {
				ImGui::SameLine();
			}

			ImGui::PushItemWidth(40);
			ImGui::InputInt("MIDI Resolution", &midi_resolution_, 0, 0, ImGuiInputTextFlags_ReadOnly);

			if(ImGui::InputInt("Tick Multiplier", &ticks_multiplier_, 0, 0)) {
				ticks_multiplier_ = std::clamp(ticks_multiplier_, 0, 999);
			}

			if(meets_min_height) {
				ImGui::NewLine();
			}
			if(ImGui::Button("Load Ins. Lib.")) {
				const static std::string statusMsg = "Clicked \"Load Ins. Lib.\"";
				status_                             = statusMsg;
			}
			if(meets_min_width) {
				ImGui::SameLine();
			}
			if(!meets_min_width && meets_min_height) {
				ImGui::NewLine();
			}
			if(ImGui::Button("Quick Convert")) {
				const static std::string statusMsg = "Clicked \"Quick Convert\"";
				status_                             = statusMsg;
			}

			ImGui::SetCursorPosY(windowHeight - 20);
			ImGui::SetNextWindowBgAlpha(0.75f);
			dear::Child{"Status Bar"} && [&] {
				ImGui::TextUnformatted(status_.c_str());
			};
		};

		first_frame_completed = true;
	}

	void main_window::show_menu_bar() {
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {8, 0});
		ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
		dear::MenuBar{} && [this] {
			dear::Menu{"File"} && [this] {
				if(ImGui::MenuItem("Open Midi", "Ctrl+O")) {
					ImGuiFileDialog::Instance()->OpenDialog(OpenMidi, "Choose a Midi file", ".mid,.midi", default_file_dialog_config);
				}
				dear::Menu{"Open Recent"} && [this] {
					const auto &paths = persistence->recent_midis();
					for(const auto &current_path: std::ranges::reverse_view(paths)) {
						ImGui::PushID(&current_path);
						#ifdef __WIN32 // Windows being windows uses UTF-16 instead of UTF-8
						std::string str = current_path->string(); // Convert to UTF-8
						if(ImGui::MenuItem(str.c_str())) {
							open_midi(fs::path(*current_path));
						}
						#else
						if(ImGui::MenuItem(current_path->c_str())) {
							open_midi(fs::path(*current_path));
						}
						#endif
						ImGui::PopID();
					}
				};
				if(ImGui::MenuItem("Save", "Ctrl+S")) {
					save_smps_menu();
				}
				if(ImGui::MenuItem("Save As..")) {
					save_smps_menu(true);
				}
				ImGui::Separator();
				if(ImGui::MenuItem("Exit")) {
					exit_menu();
				}
			};
			dear::Menu{"Instruments & Mappings"} && [this] {
				if(ImGui::MenuItem("Open mapping configuration", "F5")) {
					open_mapping_menu();
				}
				if(ImGui::MenuItem("Save mapping configuration", "F6")) {
					save_mapping_menu();
				}
				ImGui::Separator();
				if(ImGui::MenuItem("Open instrument editor", "Ctrl+I")) {
					open_instrument_editor();
				}
				if(ImGui::MenuItem("Open mappings editor", "Ctrl+M")) {
					open_mappings_editor();
				}
			};
			dear::Menu{"Extras"} && [this] {
				if(ImGui::MenuItem("Tempo calculator", "Ctrl+T")) {
					open_tempo_calculator();
				}
				ImGui::Checkbox("Convert song title", &convert_song_title_);
				ImGui::Checkbox("Per-file instruments", &per_file_instruments_);
				ImGui::Checkbox("Auto reload MIDI", &auto_reload_midi_);
				ImGui::Checkbox("Auto optimize MIDI", &auto_optimize_midi_);
				ImGui::Checkbox("Chorus CC volume boost", &chorus_cc_volume_boost_);
				ImGui::Checkbox("Pan law compensation", &pan_law_compensation_);
				if constexpr (debug_mode) {
					ImGui::Separator();
					static bool override;
					ImGui::Checkbox("Override Idle", &override);
					if(override) {
						handler.idling.override_this_frame = true;
					}

					ImGui::Checkbox("Show demo window", &MID3SMPS::show_demo_window);
				}
			};
		};
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
	}

	fs::path get_path_from_file_dialog() {
		auto pathStr = ImGuiFileDialog::Instance()->GetCurrentFileName();
		if(pathStr[0] == '"' && *--pathStr.end() == '"') { // Handle windows' "Copy as path" option
			pathStr = pathStr.substr(1, pathStr.length() - 2);
		}
		fs::path path = pathStr;
		if(path.is_absolute()) {
			return path;
		}
		const fs::path parent = ImGuiFileDialog::Instance()->GetCurrentPath();
		return parent / path;
	}

	void main_window::render_file_dialogs() {
		if(ImGuiFileDialog::Instance()->Display(OpenMidi)) {
			if(ImGuiFileDialog::Instance()->IsOk()) {
				if(auto path = get_path_from_file_dialog(); fs::exists(path)) {
					open_midi(std::move(path));
				} else {
					status_ = fmt::format("{} is not a valid path", path.string());
					fmt::print(stderr, "{}", status_);
				}
			}
			ImGuiFileDialog::Instance()->Close();
		}
		if(ImGuiFileDialog::Instance()->Display(SaveSmps)) {
			if(ImGuiFileDialog::Instance()->IsOk()) {
				std::thread(&main_window::save_smps, this, get_path_from_file_dialog()).detach();
			}
			ImGuiFileDialog::Instance()->Close();
		}
		if(ImGuiFileDialog::Instance()->Display(OpenMapping)) {
			if(ImGuiFileDialog::Instance()->IsOk()) {
				if(auto path = get_path_from_file_dialog(); fs::exists(path)) {
					open_mapping(std::move(path));
				} else {
					status_ = fmt::format("{} is not a valid path", path.string());
					fmt::print(stderr, "{}", status_);
				}
			}
			ImGuiFileDialog::Instance()->Close();
		}
	}

	void main_window::verify_and_set_midi(fs::path &&midi) {
		// Read raw from a MIDI file
		std::ifstream file{midi, std::ios::binary};

		std::vector<uint8_t> bytes;
		bytes.assign(std::istreambuf_iterator(file), std::istreambuf_iterator<char>());

		// Parse
		parse_result_ = reader_.parse(bytes);
		switch(parse_result_) {
			case libremidi::reader::invalid:
				// Throw error
				status_ = fmt::format("Invalid midi file");
				fmt::print(stderr, "{}", status_);
				return;
			case libremidi::reader::incomplete:
				status_ = fmt::format("Midi file loading incomplete");
				break;
			case libremidi::reader::complete:
				status_ = fmt::format("Midi file loading complete but not validated");
				break;
			case libremidi::reader::validated:
				status_ = fmt::format("Midi file loaded and validated");
				break;
			default:
				std::unreachable();
		}

		midi_path_ = midi;
		cache_string(&midi_path_, midi_path_.filename().string());
		persistence->insert_recent(std::move(midi));
	}

	void main_window::save_smps_menu(bool save_as) {
		if(save_as) {
			ImGuiFileDialog::Instance()->OpenDialog(SaveSmps, "Select a destination", ".bin", default_file_dialog_config);
		} else {
			std::thread(&main_window::save_smps, this, last_smps_path_).detach();
		}
	}

	void main_window::open_midi(fs::path &&midi) {
		status_ = fmt::format("Loading {}", midi.string());
		std::thread(&main_window::verify_and_set_midi, this, std::move(midi)).detach();
	}

	void main_window::save_smps(const fs::path &path) {
		last_smps_path_ = path;
	}

	void main_window::open_mapping(fs::path &&map_path, bool set_persistence) {
		try {
			map_ = M2S::mapping(map_path);
			status_ = fmt::format("Loaded {}", map_path.filename().string());
			if(set_persistence) {
				persistence->last_config_ = map_path;
			}
			if(ym2612_edit_ && fs::exists(map_.gyb())) {
				ym2612_edit_->gyb_ = M2S::gyb{map_.gyb()};
			}
			cache_string(&map_.gyb(), map_.gyb().filename().string());
			mapping_path_ = std::move(map_path);
		} catch(const std::runtime_error &error) {
			status_ = fmt::format("Failed to load map: {}", error.what());
		}
	}

	void main_window::exit_menu() {}

	void main_window::open_mapping_menu() {
		ImGuiFileDialog::Instance()->OpenDialog(OpenMapping, "Choose a mapping file", ".cfg", default_file_dialog_config);
	}

	void main_window::save_mapping_menu() {}

	void main_window::open_instrument_editor() {
		if(!ym2612_edit_) {
			ym2612_edit_ = std::make_unique<ym2612_edit>();
			if(fs::exists(map_.gyb())) {
				ym2612_edit_->gyb_ = M2S::gyb{map_.gyb()};
			}
		} else {
			ImGui::SetWindowFocus(ym2612_edit_->window_title());
		}
		ym2612_edit_->stay_open_ = true;
	}

	void main_window::open_mappings_editor() {}

	void main_window::open_tempo_calculator() {}

	void main_window::on_close() {}

	void main_window::render_children() {
		render_file_dialogs();
		if(ym2612_edit_ && ym2612_edit_->keep()) {
			ym2612_edit_->render();
		}
	}
} // MID3SMPS
