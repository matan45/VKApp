#include "ConsoleLog.hpp"
#include "print/EditorLogger.hpp"

namespace windows {
	void ConsoleLog::draw()
	{
		if (ImGui::Begin("Console")) {
			if (ImGui::Button("clear"))
				util::imguiConsoleBuffer.clear();

			//TODO fps counter
			/**
			ImGui::SameLine();
			ImGui::Text("FPS: %.2f", utilities::resource::Time::getNumberFrame());
			ImGui::SameLine();
			ImGui::Text("Delta Time: %.4f", utilities::resource::Time::getDeltaTime());
			ImGui::Separator();*/
			ImGui::Separator();
			ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0, 0, 0 ,255 });
			ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);

			ImGui::PushStyleColor(ImGuiCol_Text, { 0, 255, 0 ,255 });

			// Display all messages from the buffer
			for (const auto& logEntry : util::imguiConsoleBuffer) {
				ImGui::TextUnformatted(logEntry.c_str());
			}

			ImGui::PopStyleColor();
			

			// Scroll to the bottom to show the latest log entry
			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);

			ImGui::PopStyleColor();
			ImGui::EndChild();
		}
		ImGui::End();
	}
}
