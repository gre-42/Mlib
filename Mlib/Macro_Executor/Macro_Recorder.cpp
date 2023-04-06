#include "Macro_Recorder.hpp"
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Macro_Manifest.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;
using namespace Mlib;

void MacroRecorder::operator()(const MacroLineExecutor& macro_line_executor)
{
    auto manifest = MacroManifest::from_json(macro_line_executor.script_filename_);
    auto ifs_p = create_ifstream(manifest.script_file);
    auto& ifs = *ifs_p;
    if (ifs.fail()) {
        THROW_OR_ABORT("Could not open script file \"" + manifest.script_file + '"');
    }

    if (manifest.script_file.ends_with(".scn.json")) {
        if (macro_line_executor.verbose_) {
            linfo() << "Processing JSON scene file \"" << manifest.script_file << '"';
        }
        json j;
        ifs >> j;
        if (!ifs.eof() && ifs.fail()) {
            THROW_OR_ABORT("Error reading from file: \"" + macro_line_executor.script_filename_ + '"');
        }
        if (j.type() == nlohmann::detail::value_t::array) {
            for (const auto& e : j) {
                if (e.contains("declare_macro")) {
                    auto name = e.at("declare_macro").get<std::string>();
                    if (macro_line_executor.verbose_) {
                        linfo() << "Storing macro \"" << name << '"';
                    }
                    if (!json_macros_.try_emplace(
                        name,
                        JsonMacro{
                            .filename = manifest.script_file,
                            .content = e.at("content")
                        }).second)
                    {
                        THROW_OR_ABORT("Macro with name \"" + name + "\" already exists");
                    }
                } else {
                    JsonMacroArguments args;
                    args.insert_json(manifest.json_variables);
                    macro_line_executor(e, &manifest.text_variables, &args);
                }
            }
        } else {
            THROW_OR_ABORT("Unexpected element in file: \"" + macro_line_executor.script_filename_ + '"');
        }
    } else if (manifest.script_file.ends_with(".scn")) {
        static const DECLARE_REGEX(macro_begin_reg, "^\\s*macro_begin ([\\w+-.]+)$");
        static const DECLARE_REGEX(macro_end_reg, "^\\s*macro_end$");
        static const DECLARE_REGEX(alias_reg, "^\\s*global ([\\w+-.]+)\\s*=\\s*([\\w+-.]*)$");

        std::string line;
        std::list<std::pair<std::string, TextMacro>> recording_macros;
        while (std::getline(ifs, line, ';')) {
            Mlib::re::smatch match;
            if (Mlib::re::regex_match(line, match, macro_begin_reg)) {
                // linfo() << macro_line_executor.global_substitutions_;
                if (macro_line_executor.verbose_) {
                    linfo() << "Recording macro \"" << match[1].str() << '"';
                }
                recording_macros.emplace_back(match[1].str(), TextMacro{.filename = manifest.script_file});
            } else if (Mlib::re::regex_match(line, match, macro_end_reg)) {
                if (macro_line_executor.verbose_) {
                    linfo() << "Finishing macro \"" << recording_macros.back().first << '"';
                }
                if (recording_macros.empty()) {
                    THROW_OR_ABORT("Macro-end despite no active macro");
                }
                auto it = text_macros_.insert(std::make_pair(recording_macros.back().first, recording_macros.back().second));
                if (!it.second) {
                    THROW_OR_ABORT("Duplicate macro: " + recording_macros.back().first);
                }
                recording_macros.pop_back();
            } else if (!recording_macros.empty()) {
                if (macro_line_executor.verbose_) {
                    linfo() << "Adding line to macro \"" << recording_macros.back().first << '"';
                }
                recording_macros.back().second.lines.push_back(line);
            } else if (Mlib::re::regex_match(line, match, alias_reg)) {
                if (!globals_.insert(match[1].str(), match[2].str())) {
                    THROW_OR_ABORT("Global variable \"" + match[1].str() + "\" already exists");
                }
            } else {
                macro_line_executor(line, &manifest.text_variables);
            }
        }
        if (!ifs.eof() && ifs.fail()) {
            THROW_OR_ABORT("Error reading from file: \"" + macro_line_executor.script_filename_ + '"');
        }
        if (!recording_macros.empty()) {
            THROW_OR_ABORT("Missing macro_end; in file \"" + macro_line_executor.script_filename_ + '"');
        }
    } else {
        THROW_OR_ABORT("Unknown script file extension: \"" + macro_line_executor.script_filename_ + '"');
    }
}
