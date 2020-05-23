#pragma once

#include <string>
#include <map>
#include <memory>

#if ENGINE_ENABLE_AUDIO
#include "AL/al.h"
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"
#include <ogg/ogg.h>

#include <sstream>
#include <iostream>
#include <fstream>
#endif

#include <constants/position.hpp>

struct SoundDefinition {
	const static std::size_t NUM_BUFFERS = 4;
	const static ALsizei BUFFER_SIZE = 65536;

	SoundDefinition() {};
	~SoundDefinition() {};

	// Not Copyable

	std::string name;
#if ENGINE_ENABLE_AUDIO
	ALuint source;
	ALuint buffer;

	ALfloat offset;
	ALenum state;
	bool is_ogg = false;

	// Different for OGG Files
	ALuint buffers[NUM_BUFFERS];
	std::string filename;
	std::ifstream file;
	std::uint8_t channels;
	std::int32_t sampleRate;
	std::uint8_t bitsPerSample;
	ALsizei size;

	// ALuint source;
	ALsizei sizeConsumed = 0;
	ALenum format;
	OggVorbis_File oggVorbisFile;
	std::int_fast32_t oggCurrentSection = 0;
	std::size_t duration;

#endif
	std::string err_msg;
};

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

	using SoundMap = std::map<std::string, std::unique_ptr<SoundDefinition>>;
	SoundMap sounds;
};