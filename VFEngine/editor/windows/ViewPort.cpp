#include "ViewPort.hpp"
#include "imgui.h"


namespace windows {

	ViewPort::ViewPort(controllers::OffScreen& offscreen) :offscreen{ offscreen }
	{
	}

	void ViewPort::draw()
	{
		if (ImGui::Begin("ViewPort")) {
			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			ImGui::Image(offscreen.render(), ImVec2{viewportPanelSize.x, viewportPanelSize.y});
		}
		ImGui::End();
	}

}
