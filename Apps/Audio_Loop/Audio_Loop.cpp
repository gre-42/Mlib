#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Audio/Audio_Buffer.hpp>
#include <Mlib/Audio/Audio_Context.hpp>
#include <Mlib/Audio/Audio_Device.hpp>
#include <Mlib/Audio/Audio_Source.hpp>
#include <Mlib/Audio/List_Audio_Devices.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <iostream>

using namespace Mlib;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: audio_loop filename [--loop] [--pitch <value>]",
        {"--loop"},
        {"--pitch"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed(1);

        list_audio_devices();
        AudioDevice device;
        AudioContext context{ device };
        AudioBuffer buffer;
        buffer.load_wave(args.unnamed_value(0));
        AudioSource source;
        source.attach(buffer);
        source.play();
        if (args.has_named("--loop")) {
            source.set_loop(true);
        }
        if (args.has_named_value("--pitch")) {
            source.set_pitch(safe_stof(args.named_value("--pitch")));
        }
        source.join();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
