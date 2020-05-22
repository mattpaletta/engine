#include "engine/audio.hpp"

#include <vector>

// HELPERS
#include <cassert>
#include <string>
#include <iostream>

#if ENGINE_ENABLE_AUDIO
#include "AL/alc.h"
#include "sndfile.h"

namespace openal {

    static void list_audio_devices(const ALCchar* devices) {
        const ALCchar* device = devices, * next = devices + 1;
        size_t len = 0;

        fprintf(stdout, "Devices list:\n");
        fprintf(stdout, "----------\n");
        while (device && *device != '\0' && next && *next != '\0') {
            fprintf(stdout, "%s\n", device);
            len = strlen(device);
            device += (len + 1);
            next += (len + 2);
        }
        fprintf(stdout, "----------\n");
    }

    /* InitAL opens a device and sets up a context using default attributes, making
     * the program ready to call OpenAL functions. */
    int InitAL(const int desired_device = 0) {
        const ALCchar* name;
        ALCdevice* device;
        ALCcontext* ctx;

        /* Open and initialize a device */
        device = NULL;
        if (desired_device != 0) {
            device = alcOpenDevice((const ALCchar*)std::to_string(desired_device).c_str());
            if (!device) {
                std::cerr << "Failed to open " << desired_device << ", trying default" << std::endl;
            }
        }
        if (!device) {
            char* s = (char*)alcGetString(device, ALC_DEFAULT_DEVICE_SPECIFIER);
            std::cout << "Default Audio Device: " << s << std::endl;
            device = alcOpenDevice(NULL);
        }

        if (!device) {
            std::cerr << "Could not open a device!" << std::endl;

            // Try and list devices
            const ALboolean enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
            if (enumeration == AL_FALSE) {
                list_audio_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
            } else {
                std::cout << "This device does not support enumeration OpenAL Extension." << std::endl;
            }

#if _WIN32 || _WIN64
            std::cout << "Try installing OpenAL: (https://www.openal.org/downloads) and installing `OpenAL Windows Installer`" << std::endl;
#endif
            return 1;
        }

        ctx = alcCreateContext(device, NULL);
        if (ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE) {
            if (ctx != NULL) {
                alcDestroyContext(ctx);
            }
            alcCloseDevice(device);
            std::cerr << "Could not set a context!" << std::endl;
            return 1;
        }

        name = NULL;
        if (alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT")) {
            name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
        }
        if (!name || alcGetError(device) != AL_NO_ERROR) {
            name = alcGetString(device, ALC_DEVICE_SPECIFIER);
        }
        std::cout << "Opened" << name << std::endl;

        return 0;
    }

    /* CloseAL closes the device belonging to the current context, and destroys the
     * context. */
    void CloseAL(void) {
        ALCdevice* device;
        ALCcontext* ctx;

        ctx = alcGetCurrentContext();
        if (ctx == NULL) {
            return;
        }

        device = alcGetContextsDevice(ctx);

        alcMakeContextCurrent(NULL);
        alcDestroyContext(ctx);
        alcCloseDevice(device);
    }

    const std::string FormatName(ALenum format) {
        switch (format) {
        case AL_FORMAT_MONO8: return "Mono, U8";
        case AL_FORMAT_MONO16: return "Mono, S16";
        case AL_FORMAT_STEREO8: return "Stereo, U8";
        case AL_FORMAT_STEREO16: return "Stereo, S16";
        }
        return "Unknown Format";
    }

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

    int altime_get(void) {
        static int start_time = 0;
        int cur_time;
        union {
            FILETIME ftime;
            ULARGE_INTEGER ulint;
        } systime;
        GetSystemTimeAsFileTime(&systime.ftime);
        /* FILETIME is in 100-nanosecond units, or 1/10th of a microsecond. */
        cur_time = (int)(systime.ulint.QuadPart / 10000);

        if (!start_time)
            start_time = cur_time;
        return cur_time - start_time;
    }

    void al_nssleep(unsigned long nsec) {
        Sleep(nsec / 1000000);
    }

#else

#include <sys/time.h>
#include <unistd.h>
#include <time.h>

    int altime_get(void) {
        static int start_time = 0u;
        int cur_time;

#if _POSIX_TIMERS > 0
        struct timespec ts;
        int ret = clock_gettime(CLOCK_REALTIME, &ts);
        if (ret != 0) return 0;
        cur_time = (int)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#else /* _POSIX_TIMERS > 0 */
        struct timeval tv;
        int ret = gettimeofday(&tv, NULL);
        if (ret != 0) return 0;
        cur_time = (int)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif

        if (!start_time)
            start_time = cur_time;
        return cur_time - start_time;
    }

    void al_nssleep(unsigned long nsec) {
        struct timespec ts, rem;
        ts.tv_sec = (time_t)(nsec / 1000000000ul);
        ts.tv_nsec = (long)(nsec % 1000000000ul);
        while (nanosleep(&ts, &rem) == -1 && errno == EINTR) {
            ts = rem;
        }
    }

#endif


    /* LoadBuffer loads the named audio file into an OpenAL buffer object, and
     * returns the new buffer ID.
     */
    static ALuint LoadSound(const std::string& filename) {
        ALenum err, format;
        ALuint buffer;
        SNDFILE* sndfile;
        SF_INFO sfinfo;
        short* membuf;
        sf_count_t num_frames;
        ALsizei num_bytes;

        /* Open the audio file and check that it's usable. */
        sndfile = sf_open(filename.c_str(), SFM_READ, &sfinfo);
        if (!sndfile) {
            std::cerr << "Could not open audio in " << filename << " : " << sf_strerror(sndfile) << std::endl;
            return 0;
        }
        if (sfinfo.frames < 1 || sfinfo.frames >(sf_count_t) (INT_MAX / sizeof(short)) / sfinfo.channels) {
            std::cerr << "Bad sample count in " << filename << " (%" << sfinfo.frames << ")" << std::endl;;
            sf_close(sndfile);
            return 0;
        }

        /* Get the sound format, and figure out the OpenAL format */
        if (sfinfo.channels == 1) {
            format = AL_FORMAT_MONO16;
        } else if (sfinfo.channels == 2) {
            format = AL_FORMAT_STEREO16;
        } else {
            std::cout << "Unsupported channel count: " << sfinfo.channels << std::endl;
            sf_close(sndfile);
            return 0;
        }

        /* Decode the whole audio file to a buffer. */
        membuf = (short*)malloc((size_t)(sfinfo.frames * sfinfo.channels) * sizeof(short));

        num_frames = sf_readf_short(sndfile, membuf, sfinfo.frames);
        if (num_frames < 1) {
            free(membuf);
            sf_close(sndfile);
            std::cerr << "Failed to read samples in " << filename << " (%" << num_frames << ")" << std::endl;
            return 0;
        }
        num_bytes = (ALsizei)(num_frames * sfinfo.channels) * (ALsizei)sizeof(short);

        /* Buffer the audio data into a new buffer object, then free the data and
         * close the file.
         */
        buffer = 0;
        alGenBuffers(1, &buffer);
        alBufferData(buffer, format, membuf, num_bytes, sfinfo.samplerate);

        free(membuf);
        sf_close(sndfile);

        /* Check if an error occured, and clean up if so. */
        err = alGetError();
        if (err != AL_NO_ERROR) {
            std::cerr << "OpenAL Error: " << alGetString(err) << std::endl;
            if (buffer && alIsBuffer(buffer)) {
                alDeleteBuffers(1, &buffer);
            }
            return 0;
        }

        return buffer;
    }

    void playSound() {
        ALuint source, buffer;
        ALfloat offset;
        ALenum state;

        if (InitAL() != 0) {
            throw std::runtime_error("Failed to initalize OpenAL");
        }

        const std::string audioPath{ "../sounds/bleep.wav" };
        buffer = LoadSound(audioPath);
        if (!buffer) {
            CloseAL();
            throw std::runtime_error("Failed to initalize OpenAL");
        }

        source = 0;
        alGenSources(1, &source);
        alSourcei(source, AL_BUFFER, (ALint)buffer);
        assert(alGetError() == AL_NO_ERROR && "Failed to setup sound source");

        /* Play the sound until it finishes. */
        alSourcePlay(source);
        do {
            al_nssleep(10000000);
            alGetSourcei(source, AL_SOURCE_STATE, &state);

            /* Get the source offset. */
            alGetSourcef(source, AL_SEC_OFFSET, &offset);
            // std::cout << "\rOffset: " << offset;
            // std::cout << std::endl;
        } while (alGetError() == AL_NO_ERROR && state == AL_PLAYING);
        std::cout << std::endl;

        /* All done. Delete resources, and close down OpenAL. */
        alDeleteSources(1, &source);
        alDeleteBuffers(1, &buffer);

        CloseAL();
    }
} // End Namespace
#endif // End ENABLE_AUDIO

///////////////////////////////////////////
// Start AudioEngine Impl
//////////////////////////////////////////

AudioEngine::AudioEngine() : isShutdown(false), sounds() {
    this->Init();
}

AudioEngine::~AudioEngine() {
    if (!this->isShutdown) {
        this->Shutdown();
    }
}

void AudioEngine::Init() {
#if ENGINE_ENABLE_AUDIO
    if (openal::InitAL() != 0) {
        throw std::runtime_error("Failed to initalize OpenAL");
    }
#endif
}

void AudioEngine::Update() {
#if ENGINE_ENABLE_AUDIO
    for (auto& thisSound : this->sounds) {
        if (alGetError() == AL_NO_ERROR && thisSound.second.state == AL_PLAYING) {
            openal::al_nssleep(10000000);
            alGetSourcei(thisSound.second.source, AL_SOURCE_STATE, &thisSound.second.state);

            /* Get the source offset. */
            alGetSourcef(thisSound.second.source, AL_SEC_OFFSET, &thisSound.second.offset);
        }
    }
#endif
}

void AudioEngine::Shutdown() {
#if ENGINE_ENABLE_AUDIO
    openal::CloseAL();
#endif
    this->isShutdown = true;
}

void AudioEngine::LoadSound(const std::string& strSoundName, bool b3d, bool bLooping, bool bStream) {
    this->sounds.emplace(strSoundName, SoundDefinition()); // Create default SoundDefinition
#if ENGINE_ENABLE_AUDIO
    this->sounds.at(strSoundName).buffer = openal::LoadSound(strSoundName);

    if (!this->sounds.at(strSoundName).buffer) {
        this->sounds.at(strSoundName).err_msg = "Failed to initialize OpenAL";
    } else {
        if (bLooping) {
            // Defaults to false
            alSourcei(this->sounds.at(strSoundName).source, AL_LOOPING, AL_TRUE);
        }

        alGenSources(1, &this->sounds.at(strSoundName).source);
        alSourcei(this->sounds.at(strSoundName).source, AL_BUFFER, (ALint)this->sounds.at(strSoundName).buffer);
        if (alGetError() == AL_NO_ERROR) {
            this->sounds.at(strSoundName).err_msg = "Failed to setup sound source";
        }
    }
#endif
}

void AudioEngine::Play(const std::string& strSoundName, const glm::vec3& vPos, float vVolumedB) {
    auto& thisSound = this->sounds.at(strSoundName);
#if ENGINE_ENABLE_AUDIO
    alSourcePlay(thisSound.source);
#endif
}

void AudioEngine::UnLoadSound(const std::string& strSoundName) {
    auto& thisSound = this->sounds.at(strSoundName);

#if ENGINE_ENABLE_AUDIO
    /* All done. Delete resources */
    alDeleteSources(1, &thisSound.source);
    alDeleteBuffers(1, &thisSound.buffer);
#endif
    this->sounds.erase(strSoundName);
}

bool AudioEngine::IsPlaying(const std::string& strSoundName) const {
#if ENGINE_ENABLE_AUDIO
    return this->sounds.at(strSoundName).state == AL_PLAYING;
#else
    return false;
#endif
}

bool AudioEngine::isLoaded(const std::string& soundName) const {
    return this->sounds.find(soundName) != this->sounds.end();
}