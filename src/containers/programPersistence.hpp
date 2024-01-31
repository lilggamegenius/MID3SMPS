#pragma once

#include <filesystem>
#include <imgui_internal.h>
#include <memory>
#include <vector>

namespace MID3SMPS {
	namespace fs = std::filesystem;

	class ProgramPersistence {
		friend class MainWindow;
		enum Mode{
			None,
			Data,
			OpenHistory
		} mode = None;

		fs::path last_config_;
		std::vector<std::unique_ptr<fs::path>> recent_midis_;

	public:
		static constexpr auto TypeName = "ProgramPersistence";

		static void WriteAll(ImGuiContext*, ImGuiSettingsHandler*, ImGuiTextBuffer* out_buf) noexcept;
		static void* ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name) noexcept;
		static void ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line) noexcept;

		static void ReadData(std::ispanstream &input);
		static void ReadRecents(std::ispanstream &input);

		[[nodiscard]] constexpr auto& last_config() const noexcept{
			return last_config_;
		}

		[[nodiscard]] constexpr auto& recent_midis() const noexcept{
			return recent_midis_;
		}

		void insert_recent(fs::path &&path) {
			for(auto iter = recent_midis_.begin(); iter != recent_midis_.end(); ++iter) {
				if(auto &ptr = *iter; *ptr == path) {
					recent_midis_.erase(iter);
					break;
				}
			}
			recent_midis_.push_back(std::make_unique<fs::path>(path)); // Insert path at end
		}

	private:
		static constexpr Mode mode_from_string(const std::string_view view) noexcept{
			if(view == "Data") return Data;
			if(view == "OpenHistory") return OpenHistory;
			return None;
		}

		static constexpr std::string_view string(const Mode mode) noexcept{
			switch(mode) {
				case Data:
					return "Data";
				case OpenHistory:
					return "OpenHistory";
				default:
					return "None";
			}
		}

		[[nodiscard]] constexpr bool empty() const noexcept {
			if(!last_config_.empty()) return false;
			return true;
		}
	};

	inline auto persistence = std::make_unique<ProgramPersistence>();
} // MID3SMPS