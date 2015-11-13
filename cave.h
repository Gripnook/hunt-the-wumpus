#ifndef WUMPUS_CAVE_H
#define WUMPUS_CAVE_H

#include <vector>
#include <array>
#include <string>

namespace wumpus {

class Cave {
public:
	Cave();

	void print_game_info() const;
	void hunt();
private:
	static const std::string wumpus_adjacent_message;
	static const std::string bat_adjacent_message;
	static const std::string pit_adjacent_message;
	static const std::string wumpus_dead_message;
	static const std::string player_eaten_message;
	static const std::string player_dropped_in_random_room_message;
	static const std::string player_fell_message;
	static const std::string player_quit_message;
	static const std::string wumpus_moves_message;

	static const int num_rooms = 20;
	static const int connections_per_room = 3;
	static const std::array<
		std::array<int, connections_per_room>,
		num_rooms> room_connections;
	
	static const int num_bats = 2;
	static const int num_pits = 2;
	
	static const int arrow_range = 3;
	static const int num_arrows = 5;

	struct Room {
		Room() : number{} {}
		explicit Room(int number) : number{number} {}
		int number;
		bool wumpus {false};
		bool bat {false};
		bool pit {false};
		std::array<Room*, connections_per_room> adjacent_rooms;
	};

	std::array<Room, num_rooms> rooms;
	Room* player_room {nullptr};
	Room* wumpus_room {nullptr};

	int arrows {num_arrows};

	enum class Game_state {
		none,
		player_eaten,
		player_fell,
		wumpus_dead,
		player_quit
	} state {Game_state::none};

	void init_hunt();
	void reset_rooms();
	void shuffle_room_numbers();
	void place_hazards();

	struct Action {
		enum Action_type {
			none, move, shoot, quit
		} type {none};
		std::array<int, arrow_range> targets;
	};

	bool is_hunt_over() const;
	void play_round_of_hunt();
	void inform_player_of_hazards() const;
	Action get_action() const;
	void print_prompt() const;
	void move(int target);
	void check_room_hazards();
	void shoot(const std::array<int, arrow_range>& targets);
	Room* get_next_room_for_arrow_flight(Room* previous_room, Room* current_room, int target) const;
	void move_wumpus();
	void quit();
	void end_hunt() const;

	int random(int lower, int upper) const;
	int random(int lower, int upper, const std::vector<int>& excludes) const;
	
	void print_debug() const;
};

}

#endif

