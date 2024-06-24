#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Audio/Audio_Buffer.hpp>
#include <Mlib/Audio/Audio_Buffer_Sequence.hpp>
#include <Mlib/Audio/Audio_Context.hpp>
#include <Mlib/Audio/Audio_Device.hpp>
#include <Mlib/Audio/Audio_File_Sequence.hpp>
#include <Mlib/Audio/Cross_Fade.hpp>
#include <Mlib/Audio/List_Audio_Devices.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Threads/Realtime_Threads.hpp>
#include <Mlib/Time/Sleep.hpp>
#include <chrono>
#include <iostream>
#include <thread>

using namespace Mlib;

int main(int argc, char** argv) {
    reserve_realtime_threads(0);
    const ArgParser parser(
        "Usage: audio_sequence filename.json "
        "--f0 <Hz> "
        "--f1 <Hz> "
        "[--dgain <value>] "
        "[--dt_fade <s>] "
        "[--dt_append <s>] "
        "[--niter <value>] "
        "[--gain <value>] "
        "[--pitch <value>] "
        "[--pitch_adjustment {rounding,up_sampling,down_sampling}] "
        "[--audio_frequency <value>]"
        "[--verbose]",
        {"--verbose"},
        {"--dgain",
         "--dt_fade",
         "--dt_append",
         "--gain",
         "--pitch",
         "--f0",
         "--f1",
         "--niter",
         "--pitch_adjustment",
         "--audio_frequency"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed(1);

        list_audio_devices();
        AudioDevice device;
        AudioContext context{device, safe_stou(args.named_value("--audio_frequency", "0"))};
        std::string meta_filename = args.unnamed_value(0);

        auto items = load_audio_file_sequence(meta_filename);
        std::list<AudioBufferAndFrequency> buffers;
        for (const auto& i : items) {
            buffers.push_back(AudioBufferAndFrequency{
                .buffer = std::make_shared<AudioBuffer>(AudioBuffer::from_wave(i.filename)),
                .frequency = i.frequency});
        }
        AudioBufferSequence buffer_seq{std::vector(buffers.begin(), buffers.end())};
        float dgain = safe_stof(args.named_value("--dgain", "0.02"));
        float dt_fade = safe_stof(args.named_value("--dt_fade", "0.01"));
        float dt_append = safe_stof(args.named_value("--dt_append", "0.1"));
        float pitch = safe_stof(args.named_value("--pitch", "1"));
        float gain_factor = safe_stof(args.named_value("--gain", "1"));
        auto paused = [](){return false;};
        CrossFade cross_fade{ PositionRequirement::POSITION_NOT_REQUIRED, paused, dgain, dt_fade };
        for (float f : Linspace<float>{
            safe_stof(args.named_value("--f0")),
            safe_stof(args.named_value("--f1")),
            safe_stoz(args.named_value("--niter", "100"))})
        {
            auto& bf = buffer_seq.get_buffer_and_frequency(
                f,
                pitch_adjustment_strategy_from_string(
                    args.named_value("--pitch_adjustment", "up_sampling")));
            if (args.has_named("--verbose")) {
                linfo() << "Requested frequency: " << f << "Hz. Template frequency: " << bf.frequency;
            }
            cross_fade.play(*bf.buffer, gain_factor, f * pitch, bf.frequency);
            Mlib::sleep_for(std::chrono::duration<float>(dt_append));
        }
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
