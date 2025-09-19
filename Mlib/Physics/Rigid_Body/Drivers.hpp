#pragma once
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Ref.hpp>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace Mlib {

class IPlayer;

class Drivers {
public:
    using PlayersMap = std::map<std::string, DestructionFunctionsTokensRef<IPlayer>>;

    Drivers();
    ~Drivers();
    void set_seats(std::vector<std::string> seats);
    bool seat_exists(const std::string& seat) const;
    bool seat_is_free(const std::string& seat);
    void add(std::string seat, DanglingBaseClassRef<IPlayer> player, SourceLocation loc);
    DanglingBaseClassPtr<IPlayer> try_get(const std::string& seat) const;
    void clear();
    const std::string* first_free_seat() const;
    const std::string* next_free_seat(const std::string& current_seat) const;
    const PlayersMap& players_map() const;
private:
    std::vector<std::string> seats_;
    std::set<std::string> seats_set_;
    PlayersMap players_;
};

}
