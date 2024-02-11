#include <spanstream>
#include <fmt/compile.h>

#include "program_persistence.hpp"

namespace MID3SMPS {
	void* program_persistence::read_open(ImGuiContext*, ImGuiSettingsHandler*, const char* name) noexcept{
		persistence->mode = mode_from_string(name);
		return persistence.get();
	}

	void program_persistence::read_line(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line) noexcept{
		if(persistence.get() != entry) {
			return;
		}

		const std::string_view view = line;
		std::ispanstream input(view);

		switch(persistence->mode) {
			case data:
				read_data(input);
				return;
			case open_history:
				read_recents(input);
				return;
			case none:
			default:
				break;
		}
	}

	void program_persistence::read_data(std::ispanstream &input) {
		std::string line;
		std::getline(input, line, '=');
		if(line == "lastConfig") {
			std::getline(input, line);
			persistence->last_config_ = line;
		}
	}

	void program_persistence::read_recents(std::ispanstream &input) {
		std::string midi;
		std::getline(input, midi);
		fs::path midi_path = midi;
		if(midi_path.empty()) return;
		persistence->insert_recent(std::move(midi_path));
	}

	void program_persistence::write_all(ImGuiContext *, ImGuiSettingsHandler *, ImGuiTextBuffer *out_buf) noexcept{
		static constexpr auto header_fmt = FMT_COMPILE("[{}][{}]\n");
		if(!persistence->empty()) {
			static const auto header = fmt::format(header_fmt, TypeName, string(data));
			out_buf->append(header.c_str());
			const auto config_path = fmt::format("lastConfig={}\n", persistence->last_config_.string());
			out_buf->append(config_path.c_str());

			// Close out section
			out_buf->append("\n");
		}
		if(!persistence->recent_midis_.empty()) {
			static const auto header = fmt::format(header_fmt, TypeName, string(open_history));
			out_buf->append(header.c_str());
			for(const auto &midi_path : persistence->recent_midis_) {
				const auto str = fmt::format("{}\n", midi_path->string());
				out_buf->append(str.c_str());
			}

			// Close out section
			out_buf->append("\n");
		}
	}
} // MID3SMPS