#include <fstream>

#include "Game.hpp"

using namespace std;

int main()
{
	unsigned int firstPlayer, size;
	ifstream in("config.txt");
	in >> firstPlayer >> size;
	Player player = Player::Human;
	if (firstPlayer == 1)
		player = Player::AI;

	Game game(size, player);
	game.launch();
}

