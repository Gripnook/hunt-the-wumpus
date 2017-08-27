#include "console_game.h"

#include <iostream>
#include <sstream>

namespace wumpus {

Console_game::Console_game() : game{std::cout}
{
}

void Console_game::play()
{
    print_game_info();
    game.init_hunt();
    while (!game.is_hunt_over())
        play_round_of_hunt();
    game.end_hunt();
}

void Console_game::print_game_info()
{
    std::cout << Game::game_info();
}

void Console_game::play_round_of_hunt()
{
    game.inform_player_of_hazards();
    Action action = get_action();
    switch (action.type)
    {
    case Action::Action_type::move:
        game.move(action.targets[0]);
        break;
    case Action::Action_type::shoot:
        game.shoot(action.targets);
        break;
    case Action::Action_type::quit:
        game.quit();
        break;
    default:
        throw std::logic_error("Invalid player action");
    }
}

Action Console_game::get_action()
{
    Action action;
    while (!is_valid_action(action))
    {
        print_prompt();
        std::string input;
        std::getline(std::cin, input);
        std::istringstream in(input);
        char type = '\0';
        in >> type;
        if (type == 'd')
        {
            print_debug();
        }
        else
        {
            action.type = get_action_type(type);
            for (int i = 0; i < arrow_range; ++i)
            {
                in >> action.targets[i];
                if (!in)
                    action.targets[i] = 0;
                in.get();
            }
        }
    }
    return action;
}

bool Console_game::is_valid_action(const Action& action) const
{
    switch (action.type)
    {
    case Action::Action_type::move:
        return game.can_move(action.targets[0]);
    case Action::Action_type::shoot:
        return game.can_shoot(action.targets);
    case Action::Action_type::quit:
        return true;
    default:
        return false;
    }
}

Action::Action_type Console_game::get_action_type(char type) const
{
    switch (type)
    {
    case 'm':
        return Action::Action_type::move;
    case 's':
        return Action::Action_type::shoot;
    case 'q':
        return Action::Action_type::quit;
    default:
        return Action::Action_type::none;
    }
}

void Console_game::print_prompt()
{
    auto player_room = game.get_player_room();
    auto arrows = game.get_arrows();

    std::cout << "You are in room " << player_room->number << "; "
              << "There are tunnels to rooms "
              << player_room->adjacent_rooms[0]->number;
    for (int i = 1; i < connections_per_room - 1; ++i)
        std::cout << ", " << player_room->adjacent_rooms[i]->number;
    std::cout << ", and "
              << player_room->adjacent_rooms[connections_per_room - 1]->number
              << ";" << std::endl
              << "You have " << arrows
              << " arrows remaining; move (m), shoot (s), or quit (q)?"
              << std::endl;
}

void Console_game::print_debug()
{
    for (const Room& room : game.get_rooms())
    {
        std::cout << room.number << ":" << room.adjacent_rooms[0]->number;
        for (int i = 1; i < connections_per_room; ++i)
            std::cout << "," << room.adjacent_rooms[i]->number;
        if (game.get_player_room()->number == room.number)
            std::cout << " player";
        if (room.wumpus)
            std::cout << " wumpus";
        if (room.bat)
            std::cout << " bat";
        if (room.pit)
            std::cout << " pit";
        std::cout << std::endl;
    }
}
}
