#include "MainWindow.hpp"
#include "containers/programPersistence.hpp"

#include <imgui.h>
#include <ImGuiFileDialog.h>
#include <imguiwrap.dear.h>
#include <ranges>
#include <thread>
#include <fmt/core.h>

using namespace std::literals;

static const IGFD::FileDialogConfig DefaultFileDialogConfig{
	.path = "",
	.fileName = "",
	.filePathName = ".",
	.flags = ImGuiFileDialogFlags_ConfirmOverwrite,
	.sidePane = nullptr,
	.userFileAttributes = nullptr
};

namespace MID3SMPS {
	template<typename T>
	void cached_wrap_text(const std::string &label, dirtyable<T> &variable, std::string &cache, const std::string &emptyText) {
		if(variable.dirty()) {
			if(!variable.has_value() || variable->empty()) {
				cache = emptyText;
			} else {
				if constexpr(std::is_same_v<T, fs::path>) {
					cache = variable->filename().string();
				} else {
					cache = variable->string();
				}
			}
			variable.clearDirty();
		}

		const auto textWidth   = ImGui::CalcTextSize(cache.c_str()).x;
		const auto labelLength = ImGui::CalcTextSize(label.c_str()).x;

		const auto windowWidth = ImGui::GetWindowSize().x;

		dear::TextUnformatted(label);
		const auto combined = textWidth + labelLength + 30;
		if(combined <= windowWidth) {
			ImGui::SameLine();
		}
		dear::TextUnformatted(cache);
	}

	void MainWindow::render_impl() {
		dear::Begin(window_title(), &stayOpen, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse) && [&] {
			showMenuBar();

			constexpr float minWidth  = 200;
			constexpr float minHeight = 200;

			const auto windowWidth  = ImGui::GetWindowSize().x;
			const auto windowHeight = ImGui::GetWindowSize().y;

			const bool meets_min_width  = windowWidth > minWidth;
			const bool meets_min_height = windowHeight > minHeight;

			static std::string midiPathCache;
			cached_wrap_text("Loaded MIDI:"s, midiPath, midiPathCache, "No MIDI loaded"s);
			if(meets_min_height) {
				ImGui::NewLine();
			}

			static std::string bankPathCache;
			cached_wrap_text("Loaded Bank:"s, mappingPath, bankPathCache, "No bank loaded"s);
			if(meets_min_height) {
				ImGui::NewLine();
			}

			ImGui::PushItemWidth(40);
			if(ImGui::InputInt("Ticks/Quarter", &ticksPerQuarter, 0, 0)) {
				ticksPerQuarter = std::clamp(ticksPerQuarter, 0, 999);
			}
			if(meets_min_width && windowWidth > minWidth + 50) {
				ImGui::SameLine();
			}

			ImGui::PushItemWidth(40);
			ImGui::InputInt("MIDI Resolution", &midiResolution, 0, 0, ImGuiInputTextFlags_ReadOnly);

			if(ImGui::InputInt("Tick Multiplier", &ticksMultiplier, 0, 0)) {
				ticksMultiplier = std::clamp(ticksMultiplier, 0, 999);
			}

			if(meets_min_height) {
				ImGui::NewLine();
			}
			if(ImGui::Button("Load Ins. Lib.")) {
				const static std::string statusMsg = "Clicked \"Load Ins. Lib.\"";
				status                             = statusMsg;
			}
			if(meets_min_width) {
				ImGui::SameLine();
			}
			if(!meets_min_width && meets_min_height) {
				ImGui::NewLine();
			}
			if(ImGui::Button("Quick Convert")) {
				const static std::string statusMsg = "Clicked \"Quick Convert\"";
				status                             = statusMsg;
			}

			ImGui::SetCursorPosY(windowHeight - 20);
			ImGui::SetNextWindowBgAlpha(0.75f);
			dear::Child("Status Bar") && [&] {
				dear::Text(fmt::format("{}", status));
			};
		};

		render_children_impl();
	}

	void MainWindow::render_children_impl() {
		renderFileDialogs();
	}

	void MainWindow::showMenuBar() {
		ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
		dear::MenuBar() && [this] {
			dear::Menu("File") && [this] {
				if(ImGui::MenuItem("Open Midi", "Ctrl+O")) {
					ImGuiFileDialog::Instance()->OpenDialog("OpenMidi", "Choose a Midi file", ".mid,.midi", DefaultFileDialogConfig);
				}
				dear::Menu("Open Recent") && [this] {
					const auto &paths = persistence->recent_midis();
					for(const auto &current_path : std::ranges::reverse_view(paths)) {
						ImGui::PushID(&current_path);
						#ifdef __WIN32 // Windows being windows uses UTF-16 instead of UTF-8
						std::string str = current_path->string(); // Convert to UTF-8
						if(ImGui::MenuItem(str.c_str())) {
							verifyAndSetMidi(fs::path(*current_path));
						}
						#else
						if(ImGui::MenuItem(iter->c_str())) {
							verifyAndSetMidi(fs::path(*iter));
						}
						#endif
						ImGui::PopID();
					}
				};
				if(ImGui::MenuItem("Save", "Ctrl+S")) {
					saveSmpsMenu();
				}
				if(ImGui::MenuItem("Save As..")) {
					saveSmpsMenu(true);
				}
				ImGui::Separator();
				if(ImGui::MenuItem("Exit")) {
					exitMenu();
				}
			};
			dear::Menu("Instruments & Mappings") && [this] {
				if(ImGui::MenuItem("Open mapping configuration", "F5")) {
					openMappingMenu();
				}
				if(ImGui::MenuItem("Save mapping configuration", "F6")) {
					saveMappingMenu();
				}
				ImGui::Separator();
				if(ImGui::MenuItem("Open instrument editor", "Ctrl+I")) {
					openInstrumentEditor();
				}
				if(ImGui::MenuItem("Open mappings editor", "Ctrl+M")) {
					openMappingsEditor();
				}
			};
			dear::Menu("Extras") && [this] {
				if(ImGui::MenuItem("Tempo calculator", "Ctrl+T")) {
					openTempoCalculator();
				}
				ImGui::Checkbox("Convert song title", &convertSongTitle);
				ImGui::Checkbox("Per-file instruments", &perFileInstruments);
				ImGui::Checkbox("Auto reload MIDI", &autoReloadMidi);
				ImGui::Checkbox("Auto optimize MIDI", &autoOptimizeMidi);
				ImGui::Checkbox("Chorus CC volume boost", &chorusCCVolumeBoost);
				ImGui::Checkbox("Pan law compensation", &panLawCompensation);
				#if DEBUG
				ImGui::Separator();
				static bool override;
				ImGui::Checkbox("Override Idle", &override);
				static FpsIdling::Override idleOverride;
				if(override && !idleOverride) {
					idleOverride = windowHandler.idling().getOverride();
				} else if(!override && idleOverride) {
					idleOverride = {};
				}
				#endif
			};
		};
	}

	fs::path getPathFromFileDialog() {
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

	void MainWindow::renderFileDialogs() {
		if(ImGuiFileDialog::Instance()->Display("OpenMidi")) {
			if(ImGuiFileDialog::Instance()->IsOk()) {
				if(fs::path path = getPathFromFileDialog(); fs::exists(path)) {
					status = fmt::format("Loading {}", path.string());
					std::thread(&MainWindow::verifyAndSetMidi, this, std::move(path)).detach();
				} else {
					status = fmt::format("{} is not a valid path", path.string());
					fmt::print(stderr, "{}", status);
				}
			}
			ImGuiFileDialog::Instance()->Close();
		}
		if(ImGuiFileDialog::Instance()->Display("saveSmps")) {
			if(ImGuiFileDialog::Instance()->IsOk()) {
				fs::path path = getPathFromFileDialog();
				std::thread(&MainWindow::saveSmps, this, std::move(path)).detach();
			}
			ImGuiFileDialog::Instance()->Close();
		}
	}

	void MainWindow::verifyAndSetMidi(fs::path &&midi) {
		// Read raw from a MIDI file
		std::ifstream file{midi.string(), std::ios::binary};

		std::vector<uint8_t> bytes;
		bytes.assign(std::istreambuf_iterator(file), std::istreambuf_iterator<char>());

		// Parse
		parseResult = reader.parse(bytes);
		switch(parseResult) {
			case libremidi::reader::invalid:
				// Throw error
				status = fmt::format("Invalid midi file");
				fmt::print(stderr, "{}", status);
				return;
			case libremidi::reader::incomplete: status = fmt::format("Midi file loading incomplete");
				break;
			case libremidi::reader::complete: status = fmt::format("Midi file loading complete but not validated");
				break;
			case libremidi::reader::validated: status = fmt::format("Midi file loaded and validated");
				break;
			default: std::unreachable();
		}

		midiPath = midi;
		midiPath.markDirty();
		persistence->insert_recent(std::move(midi));
	}

	void MainWindow::saveSmpsMenu(bool saveAs) {
		if(saveAs) {
			ImGuiFileDialog::Instance()->OpenDialog("saveSmps", "Select a destination", ".bin", DefaultFileDialogConfig);
		} else {
			std::thread(&MainWindow::saveSmps, this, *lastSmpsPath).detach();
		}
	}

	void MainWindow::saveSmps(const fs::path &path) {
		lastSmpsPath = path; // Dirty is set automatically if path is different
	}

	void MainWindow::exitMenu() {}

	void MainWindow::openMappingMenu() {}

	void MainWindow::saveMappingMenu() {}

	void MainWindow::openInstrumentEditor() {}

	void MainWindow::openMappingsEditor() {}

	void MainWindow::openTempoCalculator() {}

	void MainWindow::on_close_impl() {}

	bool MainWindow::keep_impl() const {
		return stayOpen;
	}

	const char *MainWindow::window_title_impl() {
		return "MID3SMPS";
	}

	MainWindow::MainWindow(WindowHandler &handler) : windowHandler(handler) {}
} // MID3SMPS
