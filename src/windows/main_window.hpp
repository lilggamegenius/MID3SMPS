#pragma once

#include <filesystem>
#include <libremidi/libremidi.hpp>
#include <libremidi/reader.hpp>

#include "window.hpp"
#include "containers/dirtyable.hpp"

namespace fs = std::filesystem;

namespace MID3SMPS {
	class ym2612_edit;
	class main_window : public window<main_window>{
		bool stay_open_ = true;

		dirtyable<fs::path> midi_path_{fs::path()};
		dirtyable<fs::path> last_smps_path_{fs::path()};

		libremidi::reader reader_{};
		libremidi::reader::parse_result parse_result_{};

		dirtyable<fs::path> mapping_path_{fs::path()};

		std::unique_ptr<ym2612_edit> ym2612_edit_{};

		int ticks_per_quarter_{};
		int ticks_multiplier_{};

		int midi_resolution_{};

		mutable std::string status_;

		void show_menu_bar();
		void render_file_dialogs();
		void verify_and_set_midi(fs::path&& midi);
		void open_midi(fs::path &&midi);
		void save_smps(const fs::path &path);
		void open_mapping(fs::path &&map_path, bool set_persistence = true);

		// File Menu
		//void openMidiMenu();
		void save_smps_menu(bool save_as = false);
		void exit_menu();

		// Instruments & Mappings Menu
		void open_mapping_menu();
		void save_mapping_menu();

		void open_instrument_editor();
		void open_mappings_editor();

		// Extras Menu
		void open_tempo_calculator();
		bool convert_song_title_{};
		bool per_file_instruments_{};
		bool auto_reload_midi_ = true;
		bool auto_optimize_midi_ = true;

		bool chorus_cc_volume_boost_ = true;
		bool pan_law_compensation_ = true;

		friend class window;
		void render_impl();
		void render_children_impl();
		void on_close_impl();
		[[nodiscard]] bool keep_impl() const;
		[[nodiscard]] static constexpr const char* window_title_impl(){
			return "MID3SMPS";
		}

		// ReSharper disable CppInconsistentNaming
		static constexpr std::string OpenMidi = "OpenMidi";
		static constexpr std::string SaveSmps = "SaveSmps";
		static constexpr std::string OpenMapping = "OpenMapping";
		// ReSharper restore CppInconsistentNaming
	};

	fs::path get_path_from_file_dialog();
} // MID3SMPS