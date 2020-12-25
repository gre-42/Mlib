#include "Folder_Data_Generator.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Frames/Image_Frame.hpp>
#include <Mlib/Sfm/Pipelines/ImagePipeline.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

using namespace Mlib;
using namespace Mlib::Sfm;

void sort_directory_entries_by_filename(std::vector<fs::directory_entry>& entries) {
    std::sort(
        entries.begin(),
        entries.end(),
        [](
            const fs::directory_entry& left,
            const fs::directory_entry& right) -> bool {
        return left.path().filename() < right.path().filename();
    });
}

std::vector<std::string> get_sorted_files(const std::string& folder) {
    std::vector<fs::directory_entry> entries;
    for (const auto& entry : fs::directory_iterator(folder)) {
        if (fs::is_regular_file(entry)) {
            entries.push_back(entry);
        }
    }
    sort_directory_entries_by_filename(entries);
    std::vector<std::string> files;
    for (const auto& entry : entries) {
        files.push_back(entry.path().string());
    }
    return files;
}

void Mlib::Sfm::process_files_with_pipeline(
    const std::string& cache_dir,
    const std::vector<std::string>& image_files,
    const std::vector<std::string>* camera_files,
    ImagePipeline& pipeline,
    std::ostream& ostream,
    size_t nimages,
    size_t ncameras)
{
    if ((camera_files != nullptr) && (camera_files->size() != image_files.size())) {
        throw std::runtime_error("Number of images differs from number of cameras");
    }
    //try {
        std::chrono::milliseconds time{0};
        size_t i = 0;
        std::vector<std::string>::const_iterator camera_it;
        if (camera_files != nullptr) {
            camera_it = camera_files->begin();
        }
        for (const auto& image_filename : image_files) {
            if (i == nimages) {
                break;
            }
            {
                const std::string txt_filename = cache_dir + "/Input/input-" + std::to_string(time.count()) + ".txt";
                std::ofstream ofs{txt_filename};
                ofs.write(image_filename.c_str(), image_filename.length());
                ofs.flush();
                if (ofs.fail()) {
                    throw std::runtime_error("Could not write to file \"" + txt_filename + "\"");
                }
            }
            std::cout << "Loading " << i << " / " << image_files.size() << ", " << time.count() << " ms" << ": " << image_filename << std::endl;
            PpmImage raw = PpmImage::load_from_file(image_filename);
            ImageFrame image_frame;
            image_frame.grayscale = raw.to_float_grayscale();
            image_frame.rgb = raw.to_float_rgb();
            if ((camera_files == nullptr) || (i > ncameras)) {
                pipeline.process_image_frame(time, image_frame);
            } else {
                std::cout << "Loading " << i << " / " << camera_files->size() << ", " << time.count() << " ms" << ": " << *camera_it << std::endl;
                Array<float> ke = Array<float>::load_txt_2d(*camera_it);
                if (any(ke.shape() != ArrayShape{3, 4})) {
                    throw std::runtime_error("Camera matrix has incorrect dimensions");
                }
                Array<float> ike = inverted_homogeneous_3x4(ke);
                Array<float> R = R3_from_Nx4(ike, 3);
                Array<float> t = t3_from_Nx4(ike, 3);
                CameraFrame camera_frame{R, t, CameraFrame::undefined_kep};
                pipeline.process_image_frame(time, image_frame, &camera_frame);
                ++camera_it;
            }
            time += std::chrono::milliseconds{10};
            ++i;
        }
    //} catch (const std::exception& e) {
    //    try {
    //        pipeline.print_statistics(ostream);
    //    } catch (const std::exception& e1) {
    //        std::cerr << "Error in print_statistics during exception-handling: " << e1.what() << std::endl;
    //    }
    //    throw;
    //}
    pipeline.print_statistics(ostream);
}

void Mlib::Sfm::process_folder_with_pipeline(
    const std::string& cache_dir,
    const std::string& image_folder,
    const std::string* camera_folder,
    ImagePipeline& pipeline,
    std::ostream& ostream,
    size_t nimages,
    size_t ncameras)
{
    std::vector<std::string> image_files = get_sorted_files(image_folder);
    std::vector<std::string> camera_files;
    if (camera_folder != nullptr) {
        camera_files = get_sorted_files(*camera_folder);
    }
    process_files_with_pipeline(
        cache_dir,
        image_files,
        camera_folder == nullptr ? nullptr : &camera_files,
        pipeline,
        ostream,
        nimages,
        ncameras);
}
