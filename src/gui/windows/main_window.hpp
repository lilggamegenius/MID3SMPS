#pragma once

#include <filesystem>
#include <libremidi/reader.hpp>

#include "window.hpp"
#include "ym2612_edit.hpp"
#include "containers/files/mid2smps/mapping.hpp"

namespace fs = std::filesystem;

namespace MID3SMPS {
	class main_window : public window {
		fs::path midi_path_{};
		fs::path last_smps_path_{};

		libremidi::reader reader_{};
		libremidi::reader::parse_result parse_result_{};

		fs::path mapping_path_{};
		M2S::mapping map_;

		std::unique_ptr<ym2612_edit> ym2612_edit_{};

		int ticks_per_quarter_{};
		int ticks_multiplier_{};

		int midi_resolution_{};

		mutable std::string status_;

		void cached_wrap_text(const std::string &label, cached_key_t key, const std::string &emptyText);

		void show_menu_bar();
		void render_file_dialogs();
		void verify_and_set_midi(fs::path &&midi);
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
		bool auto_reload_midi_   = true;
		bool auto_optimize_midi_ = true;

		bool chorus_cc_volume_boost_ = true;
		bool pan_law_compensation_   = true;

		// ReSharper disable CppInconsistentNaming
		static constexpr std::string OpenMidi    = "OpenMidi";
		static constexpr std::string SaveSmps    = "SaveSmps";
		static constexpr std::string OpenMapping = "OpenMapping";
		// ReSharper restore CppInconsistentNaming

	public:
		void render() override;
		void render_children() override;
		void on_close() override;

		[[nodiscard]] constexpr const char *window_title() const override {
			return "MID3SMPS";
		}

		main_window() = default;
		main_window(main_window &&main_window_) noexcept : window(std::move(main_window_)) {}

		~main_window() override = default;
	};

	fs::path get_path_from_file_dialog();
}
