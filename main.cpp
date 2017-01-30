#include "cave.h"
using wumpus::Cave;

int main() {
	Cave cave;
	cave.print_game_info();
	cave.hunt();
	return 0;
}

