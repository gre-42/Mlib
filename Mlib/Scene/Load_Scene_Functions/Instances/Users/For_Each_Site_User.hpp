#pragma once
namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;
enum class UserType;

class ForEachSiteUser {
public:
    ForEachSiteUser(UserType user_type);
    ~ForEachSiteUser();
    void execute(const LoadSceneJsonUserFunctionArgs& args);
private:
    UserType user_type_;
};

}
