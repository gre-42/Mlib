#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Audio/Audio_Buffer.hpp>
#include <Mlib/Audio/Audio_Context.hpp>
#include <Mlib/Audio/Audio_Device.hpp>
#include <Mlib/Audio/Audio_Source.hpp>
#include <Mlib/Audio/List_Audio_Devices.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: audio_loop filename [--loop] [--pitch <value>] [--gain <value>] [--audio_frequency <value>]",
        {"--loop"},
        {"--pitch", "--gain", "--audio_frequency"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed(1);

        list_audio_devices(linfo(LogFlags::NO_APPEND_NEWLINE).ref());
        AudioDevice device;
        AudioContext context{ device, safe_stou(args.named_value("--audio_frequency", "0")) };
        auto buffer = AudioBuffer::from_wave(args.unnamed_value(0));
        AudioSource source{buffer, PositionRequirement::POSITION_NOT_REQUIRED};
        source.set_loop(args.has_named("--loop"));
        source.set_pitch(safe_stof(args.named_value("--pitch", "1")));
        source.set_gain(safe_stof(args.named_value("--gain", "1")));
        source.play();
        source.join();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
