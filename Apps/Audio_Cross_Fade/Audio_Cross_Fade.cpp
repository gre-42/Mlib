#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Audio/Audio_Buffer.hpp>
#include <Mlib/Audio/Audio_Context.hpp>
#include <Mlib/Audio/Audio_Device.hpp>
#include <Mlib/Audio/Cross_Fade.hpp>
#include <Mlib/Audio/List_Audio_Devices.hpp>
#include <Mlib/Memory/Event_Emitter.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Time/Sleep.hpp>
#include <chrono>
#include <thread>

using namespace Mlib;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: audio_cross_fade filename ... [--dgain <value>] [--dt_fade <ms>] [--dt_append <ms>] [--gain <value>] [--pitch <value>] [--audio_frequency <value>]",
        {},
        {"--dgain", "--dt_fade", "--dt_append", "--gain", "--pitch", "--audio_frequency"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed_atleast(1);

        list_audio_devices(linfo(LogFlags::NO_APPEND_NEWLINE).ref());
        AudioDevice device;
        AudioContext context{device, safe_stou(args.named_value("--audio_frequency", "0"))};
        std::list<AudioBuffer> buffers;
        for (const auto& l : args.unnamed_values()) {
            buffers.emplace_back(AudioBuffer::from_wave(l));
        }
        float dgain = safe_stof(args.named_value("--dgain"));
        float dt_fade = safe_stof(args.named_value("--dt_fade"));
        float dt_append = safe_stof(args.named_value("--dt_append"));
        float pitch = safe_stof(args.named_value("--pitch", "1"));
        float gain_factor = safe_stof(args.named_value("--gain", "1"));
        auto paused = [](){return false;};
        EventEmitter paused_changed{[](){return false;}};
        CrossFade cross_fade{ PositionRequirement::POSITION_NOT_REQUIRED, paused, paused_changed, dgain };
        cross_fade.start_background_thread(dt_fade);
        for (const auto& b : buffers) {
            cross_fade.play(b, gain_factor, pitch);
            Mlib::sleep_for(std::chrono::duration<float>(dt_append));
        }
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
