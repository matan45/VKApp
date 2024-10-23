#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include <future>
#include <mutex>
#include <chrono>
#include <thread>

#include "Types.hpp"

namespace resource {
	class ResourceManager
	{
	private:
		inline static std::unordered_map<std::string, std::weak_ptr<TextureData>> textureCache;
		inline static std::unordered_map<std::string, std::weak_ptr<AudioData>> audioCache;
		inline static std::unordered_map<std::string, std::weak_ptr<MeshData>> meshCache;

		inline static std::mutex cacheMutex;
		inline static std::jthread cleanupThread;
		inline static std::condition_variable cleanupCondition;
		inline static std::atomic<bool> running;

	public:
		static std::future <std::shared_ptr<TextureData>> loadTextureAsync(std::string_view path);
		static std::future <std::shared_ptr<AudioData>> loadAudioAsync(std::string_view path);
		static std::future <std::shared_ptr<MeshData>> loadMeshAsync(std::string_view path);

		static void init();
		static void cleanUp();

	private:
		static void periodicCleanup();
		static void unloadUnusedResources();
		static void notifyThread();
		static void releaseResources();

		template <typename T>
		static std::future<T> make_ready_future(T value) {
			std::promise<T> promise;
			promise.set_value(std::move(value));
			return promise.get_future();
		}
	};
}


