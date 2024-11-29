#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include <future>
#include <mutex>
#include <chrono>
#include <thread>

#include <filesystem>
namespace fs = std::filesystem;

#include "../print/EditorLogger.hpp"
#include "Types.hpp"
#include "ShaderResource.hpp"

namespace resource {
	class ResourceManager
	{
	private:
		inline static std::unordered_map<std::string, std::weak_ptr<TextureData>> textureCache;
		inline static std::unordered_map<std::string, std::weak_ptr<HDRData>> hdrCache;
		inline static std::unordered_map<std::string, std::weak_ptr<AudioData>> audioCache;
		inline static std::unordered_map<std::string, std::weak_ptr<MeshesData>> meshCache;
		inline static std::unordered_map<std::string, std::weak_ptr<std::vector<ShaderModel>>> shaderCache;

		inline static std::mutex cacheMutex;
		inline static std::jthread cleanupThread;
		inline static std::condition_variable cleanupCondition;
		inline static std::atomic<bool> running;

	public:
		static FileType readHeaderFile(const fs::path& filePath);
		static std::future <std::shared_ptr<TextureData>> loadTextureAsync(std::string_view path);
		static std::future <std::shared_ptr<HDRData>> loadHDRAsync(std::string_view path);
		static std::future <std::shared_ptr<AudioData>> loadAudioAsync(std::string_view path);
		static std::future <std::shared_ptr<MeshesData>> loadMeshAsync(std::string_view path);
		static std::future <std::shared_ptr<std::vector<ShaderModel>>> loadShaderAsync(std::string_view path);

		static void init();
		static void cleanUp();

	private:
		static void periodicCleanup();
		static void unloadUnusedResources();
		static void notifyThread();
		static void releaseResources();

		template <typename T, typename LoaderFunc>
		static std::future<std::shared_ptr<T>> loadResourceAsync(
			std::string_view path,
			std::unordered_map<std::string, std::weak_ptr<T>>& cache,
			LoaderFunc loader);

		template <typename T>
		static std::future<T> make_ready_future(T value) {
			std::promise<T> promise;
			promise.set_value(std::move(value));
			return promise.get_future();
		}
	};

	template<typename T, typename LoaderFunc>
	inline std::future<std::shared_ptr<T>> ResourceManager::loadResourceAsync(std::string_view path, std::unordered_map<std::string, std::weak_ptr<T>>& cache, LoaderFunc loader)
	{
		if (auto resource = cache[path.data()].lock()) {
			return make_ready_future(resource);
		}

		return std::async(std::launch::async, [path, loader, &cache]() {
			try {
				auto resource = std::make_shared<T>(loader(path));
				{
					std::scoped_lock lock(cacheMutex);
					cache[path.data()] = resource;
				}
				return resource;
			}
			catch (const std::exception& e) {
				vfLogError("Error loading resource: {} - {}", path, e.what());
				return std::shared_ptr<T>(nullptr);
			}
			});
	}
}


