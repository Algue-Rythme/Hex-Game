#include <chrono>
#include <queue>
#include <random>

#include "Data.hpp"

using namespace std;
using namespace sf;

static const unsigned int seed = chrono::system_clock::now().time_since_epoch().count();
static default_random_engine randGenerator(seed);
extern unsigned int nbSimulations;

Data::Data(unsigned int _size) : size(_size), board(size*size, Player::Empty)
{ 
	for (unsigned int y = 0; y < size; ++y)
		for (unsigned int x = 0; x < size; ++x)
			movesList.emplace(x, y);
}

vector<Vector2u> Data::neighboursCoor(Vector2u position) const
{
	vector<Vector2u> neighbours;
	
	if (position.x > 0)
	{
		neighbours.emplace_back(position.x-1, position.y);
		if (position.y < size-1)
			neighbours.emplace_back(position.x-1, position.y+1);
	}
	
	if (position.x < size-1)
	{
		neighbours.emplace_back(position.x+1, position.y);
		if (position.y > 0)
			neighbours.emplace_back(position.x+1, position.y-1);
	}
	
	if (position.y > 0)
		neighbours.emplace_back(position.x, position.y-1);
	if(position.y < size-1)
		neighbours.emplace_back(position.x, position.y+1);
		
	return neighbours;
}

Player Data::winner() const
{
	queue<Edge> waitComputed;
	vector<bool> isComputed(size*size, false);
	for (unsigned int coor = 0; coor < size; ++coor)
	{
		if (board[coor*size] == Player::Human)
		{
			isComputed[coor*size] = true;
			waitComputed.emplace(Player::Human, Vector2u(0, coor));
		}
		if (board[coor] == Player::AI)
		{
			isComputed[coor] = true;
			waitComputed.emplace(Player::AI, Vector2u(coor, 0));
		}
	}
	
	while (!waitComputed.empty())
	{
		Edge current = waitComputed.front();
		waitComputed.pop();
		
		auto neighbours = neighboursCoor(current.position);
		for (auto& neighbour : neighbours)
		{
			unsigned int x = neighbour.x;
			unsigned int y = neighbour.y;
			if (!isComputed[y*size + x] && board[y*size + x] == current.player)
			{
				isComputed[y*size + x] = true;
				waitComputed.emplace(current.player, neighbour);
			}
		}
	}
	
	for (unsigned int coor = 0; coor < size; ++coor)
	{
		if (board[coor*size + (size-1)] == Player::Human && isComputed[coor*size + (size-1)])
			return Player::Human;
		if (board[(size-1)*size + coor] == Player::AI && isComputed[(size-1)*size + coor])
			return Player::AI;
	}
	
	return Player::Empty;
}

Player Data::MonteCarlo(Vector2u movePreced, Player current)
{
	vector<Vector2u> moves(this->moves());
	shuffle(moves.begin(), moves.end(), randGenerator);
	
	for (auto move : moves)
	{
		if (isEmpty(move))
		{
			disconnect(movePreced, current, move);
			makeMove(move, current);
			
			movePreced = move;
			current = nextPlayer(current);
		}
	}
	
	++nbSimulations;
	return winner();
}

void Data::disconnect(sf::Vector2u move, Player current, sf::Vector2u& answer) const
{
	static const vector<Vector2i> rotations =
	                   {Vector2i(0, 1), 
						Vector2i(-1, 1), 
						Vector2i(-1, 0), 
						Vector2i(0, -1), 
						Vector2i(1, -1), 
						Vector2i(1, 0)};  
						
	Vector2i plot1 = Vector2i(move) + Vector2i(1, -1);
	Vector2i path = Vector2i(move) + Vector2i(1, 0);
	Vector2i plot2 = Vector2i(move) + Vector2i(0, 1);
											    
	for (unsigned int rotation = 0; rotation < rotations.size(); ++rotation)
	{
		if (diamondPattern(plot1, path, plot2, current))
		{
			++nbSimulations;
			answer = Vector2u(path);
			return ;
		}
	
		plot1 += rotations[rotation];
		path += rotations[(rotation+1)%rotations.size()];
		plot2 += rotations[(rotation+2)%rotations.size()];
	}
}

bool Data::diamondPattern(Vector2i plot1, Vector2i path, Vector2i plot2, Player current) const
{
	if (!correct(path) || !isEmpty(Vector2u(path)))
		return false;
		
	// It is equivalent to "if (current == Human)" 
	bool pathOk = (path.x == 0) || (path.x == int(size)-1);
	if (current == Player::AI)
	{
		pathOk = (path.y == 0) || (plot1.y == int(size)-1);
	}
	
	bool plot1Ok = pathOk;
	if (correct(plot1))
		plot1Ok = (operator()(Vector2u(plot1)) == current);
		
	bool plot2Ok = pathOk;
	if (correct(plot2))
		plot2Ok = (operator()(Vector2u(plot2)) == current);
		
	return (plot1Ok && plot2Ok);
}

void Data::makeMove(Vector2u position, Player player)
{
	operator()(position) = player;
	auto it = movesList.find(position);
	if (it != movesList.end())
		movesList.erase(it);
}

bool Data::correct(Vector2i test) const
{
	return (test.x >= 0 && test.x < int(size) && test.y >= 0 && test.y < int(size));
}

vector<Vector2u> Data::moves() const
{
	return vector<Vector2u>(movesList.begin(), movesList.end());
}

bool Data::isEmpty(Vector2u position) const
{
	return operator()(position) == Player::Empty;
}

Player Data::operator()(unsigned int x, unsigned int y) const
{
	return board[y*size + x];
}

Player& Data::operator()(unsigned int x, unsigned int y)
{
	return board[y*size + x];
}

Player Data::operator()(Vector2u position) const
{
	return board[position.y*size + position.x];
}

Player& Data::operator()(Vector2u position)
{
	return board[position.y*size + position.x];
}