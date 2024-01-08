#pragma once

#include "MainWindow.hpp"

namespace MID3SMPS {
	class YM2612Edit : public Window{
	private:
		friend class MainWindow;

	public:
		void render() override;

		void renderChildren() override;

		void onClose() override;

		[[nodiscard]] bool keep() const override;

	};
} // MID3SMPS