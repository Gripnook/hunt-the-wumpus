#pragma once

#include "game.h"

namespace wumpus {

struct Action
{
    enum class Action_type
    {
        none,
        move,
        shoot,
        quit
    } type{Action_type::none};
    std::array<int, arrow_range> targets;
};

class Console_game
{
public:
    static std::string game_info();

    Console_game();

    void play();

private:
    Game game;

    void print_game_info();
    void play_round_of_hunt();
    Action get_action();
    bool is_valid_action(const Action& action) const;
    Action::Action_type get_action_type(char type) const;
    void print_prompt();
    void print_debug();
};
}
