#pragma once
#include <iosfwd>
#include <list>
#include <memory>

namespace Mlib {

enum class IoVerbosity;

namespace Dff {

class IRasterFactory;
class IPlugin;
struct Texture;
struct RasterConfig;

std::shared_ptr<Texture> read_texture_native_d3d8(
    std::istream& istr,
    const std::list<std::unique_ptr<IPlugin>>& plugins,
    const IRasterFactory* raster_factory,
    const RasterConfig* raster_config,
    IoVerbosity verbosity);

std::shared_ptr<Texture> read_texture_native_d3d9(
    std::istream& istr,
    const std::list<std::unique_ptr<IPlugin>>& plugins,
    IoVerbosity verbosity);

}}
