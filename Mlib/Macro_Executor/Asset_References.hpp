#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

struct ReplacementParameter;
class MacroLineExecutor;
class AssetGroupReplacementParameters;

class AssetReferences: public DanglingBaseClass {
    AssetReferences(const AssetReferences&) = delete;
    AssetReferences& operator = (const AssetReferences&) = delete;

public:
    AssetReferences();
    ~AssetReferences();

    bool contains(const std::string& group) const;
    void add(const std::string& group);

    const AssetGroupReplacementParameters& operator [] (const std::string& group) const;
    AssetGroupReplacementParameters& operator [] (const std::string& group);

private:
    std::map<std::string, AssetGroupReplacementParameters> replacement_parameters_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
};

}
