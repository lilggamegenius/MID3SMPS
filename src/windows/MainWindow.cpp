#include "MainWindow.hpp"

#include "imgui.h"

#include "ImGuiFileDialog.h"

#include <thread>

namespace MID3SMPS {
	bool MainWindow::render(){
		if(!ImGui::Begin("Mid3SMPS", &stayOpen, ImGuiWindowFlags_MenuBar)){
			ImGui::End();
			return stayOpen;
		}
		showMenuBar();

		ImGui::End();
		renderFileDialogs();
		return stayOpen;
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
		std::ifstream file{midi, std::ios::binary};

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
		}

		currentMidiPath = midi;
	}

	void MainWindow::saveSmpsMenu(bool saveAs){
		if(saveAs) ImGuiFileDialog::Instance()->OpenDialog("saveSmps", "Select a destination", ".bin", ".", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
		else std::thread(&MainWindow::saveSmps, this, lastSmpsPath).detach();
	}

	void MainWindow::saveSmps(const fs::path &path){
		if(path != lastSmpsPath) lastSmpsPath = path;

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

	MainWindow::MainWindow(WindowHandler &windowHandler) : windowHandler(windowHandler){

	}

	MainWindow::~MainWindow() = default;
} // MID3SMPS