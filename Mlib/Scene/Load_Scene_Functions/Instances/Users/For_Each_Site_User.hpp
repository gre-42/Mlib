#pragma once
namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;
enum class UserTypes;

class ForEachSiteUser {
public:
    ForEachSiteUser(UserTypes user_type);
    ~ForEachSiteUser();
    void execute(const LoadSceneJsonUserFunctionArgs& args);
private:
    UserTypes user_types_;
};

}
