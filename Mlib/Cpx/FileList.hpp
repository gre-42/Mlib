#pragma once
#include "dom.hpp"
#include <boost/filesystem.hpp>
#include <ostream>


namespace Mlib::cpx {

template <class TEntry>
std::list<TEntry> list_dir(
    const boost::filesystem::path& path,
    const std::function<TEntry(const boost::filesystem::path& path)>& callback)
{
    return applied(path, callback);
}

}
