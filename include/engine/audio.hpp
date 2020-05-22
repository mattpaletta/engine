#pragma once

#include <string>
#include <map>

#if ENGINE_ENABLE_AUDIO
#include "AL/al.h"
#endif

#include <constants/position.hpp>

class AudioEngine {
public:
	AudioEngine();
	~AudioEngine();

	// Delete copy and move constructors & assignment
	AudioEngine(const AudioEngine&) = delete;
	AudioEngine& operator=(const AudioEngine&) = delete;
	AudioEngine(AudioEngine&&) = delete;
	AudioEngine&& operator=(AudioEngine&&) = delete;

	void Init();
	void Update();
	void Shutdown();

	void LoadSound(const std::string& strSoundName, bool is_3d = true, bool looping = false, bool stream = false);
	void UnLoadSound(const std::string& strSoundName);
	void Set3dListenerAndOrientation(const Position& position, const glm::vec3& loop, const glm::vec3& up);
	void Play(const std::string& strSoundName, const glm::vec3& vPos = { 0, 0, 0 }, float vVolumedB = 0.0f);
/*
	void StopChannel(int channelId);
	void PauseChannel(int channelId);
	void StopAllChannels();
	void SetChannel3dPosition(int channelId, const Position& position);
	void SetChannelVolume(int nChannelId, float fVolumedB);
*/
	bool IsPlaying(const std::string& strSoundName) const;
	bool isLoaded(const std::string& soundName) const;

private:
	bool isShutdown;
	struct SoundDefinition {
		SoundDefinition() = default;
		~SoundDefinition() = default;

		std::string name;
#if ENGINE_ENABLE_AUDIO
		ALuint source;
		ALuint buffer;

		ALfloat offset;
		ALenum state;
#endif
		std::string err_msg;
	};

	using SoundMap = std::map<std::string, SoundDefinition>;
	SoundMap sounds;
};