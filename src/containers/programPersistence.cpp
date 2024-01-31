#include <spanstream>
#include <fmt/compile.h>

#include "programPersistence.hpp"

namespace MID3SMPS {
	void* ProgramPersistence::ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name) noexcept{
		persistence->mode = mode_from_string(name);
		return persistence.get();
	}

	void ProgramPersistence::ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line) noexcept{
		if(persistence.get() != entry) {
			return;
		}

		const std::string_view view = line;
		std::ispanstream input(view);

		switch(persistence->mode) {
			case Data:
				ReadData(input);
				return;
			case OpenHistory:
				ReadRecents(input);
				return;
			case None:
			default:
				break;
		}
	}

	void ProgramPersistence::ReadData(std::ispanstream &input) {
		std::string type;
		std::getline(input, type, '=');
		if(type == "path") {
			input >> persistence->last_config_;
		}
	}

	void ProgramPersistence::ReadRecents(std::ispanstream &input) {
		std::string midi;
		std::getline(input, midi);
		fs::path midi_path = midi;
		if(midi_path.empty()) return;
		persistence->insert_recent(std::move(midi_path));
	}

	void ProgramPersistence::WriteAll(ImGuiContext *, ImGuiSettingsHandler *, ImGuiTextBuffer *out_buf) noexcept{
		static constexpr auto header_fmt = FMT_COMPILE("[{}][{}]\n");
		if(!persistence->empty()) {
			static const auto header = fmt::format(header_fmt, TypeName, string(Data));
			out_buf->append(header.c_str());
			const auto config_path = fmt::format("path={}\n", persistence->last_config_.string());
			out_buf->append(config_path.c_str());

			// Close out section
			out_buf->append("\n");
		}
		if(!persistence->recent_midis_.empty()) {
			static const auto header = fmt::format(header_fmt, TypeName, string(OpenHistory));
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