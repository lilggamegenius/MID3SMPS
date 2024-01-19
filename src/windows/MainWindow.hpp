#pragma once

#include <filesystem>

#include <libremidi/libremidi.hpp>
#include <libremidi/reader.hpp>

#include "Window.hpp"
#include "backend/WindowHandler.hpp"
#include "containers/dirtyable.hpp"

namespace fs = std::filesystem;

namespace MID3SMPS {
	class MainWindow final : public Window<MainWindow>{
		WindowHandler &windowHandler;
		bool stayOpen = true;

		dirtyable<fs::path> midiPath{};
		dirtyable<fs::path> lastSmpsPath{};

		libremidi::reader reader{};
		libremidi::reader::parse_result parseResult{};

		dirtyable<fs::path> mappingPath{};

		int ticksPerQuarter = 0;
		int ticksMultiplier = 0;

		int midiResolution = 0;

		mutable std::string status;

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
		void render_impl();
		void render_children_impl();
		void on_close_impl();
		[[nodiscard]] bool keep_impl() const;
	};

	fs::path getPathFromFileDialog();
} // MID3SMPS