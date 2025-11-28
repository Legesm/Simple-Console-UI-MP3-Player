#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"

#define MINIMP3_EX_IMPLEMENTATION
#include "minimp3_ex.h"

#include "player.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>

static std::ofstream logFile("player.log", std::ios::app);

#define LOG(x) do { logFile << x << std::endl; logFile.flush(); } while(0)

MP3Player::MP3Player() {
    mp3dec_init(&mp3d);
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        LOG("Pa_Initialize failed: " << Pa_GetErrorText(err));
        return;
    }

}

MP3Player::~MP3Player() {
    stopAsync();
    if (info.buffer) free(info.buffer);
    Pa_Terminate();
}

void MP3Player::play() {
    if (!info.buffer || info.samples == 0) {
        LOG("No MP3 loaded!");
        return;
    }

    PaStreamParameters outputParams{};
    outputParams.device = Pa_GetDefaultOutputDevice();
    if (outputParams.device == paNoDevice) {
        LOG("No audio device found!");
        return;
    }

    outputParams.channelCount = info.channels;
    outputParams.sampleFormat = paInt16;
    outputParams.suggestedLatency = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;

    PaStream* stream;
    PaError err = Pa_OpenStream(&stream, nullptr, &outputParams, info.hz, 512, paClipOff, nullptr, nullptr);
    if (err != paNoError) {
        LOG("Pa_OpenStream error: " << Pa_GetErrorText(err));
        return;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        LOG("Pa_StartStream error: " << Pa_GetErrorText(err));
        Pa_CloseStream(stream);
        return;
    }

    size_t pos = 0;
    size_t totalFrames = info.samples / info.channels;
    std::vector<int16_t> buffer(512 * info.channels);

    while (pos < totalFrames && isPlaying) {
        // Пауза
        while (isPaused && isPlaying) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        size_t framesToWrite = std::min<size_t>(512, totalFrames - pos);

        // Применяем громкость
        for (size_t i = 0; i < framesToWrite * info.channels; ++i) {
            float sample = info.buffer[pos * info.channels + i] * volume.load();
            if (sample > 32767.f) sample = 32767.f;
            if (sample < -32768.f) sample = -32768.f;
            buffer[i] = static_cast<int16_t>(sample);
        }

        err = Pa_WriteStream(stream, buffer.data(), framesToWrite);
        if (err != paNoError) {
            LOG("Pa_WriteStream error: " << Pa_GetErrorText(err));
            break;
        }

        pos += framesToWrite;
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    stream = nullptr;
    isPlaying = false;
}

void MP3Player::stopAndJoin() {
    isPlaying = false;

    if (stream) {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        stream = nullptr;
    }

    if (playThread.joinable()) {
        playThread.join();
    }

    if (info.buffer) {
        free(info.buffer);
        info.buffer = nullptr;
        info.samples = 0;
    }
}

void MP3Player::playAsync() {
    if (isPlaying) return;
    isPlaying = true;
    playThread = std::thread([this] { this->play(); }); playThread.detach();
}
void MP3Player::stopAsync() {
    isPlaying = false;
    if (stream) {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        stream = nullptr;
    }
}

void MP3Player::loadFiles() {
    files.clear();
    for (const auto& entry : fs::directory_iterator(filesPath)) {
        if (entry.is_regular_file()) {
            auto path = entry.path();
            if (path.extension() == ".mp3") {
                files.push_back(path.filename().string());
            }
        }
    }
    std::sort(files.begin(), files.end());
}

bool MP3Player::load(const std::string& filename) {
    if (filesPath.empty()) {
        LOG("No file path specified!");
        return false;
    }

    fs::path fullPath = fs::path(filesPath) / filename;
    LOG("Trying to load MP3: " << fullPath.string());

    if (info.buffer) {
        free(info.buffer);
        info.buffer = nullptr;
        info.samples = 0;
    }

    int ret = mp3dec_load(&mp3d, fullPath.string().c_str(), &info, nullptr, nullptr);
    LOG("mp3dec_load returned: " << ret);
    LOG("info.buffer: " << info.buffer
        << ", info.samples: " << info.samples
        << ", info.channels: " << info.channels
        << ", info.hz: " << info.hz);

    if (ret != 0 || !info.buffer || info.samples == 0) {
        LOG("Failed to load MP3: " << filename);
        return false;
    }

    LOG("MP3 loaded successfully: " << filename);
    return true;
}

void MP3Player::changeTrack() {
    stopAsync();
    currentIndex = selectedIndex;

    if (!load(files[currentIndex])) {
        LOG("Load failed");
        return;
    }
    playAsync();
}

int MP3Player::getSelectedIndex() { return selectedIndex; }
void MP3Player::setSelectedIndex(int index) { selectedIndex = index; }
int MP3Player::getCurrentIndex() { return currentIndex.load(); }
void MP3Player::setCurrentIndex(int index) { currentIndex.store(index); }
bool MP3Player::getStatus() { return isPlaying.load(); }
std::atomic<bool>& MP3Player::getPaused() { return isPaused; }
void MP3Player::pause() { isPaused = true; }
void MP3Player::resume() { isPaused = false; }
void MP3Player::setVolume(float vol) { volume.store(vol); }
float MP3Player::getVolume() { return volume.load(); }
const std::vector<std::string>& MP3Player::getFilesArray() const { return files; }
void MP3Player::setFilesPath(const std::string& path) { filesPath = path; }
