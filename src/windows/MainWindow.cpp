#include "MainWindow.hpp"

#include <imgui.h>
#include <imguiwrap.dear.h>

#include <ImGuiFileDialog.h>

#include <thread>
#include <fmt/core.h>

using namespace std::literals;

namespace MID3SMPS {
	template<typename T>
	void CachedRightText(dirtyable<T> &variable, std::string &cache, const std::string &emptyText){
		if(variable.dirty()){
			if(variable->empty()){
				cache = emptyText;
			} else{
				cache = variable->string();
			}
			variable.clearDirty();
		}
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth   = ImGui::CalcTextSize(cache.c_str()).x;

		//ImGui::SetCursorPosX((windowWidth - textWidth) * 0.98f);
		ImGui::TextUnformatted(cache.c_str());
	}

	void MainWindow::render(){
		{
			dear::Begin mainWindow("Mid3SMPS", &stayOpen, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			if(!mainWindow){ return; }
			showMenuBar();

			//const auto windowWidth = ImGui::GetWindowSize().x;
			const auto windowHeight = ImGui::GetWindowSize().y;

			dear::TextUnformatted("Loaded MIDI:"s); ImGui::SameLine();
			static std::string midiPathCache;
			CachedRightText(midiPath, midiPathCache, "No MIDI loaded"s);
			ImGui::NewLine();

			dear::TextUnformatted("Loaded Bank:"s); ImGui::SameLine();
			static std::string bankPathCache;
			CachedRightText(mappingPath, bankPathCache, "No bank loaded"s);
			ImGui::NewLine();

			ImGui::PushItemWidth(40);
			if(ImGui::InputInt("Ticks/Quarter", &ticksPerQuarter, 0, 0)){
				ticksPerQuarter = std::clamp(ticksPerQuarter, 0, 999);
			}
			ImGui::SameLine();

			ImGui::PushItemWidth(40);
			ImGui::InputInt("MIDI Resolution", &midiResolution, 0, 0, ImGuiInputTextFlags_ReadOnly);

			if(ImGui::InputInt("Tick Multiplier", &ticksMultiplier, 0, 0)){
				ticksMultiplier = std::clamp(ticksMultiplier, 0, 999);
			}

			ImGui::NewLine();
			if(ImGui::Button("Load Ins. Lib.")){
				const static std::string statusMsg = "Clicked \"Load Ins. Lib.\"";
				status = statusMsg;
			}
			ImGui::SameLine();
			if(ImGui::Button("Quick Convert")){
				const static std::string statusMsg = "Clicked \"Quick Convert\"";
				status = statusMsg;
			}

			ImGui::SetCursorPosY(windowHeight - 20);
			ImGui::SetNextWindowBgAlpha(0.75f);
			dear::Child("Status Bar") && [&]{
				dear::Text(fmt::format("{}", status));
			};
		}

		renderFileDialogs();
	}

	void MainWindow::showMenuBar(){
		ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
		dear::MenuBar menuBar;
		if(!menuBar){
			return;
		}
		dear::Menu("File") && [this]{
			if(ImGui::MenuItem("Open Midi", "Ctrl+O")){
				ImGuiFileDialog::Instance()->OpenDialog("OpenMidi", "Choose a Midi file", ".mid,.midi", ".");
			}
			dear::Menu("Open Recent") && [] {
				// Do stuff...
			};
			if(ImGui::MenuItem("Save", "Ctrl+S")){
				saveSmpsMenu();
			}
			if(ImGui::MenuItem("Save As..")){
				saveSmpsMenu(true);
			}
			ImGui::Separator();
			if(ImGui::MenuItem("Exit")){
				exitMenu();
			}
		};
		dear::Menu("Instruments & Mappings") && []{

		};
		dear::Menu("Extras") && [this]{
			static bool override;
			ImGui::Checkbox("Override Idle", &override);
			static FpsIdling::Override idleOverride;
			if(override && !idleOverride){
				idleOverride = windowHandler.idling().getOverride();
			} else if (!override && idleOverride){
				idleOverride = {};
			}
		};
	}

	fs::path getPathFromFileDialog(){
		std::string pathStr = ImGuiFileDialog::Instance()->GetCurrentFileName();
		if(pathStr.front() == '"' && pathStr.back() == '"'){ // Checks for a quoted path, like what windows gives when you do "Copy as path"
			pathStr = pathStr.substr(1, pathStr.length()-2);
		}
		fs::path path = pathStr;
		if(path.is_absolute()){
			return path;
		}
		fs::path parent = ImGuiFileDialog::Instance()->GetCurrentPath();
		return parent / path;
	}

	void MainWindow::renderFileDialogs(){
		if(ImGuiFileDialog::Instance()->Display("OpenMidi")){
			if(ImGuiFileDialog::Instance()->IsOk()){
				fs::path path = getPathFromFileDialog();
				if(fs::exists(path)){
					std::thread(&MainWindow::verifyAndSetMidi, this, std::move(path)).detach();
				} else {
					fmt::print(stderr, "{} is not a valid path", path.string());
				}
			}
			ImGuiFileDialog::Instance()->Close();
		}
		if(ImGuiFileDialog::Instance()->Display("saveSmps")){
			if(ImGuiFileDialog::Instance()->IsOk()){
				fs::path path = getPathFromFileDialog();
				std::thread(&MainWindow::saveSmps, this, std::move(path)).detach();
			}
			ImGuiFileDialog::Instance()->Close();
		}
	}

	void MainWindow::verifyAndSetMidi(fs::path&& midi){
		// Read raw from a MIDI file
		std::ifstream file{midi.string(), std::ios::binary};

		std::vector<uint8_t> bytes;
		bytes.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

// Parse
		parseResult = reader.parse(bytes);
		switch(parseResult){
			case libremidi::reader::invalid:
				// Throw error
				fmt::print(stderr, "Invalid midi file");
				return;
			case libremidi::reader::incomplete:
				// Print warning
				break;
			case libremidi::reader::complete:
				// Print message
				break;
			case libremidi::reader::validated:
				break;
			default:
				std::unreachable();
		}

		midiPath = midi;
		midiPath.markDirty();
	}

	void MainWindow::saveSmpsMenu(bool saveAs){
		if(saveAs) {
			ImGuiFileDialog::Instance()->OpenDialog("saveSmps", "Select a destination", ".bin", ".", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
		} else {
			std::thread(&MainWindow::saveSmps, this, *lastSmpsPath).detach();
		}
	}

	void MainWindow::saveSmps(const fs::path &path){
		lastSmpsPath = path; // Dirty is set automatically if path is different

	}

	void MainWindow::exitMenu(){

	}

	void MainWindow::openMappingMenu(){

	}

	void MainWindow::saveMappingMenu(){

	}

	void MainWindow::openInstrumentEditor(){

	}

	void MainWindow::openMappingsEditor(){

	}

	void MainWindow::openTempoCalculator(){

	}

	void MainWindow::onClose(){

	}

	bool MainWindow::keep() const{
		return stayOpen;
	}

	MainWindow::MainWindow(WindowHandler &handler) : windowHandler(handler){

	}

	MainWindow::~MainWindow() = default;
} // MID3SMPS