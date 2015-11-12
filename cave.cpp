#include "cave.h"
#include <vector>
#include <array>
#include <string>
#include <sstream>
#include <random>
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace wumpus {

const std::string Cave::game_info_file = "game_info.txt";

const std::string Cave::wumpus_adjacent_message = "You smell the wumpus!";
const std::string Cave::bat_adjacent_message = "You hear flapping!";
const std::string Cave::pit_adjacent_message = "You feel a breeze!";
const std::string Cave::wumpus_dead_message = "Congratulations, you have slain the wumpus!";
const std::string Cave::player_eaten_message = "You have been eaten by the wumpus!";
const std::string Cave::player_dropped_in_random_room_message = "You are carried away by a bat!";
const std::string Cave::player_fell_message = "You have fallen into a bottomless pit!";
const std::string Cave::player_quit_message = "You flee the cave!";
const std::string Cave::wumpus_moves_message = "You hear the sound of the wumpus moving!";

const std::array<std::array<int, Cave::connections_per_room>, Cave::num_rooms> Cave::room_connections {{
	{{1, 4, 5}}, {{2, 0, 7}}, {{3, 1, 9}}, {{4, 2, 11}}, {{0, 3, 13}},
	{{6, 14, 0}}, {{7, 5, 15}}, {{8, 6, 1}}, {{9, 7, 16}}, {{10, 8, 2}}, {{11, 9, 17}}, {{12, 10, 3}}, {{13, 11, 18}}, {{14, 12, 4}}, {{5, 13, 19}},
	{{16, 19, 6}}, {{17, 15, 8}}, {{18, 16, 10}}, {{19, 17, 12}}, {{15, 18, 14}}
}};

Cave::Cave() {
	for (int i = 0; i < num_rooms; ++i) {
		rooms[i] = Room(i + 1);
		for (int j = 0; j < connections_per_room; ++j)
			rooms[i].adjacent_rooms[j] = &rooms[room_connections[i][j]];
	}
}

void Cave::print_game_info() const {
	std::ifstream in(game_info_file);
	if (!in)
		throw std::runtime_error("Unable to find game info file");
	std::string buff;
	while (std::getline(in, buff))
		std::cout << buff << std::endl;
}

void Cave::hunt() {
	init_hunt();
	while(!is_hunt_over())
		play_round_of_hunt();
	end_hunt();
}

void Cave::init_hunt() {
	reset_rooms();
	arrows = num_arrows;
	state = Game_state::none;
	shuffle_room_numbers();
	place_hazards();
}

void Cave::reset_rooms() {
	for (Room& room : rooms) {
		room.wumpus = false;
		room.bat = false;
		room.pit = false;
	}
}

void Cave::shuffle_room_numbers() {
	for (int i = 0; i < num_rooms; ++i) {
		int j = random(i, num_rooms - 1);
		int temp = rooms[j].number;
		rooms[j].number = rooms[i].number;
		rooms[i].number = temp;
	}
}

void Cave::place_hazards() {
	int player = random(0, num_rooms - 1);
	int wumpus = random(0, num_rooms - 1, {player});
	int first_bat = random(0, num_rooms - 1, {player, wumpus});
	int second_bat = random(0, num_rooms - 1, {player, wumpus, first_bat});
	int first_pit = random(0, num_rooms - 1, {player, wumpus, first_bat, second_bat});
	int second_pit = random(0, num_rooms - 1, {player, wumpus, first_bat, second_bat, first_pit});

	player_room = &rooms[player];
	wumpus_room = &rooms[wumpus];
	rooms[wumpus].wumpus = true;
	rooms[first_bat].bat = true;
	rooms[second_bat].bat = true;
	rooms[first_pit].pit = true;
	rooms[second_pit].pit = true;
}

bool Cave::is_hunt_over() const {
	return state != Game_state::none;
}

void Cave::play_round_of_hunt() {
	inform_player_of_hazards();
	Action action = get_action();
	switch (action.type) {
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
void Cave::inform_player_of_hazards() const {
	std::cout << std::endl;
	bool wumpus = false, bat = false, pit = false;
	for (Room* room : player_room->adjacent_rooms) {
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

Cave::Action Cave::get_action() const {
	while (true) {
		print_prompt();
		Action action;
		std::string input;
		std::getline(std::cin, input);
		std::istringstream in(input);
		char type = '\0';
		in >> type;
		switch (type) {
		case 'm':
		{
			action.type = Action::Action_type::move;
			in >> action.targets[0];
			for (Room* room : player_room->adjacent_rooms)
				if (room->number == action.targets[0])
					return action;
			break;
		}
		case 's':
		{
			if (arrows <= 0)
				break;
			action.type = Action::Action_type::shoot;
			in >> action.targets[0];
			bool valid = false;
			for (Room* room : player_room->adjacent_rooms) {
				if (room->number == action.targets[0]) {
					valid = true;
					break;
				}
			}
			if (!valid)
				break;
			for (int i = 1; i < arrow_range; ++i) {
				char ch = '\0';
				in.get(ch);
				in >> action.targets[i];
				if (!in)
					action.targets[i] = 0;
			}
			return action;
		}
		case 'q':
			action.type = Action::Action_type::quit;
			return action;
		case 'd':
			print_debug();
			break;
		}
	}
}

void Cave::print_prompt() const {
	std::cout << "You are in room " << player_room->number << "; "
		<< "There are tunnels to rooms "
		<< player_room->adjacent_rooms[0]->number;
	for (int i = 1; i < connections_per_room - 1; ++i)
		std::cout << ", " << player_room->adjacent_rooms[i]->number;
	std::cout << ", and " << player_room->adjacent_rooms[connections_per_room - 1]->number << ";" << std::endl
		<< "You have " << arrows << " arrows remaining; move (m), shoot (s), or quit (q)?" << std::endl;
}

void Cave::move(int target) {
	for (Room* room : player_room->adjacent_rooms)
		if (room->number == target)
			player_room = room;
	check_room_hazards();
}

void Cave::check_room_hazards() {
	while (true) {
		if (player_room->wumpus) {
			state = Game_state::player_eaten;
			return;
		}
		if (player_room->pit) {
			state = Game_state::player_fell;
			return;
		}
		if (player_room->bat) {
			std::cout << player_dropped_in_random_room_message << std::endl;
			player_room = &rooms[random(0, num_rooms - 1)];
			continue;
		}
		break;
	}
}

void Cave::shoot(const std::array<int, arrow_range>& targets) {
	--arrows;
	Room* room = player_room;
	Room* previous_room = nullptr;
	for (int i = 0; i < arrow_range; ++i) {
		Room* temp = room;
		room = get_next_room_for_arrow_flight(previous_room, room, targets[i]);
		previous_room = temp;
		if (room->wumpus) {
			state = Game_state::wumpus_dead;
			return;
		}
	}
	move_wumpus();
}

Cave::Room* Cave::get_next_room_for_arrow_flight(Room* previous_room, Room* current_room, int target) const {
	int previous_number = 0;
	if (previous_room)
		previous_number = previous_room->number;
	if (previous_number != target)
		for (Room* room : current_room->adjacent_rooms)
			if (room->number == target)
				return room;
	Room* random_room = current_room->adjacent_rooms[random(0, connections_per_room - 1)];
	while (random_room->number == previous_number)
		random_room = current_room->adjacent_rooms[random(0, connections_per_room - 1)];
	return random_room;
}

void Cave::move_wumpus() {
	std::cout << wumpus_moves_message << std::endl;
	Room* new_wumpus_room = wumpus_room->adjacent_rooms[random(0, connections_per_room - 1)];
	new_wumpus_room->wumpus = true;
	wumpus_room->wumpus = false;
	wumpus_room = new_wumpus_room;
	if (player_room->wumpus) {
		state = Game_state::player_eaten;
	}
}

void Cave::quit() {
	state = Game_state::player_quit;
}

void Cave::end_hunt() const {
	switch (state) {
	case Game_state::player_eaten:
		std::cout << player_eaten_message << std::endl;
		break;
	case Game_state::player_fell:
		std::cout << player_fell_message << std::endl;
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

int Cave::random(int lower, int upper) const {
	std::uniform_int_distribution<int> rng_dist(lower, upper);
	std::default_random_engine rng_engine {std::random_device()()};
	return rng_dist(rng_engine);
}

int Cave::random(int lower, int upper, const std::vector<int>& excludes) const {
	while (true) {
		int result = random(lower, upper);
		bool valid = true;
		for (int exclude : excludes) {
			if (result == exclude) {
				valid = false;
				break;
			}
		}
		if (valid)
			return result;
	}
}

void Cave::print_debug() const {
	for (const Room& room : rooms) {
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

