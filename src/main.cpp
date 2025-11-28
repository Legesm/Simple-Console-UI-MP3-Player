#include <player.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <iostream>
#include <fstream>

static std::ofstream logFile("player.log", std::ios::app);

#define LOG(x) do { logFile << x << std::endl; logFile.flush(); } while(0)

using namespace ftxui;

int main() {
    auto screen = ScreenInteractive::Fullscreen();
    MP3Player player;
    player.setFilesPath("C:/C++/Project/PlayerTest1/cmake-build-debug/music");
    player.loadFiles();

    const auto& files = player.getFilesArray();
    if (files.empty()) {
        LOG("No music files found!");
        return 0;
    }

    int sliderVolume = 50;

    auto left_panel = Radiobox({files, &player.selectedIndex}) |
                      border | flex | size(WIDTH, GREATER_THAN, 20);

    auto right_top_panel = Renderer([&] {
        const std::string& track_name = files[player.selectedIndex];
        return window(text("Top Panel"), text("Track: (" + track_name + ")"));
    });

    auto btn_play = Button("Play", [&] {
        if (!player.getStatus()) {
            player.changeTrack();
        } else if (player.getPaused()) {
            player.resume();
        }
    });

    auto btn_pause = Button("Pause", [&] {
        if (!player.getPaused()) player.pause();
    });

    auto btn_prev = Button("Prev", [&] {
        if (player.selectedIndex > 0) {
            player.selectedIndex--;
            player.changeTrack();
        }
    });

    auto btn_next = Button("Next", [&] {
        if (player.selectedIndex + 1 < files.size()) {
            player.selectedIndex++;
            player.changeTrack();
        }
    });

    auto volume_slider = Slider("Volume", &sliderVolume, 0, 100, 5);

    auto buttons = Container::Horizontal({btn_prev, btn_play, btn_pause, btn_next, volume_slider});

    auto right_bot_panel = Renderer(buttons, [&] {
        return window(text("Bottom Panel"),
                      vbox({hbox({btn_prev->Render(), filler(), btn_play->Render(),
                                  filler(), btn_pause->Render(), filler(), btn_next->Render()}),
                            separator(),
                            volume_slider->Render()}));
    });

    int right_split_pos = 10;
    int left_width = 20;

    auto right_split = ResizableSplitBottom(right_bot_panel, right_top_panel, &right_split_pos);
    auto root = ResizableSplitRight(left_panel, right_split, &left_width) | size(WIDTH, GREATER_THAN, 20);

    auto ui = Renderer(root, [&] {
        player.setVolume(sliderVolume / 100.0f);
        return root->Render();
    });

    screen.Loop(ui);
    std::cin.get();
}
