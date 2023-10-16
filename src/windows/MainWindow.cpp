#include "MainWindow.hpp"

#include <imgui.h>
//#include <misc/cpp/imgui_stdlib.h>

#include <ImGuiFileDialog.h>

#include <thread>

namespace MID3SMPS {
	void MainWindow::render(){
		if(!ImGui::Begin("Mid3SMPS", &stayOpen, ImGuiWindowFlags_MenuBar)){
			ImGui::End();
			return;
		}
		showMenuBar();
		ImGui::SeparatorText("Loaded MIDI:");
		static std::string midiPathCache = "No MIDI loaded";
		if(midiPath.dirty()){
			midiPathCache = midiPath->string();
			midiPath.clearDirty();
		}
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth   = ImGui::CalcTextSize(midiPathCache.c_str()).x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.98f);
		ImGui::TextUnformatted(midiPathCache.c_str());

		ImGui::End();
		renderFileDialogs();
	}

	void MainWindow::showMenuBar(){
		ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
		if(!ImGui::BeginMenuBar()){
			return;
		}
		if(ImGui::BeginMenu("File")){
			if(ImGui::MenuItem("Open Midi", "Ctrl+O")){
				ImGuiFileDialog::Instance()->OpenDialog("OpenMidi", "Choose a Midi file", ".mid,.midi", ".");
			}
			if(ImGui::BeginMenu("Open Recent")){
				// Do stuff...
				ImGui::EndMenu();
			}
			if(ImGui::MenuItem("Save", "Ctrl+S")){
				saveSmpsMenu();
			}
			if(ImGui::MenuItem("Save As..")){
				saveSmpsMenu(true);
			}
			ImGui::Separator();
			if(ImGui::MenuItem("Exit")){

			}
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("Instruments & Mappings")){

			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("Extras")){

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	void MainWindow::renderFileDialogs(){
		if(ImGuiFileDialog::Instance()->Display("OpenMidi")){
			if(ImGuiFileDialog::Instance()->IsOk()){
				std::thread(&MainWindow::verifyAndSetMidi, this, ImGuiFileDialog::Instance()->GetFilePathName()).detach();
			}
			ImGuiFileDialog::Instance()->Close();
		}
		if(ImGuiFileDialog::Instance()->Display("saveSmps")){
			if(ImGuiFileDialog::Instance()->IsOk()){
				std::thread(&MainWindow::saveSmps, this, ImGuiFileDialog::Instance()->GetFilePathName()).detach();
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
	}

	void MainWindow::saveSmpsMenu(bool saveAs){
		if(saveAs ) ImGuiFileDialog::Instance()->OpenDialog("saveSmps", "Select a destination", ".bin", ".", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
		else std::thread(&MainWindow::saveSmps, this, *lastSmpsPath).detach();
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