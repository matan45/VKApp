#include "ContentBrowser.hpp"
#include "string/StringUtil.hpp"


namespace windows {
	void ContentBrowser::draw()
	{
		if (ImGui::Begin("Content Folder")) {
			// Show current path and navigation options
			ImGui::Text("Current Path: %s", StringUtil::wstringToUtf8(currentPath.wstring()).c_str());

			if (ImGui::Button("Up")) {
				auto parentPath = currentPath.parent_path();
				navigateTo(parentPath);
			}

			// Display contents of the current directory
			for (const auto& asset : assets) {
				switch (asset.type) {
				case Asset::AssetType::Texture:
					ImGui::Text("[Texture] %s", asset.name.c_str());
					break;
				case Asset::AssetType::Model:
					ImGui::Text("[Model] %s", asset.name.c_str());
					break;
				case Asset::AssetType::Shader:
					ImGui::Text("[Shader] %s", asset.name.c_str());
					break;
				case Asset::AssetType::Other:
					if (fs::is_directory(asset.path)) {
						if (ImGui::Selectable(StringUtil::wstringToUtf8(asset.name).c_str(), false, ImGuiSelectableFlags_DontClosePopups)) {
							navigateTo(asset.path);
						}
					}
					else {
						ImGui::Text("[Other] %s", asset.name.c_str());
					}
					break;
				}
			}
		}
		ImGui::End();
	}

	void ContentBrowser::loadDirectory(const fs::path& path)
	{
		assets.clear();
		for (auto& entry : fs::directory_iterator(path)) {
			Asset asset;
			asset.path = entry.path().wstring();
			asset.name = entry.path().filename().wstring();
			if (entry.is_directory()) {
				asset.type = Asset::AssetType::Other; // Indicate it's a folder
			}
			else {
				// Determine the asset type by its file extension.
				auto ext = entry.path().extension();
				if (ext == ".png" || ext == ".jpg") {
					asset.type = Asset::AssetType::Texture;
				}
				else if (ext == ".obj" || ext == ".fbx") {
					asset.type = Asset::AssetType::Model;
				}
				else if (ext == ".vert" || ext == ".frag") {
					asset.type = Asset::AssetType::Shader;
				}
				else {
					asset.type = Asset::AssetType::Other;
				}
			}
			assets.push_back(asset);
		}
	}
}
