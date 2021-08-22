#pragma once
#include <iosfwd>
#include <string>
#include <vector>

namespace Mlib::Sfm {

class ImagePipeline;

void process_files_with_pipeline(
    const std::string& cache_dir,
    const std::vector<std::string>& image_files,
    const std::vector<std::string>* camera_files,
    ImagePipeline& pipeline,
    std::ostream& ostream,
    size_t nskipped,
    size_t nimages,
    size_t ncameras,
    bool reverse);

void process_folder_with_pipeline(
    const std::string& cache_dir,
    const std::string& image_folder,
    const std::string* camera_folder,
    ImagePipeline& pipeline,
    std::ostream& ostream,
    size_t nskipped,
    size_t nimages,
    size_t ncameras,
    bool reverse);

}
