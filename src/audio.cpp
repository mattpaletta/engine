#include "engine/audio.hpp"

#include <vector>

// HELPERS
#include <cassert>
#include <cstring> // std::memcpy
#include <string>
#include <iostream>
#include <fstream>

#if ENGINE_ENABLE_AUDIO
#include "AL/alc.h"
#include "sndfile.h"

namespace openal {
    namespace ogg {
        ///////////////////////
        // OGG Helper functions
        ///////////////////////
		#define alCheckErrors() check_al_errors(__FILE__, __LINE__)
        #define alCall(function, ...) alCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)
        #define alcCall(function, device, ...) alcCallImpl(__FILE__, __LINE__, function, device, __VA_ARGS__)

        void check_al_errors(const std::string& filename, const std::uint_fast32_t line) {
            ALCenum error = alGetError();
            if (error != AL_NO_ERROR) {
                std::cerr << "***ERROR*** (" << filename << ": " << line << ")" << std::endl;
                switch (error) {
                case AL_INVALID_NAME:
                    std::cerr << "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function";
                    break;
                case AL_INVALID_ENUM:
                    std::cerr << "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function";
                    break;
                case AL_INVALID_VALUE:
                    std::cerr << "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function";
                    break;
                case AL_INVALID_OPERATION:
                    std::cerr << "AL_INVALID_OPERATION: the requested operation is not valid";
                    break;
                case AL_OUT_OF_MEMORY:
                    std::cerr << "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory";
                    break;
                default:
                    std::cerr << "UNKNOWN AL ERROR: " << error;
                }
                std::cerr << std::endl;
            }
        }

        template<typename alFunction, typename... Params>
        auto alCallImpl(const char* filename, const std::uint_fast32_t line, alFunction function, Params... params)
            ->typename std::enable_if<std::is_same<void, decltype(function(params...))>::value, decltype(function(params...))>::type {
            function(std::forward<Params>(params)...);
            check_al_errors(filename, line);
        }

        template<typename alFunction, typename... Params>
        auto alCallImpl(const char* filename, const std::uint_fast32_t line, alFunction function, Params... params)
            ->typename std::enable_if<!std::is_same<void, decltype(function(params...))>::value, decltype(function(params...))>::type {
            auto ret = function(std::forward<Params>(params)...);
            check_al_errors(filename, line);
            return ret;
        }
        ///////////////////////
        // END OGG Helper functions
        ///////////////////////

        std::size_t read_ogg_callback(void* destination, std::size_t size1, std::size_t size2, void* fileHandle) {
            SoundDefinition* audioData = reinterpret_cast<SoundDefinition*>(fileHandle);

			std::size_t length = size1 * size2;

            if (static_cast<std::size_t>(audioData->sizeConsumed) + length > static_cast<std::size_t>(audioData->size)) {
                length = static_cast<std::size_t>(audioData->size - audioData->sizeConsumed);
            }

            if (!audioData->file.is_open()) {
                audioData->file.open(audioData->filename, std::ios::binary);
                if (!audioData->file.is_open()) {
                    std::cerr << "ERROR: Could not re-open streaming file \"" << audioData->filename << "\"" << std::endl;
                    return 0;
                }
            }

			std::vector<char> moreData;
			moreData.reserve(length);

            audioData->file.clear();
            audioData->file.seekg(audioData->sizeConsumed);
            if (!audioData->file.read(moreData.data(), static_cast<ogg_int64_t>(length))) {
                if (audioData->file.eof()) {
                    audioData->file.clear(); // just clear the error, we will resolve it later
                } else if (audioData->file.fail()) {
                    std::cerr << "ERROR: OGG stream has fail bit set " << audioData->filename << std::endl;
                    audioData->file.clear();
                    return 0;
                } else if (audioData->file.bad()) {
                    perror(("ERROR: OGG stream has bad bit set " + audioData->filename).c_str());
                    audioData->file.clear();
                    return 0;
                }
            }
            audioData->sizeConsumed += length;

            std::memcpy(destination, moreData.data(), length);

            audioData->file.clear();

            return length;
        }

        std::int32_t seek_ogg_callback(void* fileHandle, ogg_int64_t to, std::int32_t type) {
            SoundDefinition* audioData = reinterpret_cast<SoundDefinition*>(fileHandle);

            if (type == SEEK_CUR) {
                audioData->sizeConsumed += to;
            } else if (type == SEEK_END) {
                audioData->sizeConsumed = audioData->size - to;
            } else if (type == SEEK_SET) {
                audioData->sizeConsumed = to;
            } else {
                return -1; // what are you trying to do vorbis?
            }

            if (audioData->sizeConsumed < 0) {
                audioData->sizeConsumed = 0;
                return -1;
            }
            if (audioData->sizeConsumed > audioData->size) {
                audioData->sizeConsumed = audioData->size;
                return -1;
            }

            return 0;
        }

        long int tell_ogg_callback(void* fileHandle) {
            SoundDefinition* audioData = reinterpret_cast<SoundDefinition*>(fileHandle);
#if ENGINE_OS == ENGINE_OS_WIN32
            // Linux sees this as a useless cast.
            return static_cast<long>(audioData->sizeConsumed);
#else
            return audioData->sizeConsumed;
#endif
        }

        bool create_stream_from_file(const std::string& filename, SoundDefinition* audioData) {
            std::cout << "Trying to open file: " << filename << std::endl;
            audioData->filename = filename;
            audioData->file.open(filename, std::ios::binary);
            if (!audioData->file.is_open()) {
                std::cerr << "ERROR: couldn't open file" << std::endl;
                return 0;
            } else {
                std::cout << "Succesfully opened file" << std::endl;
            }

            audioData->file.seekg(0, std::ios_base::beg);
            audioData->file.ignore(std::numeric_limits<std::streamsize>::max());
            audioData->size = audioData->file.gcount();
            audioData->file.clear();
            audioData->file.seekg(0, std::ios_base::beg);
            audioData->sizeConsumed = 0;

            ov_callbacks oggCallbacks;
            oggCallbacks.read_func = read_ogg_callback;
            oggCallbacks.close_func = nullptr;
            oggCallbacks.seek_func = seek_ogg_callback;
            oggCallbacks.tell_func = tell_ogg_callback;

            if (ov_open_callbacks(reinterpret_cast<void*>(audioData), &audioData->oggVorbisFile, nullptr, -1, oggCallbacks) < 0) {
                std::cerr << "ERROR: Could not ov_open_callbacks" << std::endl;
                return false;
            }

            vorbis_info* vorbisInfo = ov_info(&audioData->oggVorbisFile, -1);

            audioData->channels = vorbisInfo->channels;
            audioData->bitsPerSample = 16;
            audioData->sampleRate = static_cast<int>(vorbisInfo->rate);
            audioData->duration = ov_time_total(&audioData->oggVorbisFile, -1);

            alCall(alGenSources, 1, &audioData->source);
            alCall(alSourcef, audioData->source, AL_PITCH, 1.f);
            alCall(alSourcef, audioData->source, AL_GAIN, static_cast<ALfloat>(AL_MAX_GAIN));
            alCall(alSource3f, audioData->source, AL_POSITION, 0.f, 0.f, 0.f);
            alCall(alSource3f, audioData->source, AL_VELOCITY, 0.f, 0.f, 0.f);
            alCall(alSourcei, audioData->source, AL_LOOPING, AL_FALSE);

            alGenBuffers(SoundDefinition::NUM_BUFFERS, &audioData->buffers[0]);
			alCheckErrors();

            if (audioData->file.eof()) {
                std::cerr << "ERROR: Already reached EOF without loading data" << std::endl;
                return false;
            } else if (audioData->file.fail()) {
                std::cerr << "ERROR: Fail bit set" << std::endl;
                return false;
            } else if (!audioData->file) {
                std::cerr << "ERROR: file is false" << std::endl;
                return false;
            }

            char* data = new char[SoundDefinition::BUFFER_SIZE];

            for (std::uint8_t i = 0; i < SoundDefinition::NUM_BUFFERS; ++i) {
                long dataSoFar = 0;
                while (dataSoFar < SoundDefinition::BUFFER_SIZE) {
                    const long result = ov_read(&audioData->oggVorbisFile, &data[dataSoFar], SoundDefinition::BUFFER_SIZE - static_cast<int>(dataSoFar), 0, 2, 1, &audioData->oggCurrentSection);
                    if (result == OV_HOLE) {
                        std::cerr << "ERROR: OV_HOLE found in initial read of buffer " << i << std::endl;
                        break;
                    } else if (result == OV_EBADLINK) {
                        std::cerr << "ERROR: OV_EBADLINK found in initial read of buffer " << i << std::endl;
                        break;
                    } else if (result == OV_EINVAL) {
                        std::cerr << "ERROR: OV_EINVAL found in initial read of buffer " << i << std::endl;
                        break;
                    } else if (result == 0) {
                        std::cerr << "ERROR: EOF found in initial read of buffer " << i << std::endl;
                        break;
                    }

                    dataSoFar += result;
                }

                if (audioData->channels == 1 && audioData->bitsPerSample == 8)
                    audioData->format = AL_FORMAT_MONO8;
                else if (audioData->channels == 1 && audioData->bitsPerSample == 16)
                    audioData->format = AL_FORMAT_MONO16;
                else if (audioData->channels == 2 && audioData->bitsPerSample == 8)
                    audioData->format = AL_FORMAT_STEREO8;
                else if (audioData->channels == 2 && audioData->bitsPerSample == 16)
                    audioData->format = AL_FORMAT_STEREO16;
                else {
                    std::cerr << "ERROR: unrecognised ogg format: " << audioData->channels << " channels, " << audioData->bitsPerSample << " bps" << std::endl;
                    delete[] data;
                    return false;
                }

                alCall(alBufferData, audioData->buffers[i], audioData->format, data, static_cast<int>(dataSoFar), audioData->sampleRate);
            }

            alSourceQueueBuffers(audioData->source, static_cast<ALsizei>(SoundDefinition::NUM_BUFFERS), &audioData->buffers[0]);
			alCheckErrors();

            delete[] data;

            return true;
        }

        void play_stream(SoundDefinition* audioData) {
            alCall(alSourceStop, audioData->source);
            alCall(alSourcePlay, audioData->source);
        }

        void update_stream(SoundDefinition* audioData) {
            ALint buffersProcessed = 0;
            alCall(alGetSourcei, audioData->source, AL_BUFFERS_PROCESSED, &buffersProcessed);
            if (buffersProcessed <= 0) {
                return;
            }

            while (buffersProcessed--) {
                ALuint buffer;
                alCall(alSourceUnqueueBuffers, audioData->source, 1, &buffer);

                std::vector<char> data;
                data.resize(SoundDefinition::BUFFER_SIZE);
                data.erase(data.begin(), data.end());

                long dataSizeToBuffer = 0;
                long sizeRead = 0;

                while (sizeRead < SoundDefinition::BUFFER_SIZE) {
                    const long result = ov_read(&audioData->oggVorbisFile, &data.data()[sizeRead], SoundDefinition::BUFFER_SIZE - static_cast<int>(sizeRead), 0, 2, 1, &audioData->oggCurrentSection);
                    if (result == OV_HOLE) {
                        std::cerr << "ERROR: OV_HOLE found in update of buffer " << std::endl;
                        break;
                    } else if (result == OV_EBADLINK) {
                        std::cerr << "ERROR: OV_EBADLINK found in update of buffer " << std::endl;
                        break;
                    } else if (result == OV_EINVAL) {
                        std::cerr << "ERROR: OV_EINVAL found in update of buffer " << std::endl;
                        break;
                    } else if (result == 0) {
                        std::int32_t seekResult = ov_raw_seek(&audioData->oggVorbisFile, 0);
                        if (seekResult == OV_ENOSEEK)
                            std::cerr << "ERROR: OV_ENOSEEK found when trying to loop" << std::endl;
                        else if (seekResult == OV_EINVAL)
                            std::cerr << "ERROR: OV_EINVAL found when trying to loop" << std::endl;
                        else if (seekResult == OV_EREAD)
                            std::cerr << "ERROR: OV_EREAD found when trying to loop" << std::endl;
                        else if (seekResult == OV_EFAULT)
                            std::cerr << "ERROR: OV_EFAULT found when trying to loop" << std::endl;
                        else if (seekResult == OV_EOF)
                            std::cerr << "ERROR: OV_EOF found when trying to loop" << std::endl;
                        else if (seekResult == OV_EBADLINK)
                            std::cerr << "ERROR: OV_EBADLINK found when trying to loop" << std::endl;

                        if (seekResult != 0) {
                            std::cerr << "ERROR: Unknown error in ov_raw_seek" << std::endl;
                            return;
                        }
                    }
                    sizeRead += result;
                }
                dataSizeToBuffer = sizeRead;

                if (dataSizeToBuffer > 0) {
                    alCall(alBufferData, buffer, audioData->format, data.data(), static_cast<int>(dataSizeToBuffer), audioData->sampleRate);
                    alCall(alSourceQueueBuffers, audioData->source, 1, &buffer);
                }

                if (dataSizeToBuffer < SoundDefinition::BUFFER_SIZE) {
                    std::cout << "Data missing" << std::endl;
                }

                alCall(alGetSourcei, audioData->source, AL_SOURCE_STATE, &audioData->state);

                if (audioData->state != AL_PLAYING) {
                    alCall(alSourceStop, audioData->source);
                    alCall(alSourcePlay, audioData->source);
                }
            }
        }
    } // End Namespace

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
            device = alcOpenDevice(std::to_string(desired_device).c_str());
            if (!device) {
                std::cerr << "Failed to open " << desired_device << ", trying default" << std::endl;
            }
        }
        if (!device) {
            const char* s = alcGetString(device, ALC_DEFAULT_DEVICE_SPECIFIER);
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

#ifdef _WIN32
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
        cur_time = static_cast<int>(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#else /* _POSIX_TIMERS > 0 */
        struct timeval tv;
        int ret = gettimeofday(&tv, NULL);
        if (ret != 0) return 0;
        cur_time = static_cast<int>(tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif

        if (!start_time)
            start_time = cur_time;
        return cur_time - start_time;
    }

    void al_nssleep(unsigned long nsec) {
        struct timespec ts, rem;
        ts.tv_sec = static_cast<time_t>(nsec / 1000000000ul);
        ts.tv_nsec = static_cast<long>(nsec % 1000000000ul);
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
        if (sfinfo.frames < 1 || sfinfo.frames > static_cast<sf_count_t>(INT_MAX / sizeof(short)) / sfinfo.channels) {
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
        membuf = static_cast<short*>( malloc( static_cast<size_t>(sfinfo.frames * sfinfo.channels) * sizeof(short)) );

        num_frames = sf_readf_short(sndfile, membuf, sfinfo.frames);
        if (num_frames < 1) {
            free(membuf);
            sf_close(sndfile);
            std::cerr << "Failed to read samples in " << filename << " (%" << num_frames << ")" << std::endl;
            return 0;
        }
        num_bytes = static_cast<ALsizei>( static_cast<size_t>(num_frames * sfinfo.channels) * sizeof(short) );

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
        alSourcei(source, AL_BUFFER, static_cast<ALint>(buffer));
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
        if ((alGetError() == AL_NO_ERROR && thisSound.second->state == AL_PLAYING) || thisSound.second->is_ogg) {
            if (thisSound.second->is_ogg) {
                openal::ogg::update_stream(thisSound.second.get());
            } else {
                openal::al_nssleep(10000000);
                alGetSourcei(thisSound.second->source, AL_SOURCE_STATE, &thisSound.second->state);
                /* Get the source offset. */
                //alGetSourcef(thisSound.second->source, AL_SEC_OFFSET, &thisSound.second->offset);
            }
            alGetSourcef(thisSound.second->source, AL_SEC_OFFSET, &thisSound.second->offset);
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

void AudioEngine::LoadSound(const std::string& strSoundName, bool bLooping) {
    this->sounds.emplace(strSoundName, std::make_unique<SoundDefinition>()); // Create default SoundDefinition
    auto* sound = this->sounds.at(strSoundName).get();

#if ENGINE_ENABLE_AUDIO
    sound->buffer = openal::LoadSound(strSoundName);
    alGetError(); // Clear Error

    if (!sound->buffer) {
        if (!openal::ogg::create_stream_from_file(strSoundName, sound)) {
            // Try it with OGG
            sound->err_msg = "Failed to initialize OpenAL";
            return;
        } else {
            std::cout << "Loaded OGG File" << std::endl;
            sound->is_ogg = true;
        }
    }

    // OGG Does it's own thing.
    if (sound->err_msg == "" && !sound->is_ogg) {
        if (bLooping) {
            // Defaults to false
            alSourcei(sound->source, AL_LOOPING, AL_TRUE);
        }

        alGenSources(1, &sound->source);
        alSourcei(sound->source, AL_BUFFER, static_cast<ALint>(sound->buffer));
    }

    if (alGetError() != AL_NO_ERROR) {
        sound->err_msg = "Failed to setup sound source";
    }
#endif
}

void AudioEngine::Play(const std::string& strSoundName) {
    auto* thisSound = this->sounds.at(strSoundName).get();
#if ENGINE_ENABLE_AUDIO
        alSourceStop(thisSound->source);
        alSourcePlay(thisSound->source);
#endif
}

void AudioEngine::UnLoadSound(const std::string& strSoundName) {
    auto& thisSound = this->sounds.at(strSoundName);

#if ENGINE_ENABLE_AUDIO
    /* All done. Delete resources */
    alDeleteSources(1, &thisSound->source);
    alDeleteBuffers(1, &thisSound->buffer);
#endif
    this->sounds.erase(strSoundName);
}

bool AudioEngine::IsPlaying(const std::string& strSoundName) const {
#if ENGINE_ENABLE_AUDIO
    return this->isLoaded(strSoundName) && this->sounds.at(strSoundName)->state == AL_PLAYING;
#else
    return false;
#endif
}

bool AudioEngine::isLoaded(const std::string& soundName) const {
    return this->sounds.find(soundName) != this->sounds.end();
}
