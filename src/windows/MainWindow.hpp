#pragma once

#include "Window.hpp"
#include "backend/WindowHandler.hpp"
#include "containers/dirtyable.hpp"

#include "libremidi/libremidi.hpp"
#include "libremidi/reader.hpp"

#include <filesystem>

namespace fs = std::filesystem;

namespace MID3SMPS {
	class MainWindow : public Window{
		WindowHandler &windowHandler;
		bool stayOpen = true;

		dirtyable<fs::path> midiPath{};
		dirtyable<fs::path> lastSmpsPath{};

		libremidi::reader reader{};
		libremidi::reader::parse_result parseResult{};

		dirtyable<fs::path> mappingPath{};

		void showMenuBar();
		void renderFileDialogs();
		void verifyAndSetMidi(fs::path&& midi);

		// File Menu
		//void openMidiMenu();
		void saveSmpsMenu(bool saveAs = false);
		void saveSmps(const fs::path &path);
		void exitMenu();

		// Instruments & Mappings Menu
		void openMappingMenu();
		void saveMappingMenu();

		void openInstrumentEditor();
		void openMappingsEditor();

		// Extras Menu
		void openTempoCalculator();
		bool convertSongTitle{};
		bool perFileInstruments{};
		bool autoReloadMidi = true;
		bool autoOptimizeMidi{};

	public:
		explicit MainWindow(WindowHandler &handler);
		~MainWindow() override;
		void render() override;
		void onClose() override;
		[[nodiscard]] bool keep() const override;
	};
} // MID3SMPS