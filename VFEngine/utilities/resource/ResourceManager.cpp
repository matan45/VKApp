#include "ResourceManager.hpp"
#include "TextureResource.hpp"
#include "AudioResource.hpp"
#include "MeshResource.hpp"
#include "../print/EditorLogger.hpp"

namespace resource {

	void ResourceManager::periodicCleanup()
	{
		using namespace std::chrono_literals;
		std::unique_lock lock(cacheMutex);

		while (running) {
			// Wait for 5 minutes or until notified to wake up early.
			cleanupCondition.wait_for(lock, 5min);

			// Check if we should stop running before proceeding.
			if (!running) {
				break;
			}

			// Perform the cleanup.
			unloadUnusedResources();
		}
	}

	void ResourceManager::unloadUnusedResources()
	{
		std::scoped_lock lock(cacheMutex);
		std::erase_if(textureCache, [](const auto& pair) {
			return pair.second.expired();  // Remove the entry if the resource is no longer referenced
			});
		std::erase_if(audioCache, [](const auto& pair) {
			return pair.second.expired();  // Remove the entry if the resource is no longer referenced
			});
		std::erase_if(meshCache, [](const auto& pair) {
			return pair.second.expired();  // Remove the entry if the resource is no longer referenced
			});
	}

	std::future<std::shared_ptr<TextureData>> ResourceManager::loadTextureAsync(std::string_view path)
	{
		if (auto texture = textureCache[path.data()].lock()) {
			return make_ready_future(texture);
		}

		return std::async(std::launch::async, [path]() {
			try {
				auto texture = std::make_shared<TextureData>(TextureResource::loadTexture(path));
				{
					std::scoped_lock lock(cacheMutex);
					textureCache[path.data()] = texture;
				}

				return texture;
			}
			catch (const std::exception& e) {
				vfLogError("Error loading texture: {} - {}", path, e.what());
				return std::shared_ptr<TextureData>(nullptr);
			}
			});
	}

	std::future<std::shared_ptr<AudioData>> ResourceManager::loadAudioAsync(std::string_view path)
	{
		if (auto audio = audioCache[path.data()].lock()) {
			return make_ready_future(audio);
		}

		return std::async(std::launch::async, [path]() {
			try {
				auto audio = std::make_shared<AudioData>(AudioResource::loadAudio(path));
				{
					std::scoped_lock lock(cacheMutex);
					audioCache[path.data()] = audio;
				}

				return audio;
			}
			catch (const std::exception& e) {
				vfLogError("Error loading audio: {} - {}", path, e.what());
				return std::shared_ptr<AudioData>(nullptr);
			}
			});
	}

	std::future<std::shared_ptr<MeshData>> ResourceManager::loadMeshAsync(std::string_view path)
	{
		if (auto mesh = meshCache[path.data()].lock()) {
			return make_ready_future(mesh);
		}

		return std::async(std::launch::async, [path]() {
			try {
				auto mesh = std::make_shared<MeshData>(MeshResource::loadMesh(path));
				{
					std::scoped_lock lock(cacheMutex);
					meshCache[path.data()] = mesh;
				}

				return mesh;
			}
			catch (const std::exception& e) {
				vfLogError("Error loading mesh: {} - {}", path, e.what());
				return std::shared_ptr<MeshData>(nullptr);
			}
			});
	}

	void ResourceManager::init()
	{
		running = true;
		cleanupThread = std::jthread(&ResourceManager::periodicCleanup);
	}

	void ResourceManager::cleanUp()
	{
		
		notifyThread();
		cleanupCondition.notify_one(); // Notify the cleanup thread to wake up and exit.
		releaseResources();

	}

	void ResourceManager::notifyThread()
	{
		std::scoped_lock lock(cacheMutex);
		running = false;
	}

	void ResourceManager::releaseResources()
	{
		unloadUnusedResources();
		std::scoped_lock lock(cacheMutex);
		textureCache.clear();
		audioCache.clear();
		meshCache.clear();
	}

}
