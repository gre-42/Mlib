#pragma once
#include <istream>
#include <memory>

namespace Mlib {

enum class IoVerbosity;
    
namespace Dff {

struct Texture;
struct RasterConfig;
class IRasterFactory;

std::shared_ptr<Texture> read_native_texture_ps2(
    std::istream& istr,
    const IRasterFactory* raster_factory,
    const RasterConfig* raster_config,
    IoVerbosity verbosity);

}}
