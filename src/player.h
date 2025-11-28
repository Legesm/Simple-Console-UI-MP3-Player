#pragma once
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <filesystem>
#include <portaudio.h>

#include "minimp3.h"
#include "minimp3_ex.h"


namespace fs = std::filesystem;

class MP3Player {
private:

    mp3dec_t mp3d{};
    mp3dec_file_info_t info{};

    PaStream* stream = nullptr;
    std::thread playThread;
    std::atomic<bool> isPlaying = false;
    std::atomic<bool> isPaused = false;
    std::atomic<float> volume = 1.0f;

    std::vector<std::string> files;
    std::string filesPath;

    std::atomic<int> currentIndex{-1};

    void play();
    void stopAndJoin();

    size_t playPos = 0;
    size_t totalFrames = 0;

    static int paCallback(
        const void* inputBuffer,
        void* outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData
    );



public:
    int selectedIndex = 0;

    MP3Player();
    ~MP3Player();

    void playAsync();
    void stopAsync();
    void changeTrack();

    void loadFiles();
    bool load(const std::string& filename);

    int getCurrentIndex();
    void setCurrentIndex(int index);

    int getSelectedIndex();
    void setSelectedIndex(int index);

    bool getStatus();
    std::atomic<bool>& getPaused();

    void pause();
    void resume();
    void setVolume(float vol);
    float getVolume();

    const std::vector<std::string>& getFilesArray() const;
    void setFilesPath(const std::string& path);
};
