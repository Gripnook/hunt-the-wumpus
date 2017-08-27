#include "cave.h"
#include "random.h"
#include <vector>
#include <array>
#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

namespace wumpus {

const std::string Cave::wumpus_adjacent_message = "You smell the wumpus!";
const std::string Cave::bat_adjacent_message = "You hear flapping!";
const std::string Cave::pit_adjacent_message = "You feel a breeze!";
const std::string Cave::wumpus_dead_message =
    "Congratulations, you have slain the wumpus!";
const std::string Cave::player_eaten_message =
    "You have been eaten by the wumpus!";
const std::string Cave::player_dropped_in_random_room_message =
    "You are carried away by a bat!";
const std::string Cave::player_fell_message =
    "You have fallen into a bottomless pit!";
const std::string Cave::player_shot_message =
    "You have been hit with your own arrow!";
const std::string Cave::player_quit_message = "You flee the cave!";
const std::string Cave::wumpus_moves_message =
    "You hear the sound of the wumpus moving!";

const std::array<std::array<int, Cave::connections_per_room>, Cave::num_rooms>
    Cave::room_connections{
        {{{1, 4, 5}},    {{2, 0, 7}},    {{3, 1, 9}},    {{4, 2, 11}},
         {{0, 3, 13}},   {{6, 14, 0}},   {{7, 5, 15}},   {{8, 6, 1}},
         {{9, 7, 16}},   {{10, 8, 2}},   {{11, 9, 17}},  {{12, 10, 3}},
         {{13, 11, 18}}, {{14, 12, 4}},  {{5, 13, 19}},  {{16, 19, 6}},
         {{17, 15, 8}},  {{18, 16, 10}}, {{19, 17, 12}}, {{15, 18, 14}}}};

Cave::Cave()
{
    for (int i = 0; i < num_rooms; ++i)
    {
        rooms[i] = Room(i + 1);
        for (int j = 0; j < connections_per_room; ++j)
            rooms[i].adjacent_rooms[j] = &rooms[room_connections[i][j]];
    }
}

void Cave::print_game_info() const
{
    std::cout
        << "Welcome to Hunt the Wumpus." << std::endl
        << "Your job is to slay the wumpus living in the cave using bow and "
           "arrow."
        << std::endl
        << "Each of the " << num_rooms << " rooms is connected to "
        << connections_per_room << " other rooms by dark tunnels." << std::endl
        << "In addition to the wumpus, the cave has two hazards: bottomless "
           "pits and"
        << std::endl
        << "giant bats. If you enter a room with a bottomless pit, it's the "
           "end of the"
        << std::endl
        << "game for you. If you enter a room with a bat, the bat picks you up "
           "and"
        << std::endl
        << "drops you into another room. If you enter the room with the wumpus "
           "or he"
        << std::endl
        << "enters yours, he eats you. There are " << num_pits << " pits and "
        << num_bats << " bats in the cave." << std::endl
        << "When you enter a room you will be told if a hazard is nearby:"
        << std::endl
        << "	\"" << wumpus_adjacent_message
        << "\": It's in an adjacent room." << std::endl
        << "	\"" << pit_adjacent_message
        << "\": One of the adjacent rooms is a bottomless pit." << std::endl
        << "	\"" << bat_adjacent_message
        << "\": A giant bat is in an adjacent room." << std::endl
        << "During each turn you must make a move. The possible moves are:"
        << std::endl
        << "	\"m #\": Move to an adjacent room." << std::endl
        << "	\"s #[-#...]\": Shoot an arrow through the rooms specified. The"
        << std::endl
        << "		first room number specified must be an adjacent room. The"
        << std::endl
        << "		range of an arrow is " << arrow_range
        << " rooms, and a path will be chosen at" << std::endl
        << "		random if not specified. You have " << num_arrows
        << " arrows at the start of" << std::endl
        << "		the game." << std::endl
        << "	\"q\": Quit the game and flee the cave." << std::endl
        << "Good luck!" << std::endl;
}

void Cave::hunt()
{
    init_hunt();
    while (!is_hunt_over())
        play_round_of_hunt();
    end_hunt();
}

void Cave::init_hunt()
{
    state = Game_state::none;
    arrows = num_arrows;
    reset_rooms();
    shuffle_room_numbers();
    place_player_and_hazards();
}

void Cave::reset_rooms()
{
    for (Room& room : rooms)
    {
        room.wumpus = false;
        room.bat = false;
        room.pit = false;
    }
}

void Cave::shuffle_room_numbers()
{
    for (int i = 0; i < num_rooms; ++i)
        std::swap(rooms[i].number, rooms[random(i, num_rooms)].number);
    sort_adjacent_rooms(); // This is necessary to eliminate patterns in the
                           // display of adjacent rooms.
}

void Cave::sort_adjacent_rooms()
{
    for (Room& room : rooms)
    {
        std::sort(
            std::begin(room.adjacent_rooms),
            std::end(room.adjacent_rooms),
            [](Room* first_room, Room* second_room) {
                return first_room->number < second_room->number;
            });
    }
}

void Cave::place_player_and_hazards()
{
    std::vector<int> random_locations;
    for (int i = 0; i < 2 + num_bats + num_pits; ++i)
        random_locations.push_back(random(0, num_rooms, random_locations));

    int index = 0;
    player_room = &rooms[random_locations[index++]];
    wumpus_room = &rooms[random_locations[index++]];
    ;
    wumpus_room->wumpus = true;
    while (index < 2 + num_bats)
        rooms[random_locations[index++]].bat = true;
    while (index < 2 + num_bats + num_pits)
        rooms[random_locations[index++]].pit = true;
}

bool Cave::is_hunt_over() const
{
    return state != Game_state::none;
}

void Cave::play_round_of_hunt()
{
    inform_player_of_hazards();
    Action action = get_action();
    switch (action.type)
    {
    case Action::Action_type::move:
        move(action.targets[0]);
        break;
    case Action::Action_type::shoot:
        shoot(action.targets);
        break;
    case Action::Action_type::quit:
        quit();
        break;
    default:
        throw std::logic_error("Invalid player action!");
    }
}

void Cave::inform_player_of_hazards() const
{
    std::cout << std::endl;
    bool wumpus = false, bat = false, pit = false;
    for (const Room* room : player_room->adjacent_rooms)
    {
        if (room->wumpus)
            wumpus = true;
        if (room->bat)
            bat = true;
        if (room->pit)
            pit = true;
    }
    if (wumpus)
        std::cout << wumpus_adjacent_message << std::endl;
    if (bat)
        std::cout << bat_adjacent_message << std::endl;
    if (pit)
        std::cout << pit_adjacent_message << std::endl;
}

Cave::Action Cave::get_action() const
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

Cave::Action::Action_type Cave::get_action_type(char type) const
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

bool Cave::is_valid_action(const Action& action) const
{
    switch (action.type)
    {
    case Action::Action_type::move:
        return first_room_is_adjacent(action);
    case Action::Action_type::shoot:
        if (arrows <= 0)
            return false;
        return first_room_is_adjacent(action);
    case Action::Action_type::quit:
        return true;
    default:
        return false;
    }
}

bool Cave::first_room_is_adjacent(const Action& action) const
{
    for (const Room* room : player_room->adjacent_rooms)
        if (room->number == action.targets[0])
            return true;
    return false;
}

void Cave::print_prompt() const
{
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

void Cave::move(int target)
{
    for (Room* room : player_room->adjacent_rooms)
        if (room->number == target)
            player_room = room;
    check_room_hazards();
}

void Cave::check_room_hazards()
{
    while (true)
    {
        if (player_room->wumpus)
        {
            state = Game_state::player_eaten;
            return;
        }
        if (player_room->pit)
        {
            state = Game_state::player_fell;
            return;
        }
        if (player_room->bat)
        {
            std::cout << player_dropped_in_random_room_message << std::endl;
            player_room = &rooms[random(0, num_rooms)];
            continue;
        }
        break;
    }
}

void Cave::shoot(const std::array<int, arrow_range>& targets)
{
    --arrows;
    Room* room = player_room;
    Room* previous_room = nullptr;
    for (int i = 0; i < arrow_range; ++i)
    {
        Room* next_previous_room = room;
        room = get_next_room_for_arrow_flight(previous_room, room, targets[i]);
        previous_room = next_previous_room;
        if (room->wumpus)
        {
            state = Game_state::wumpus_dead;
            return;
        }
        if (room->number == player_room->number)
        {
            state = Game_state::player_shot;
            return;
        }
    }
    move_wumpus();
}

Cave::Room* Cave::get_next_room_for_arrow_flight(
    const Room* previous_room, const Room* current_room, int target) const
{
    int previous_number = previous_room ? previous_room->number : 0;
    if (previous_number != target)
        for (Room* room : current_room->adjacent_rooms)
            if (room->number == target)
                return room;
    const std::array<Room*, connections_per_room>& candidate_rooms =
        current_room->adjacent_rooms;
    return candidate_rooms[random_if(
        0, connections_per_room, [candidate_rooms, previous_number](int x) {
            return candidate_rooms[x]->number != previous_number;
        })];
}

void Cave::move_wumpus()
{
    std::cout << wumpus_moves_message << std::endl;
    Room* new_wumpus_room =
        wumpus_room->adjacent_rooms[random(0, connections_per_room)];
    new_wumpus_room->wumpus = true;
    wumpus_room->wumpus = false;
    wumpus_room = new_wumpus_room;
    if (player_room->wumpus)
        state = Game_state::player_eaten;
}

void Cave::quit()
{
    state = Game_state::player_quit;
}

void Cave::end_hunt() const
{
    switch (state)
    {
    case Game_state::player_eaten:
        std::cout << player_eaten_message << std::endl;
        break;
    case Game_state::player_fell:
        std::cout << player_fell_message << std::endl;
        break;
    case Game_state::player_shot:
        std::cout << player_shot_message << std::endl;
        break;
    case Game_state::wumpus_dead:
        std::cout << wumpus_dead_message << std::endl;
        break;
    case Game_state::player_quit:
        std::cout << player_quit_message << std::endl;
        break;
    default:
        throw std::logic_error("Invalid end of game state");
    }
}

void Cave::print_debug() const
{
    for (const Room& room : rooms)
    {
        std::cout << room.number << ":" << room.adjacent_rooms[0]->number;
        for (int i = 1; i < connections_per_room; ++i)
            std::cout << "," << room.adjacent_rooms[i]->number;
        if (player_room->number == room.number)
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
