#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Audio/Audio_Buffer.hpp>
#include <Mlib/Audio/Audio_Context.hpp>
#include <Mlib/Audio/Audio_Device.hpp>
#include <Mlib/Audio/Cross_Fade.hpp>
#include <Mlib/Audio/List_Audio_Devices.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <chrono>
#include <iostream>
#include <thread>

using namespace Mlib;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: audio_cross_fade filename ... [--dgain <value>] [--dt_fade <ms>] [--dt_append <ms>] [--gain <value>] [--pitch <value>]",
        {},
        {"--dgain", "--dt_fade", "--dt_append", "--gain", "--pitch"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unamed_atleast(1);

        list_audio_devices();
        AudioDevice device;
        AudioContext context{ device };
        std::list<AudioBuffer> buffers;
        for (const auto& l : args.unnamed_values()) {
            buffers.emplace_back();
            buffers.back().load_wave(l);
        }
        float dgain = safe_stof(args.named_value("--dgain"));
        float dt_fade = safe_stof(args.named_value("--dt_fade"));
        float dt_append = safe_stof(args.named_value("--dt_append"));
        float pitch = safe_stof(args.named_value("--pitch", "1"));
        float gain_factor = safe_stof(args.named_value("--gain", "1"));
        std::atomic_bool paused = false;
        CrossFade cross_fade{ paused, dgain, dt_fade };
        for (const auto& b : buffers) {
            cross_fade.play(b, gain_factor, pitch);
            std::this_thread::sleep_for(std::chrono::duration<float>(dt_append));
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
