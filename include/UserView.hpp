#ifndef USER_VIEW_HPP
#define USER_VIEW_HPP

#include <vector>

#include <SFML/Graphics.hpp>

#include "Utils.hpp"

class Game;

class UserView : public sf::Drawable, public sf::Transformable
{
	public: 
	
		UserView(Game *, unsigned int);
		UserView(const UserView&) = delete;
		UserView& operator=(const UserView&) = delete;

		void close();
		bool collision(unsigned int, unsigned int, sf::Vector2u&) const;
		virtual void draw(sf::RenderTarget&, sf::RenderStates) const;
		bool isOpen() const;
		bool readUserInput();
		void setColor(sf::Vector2u position, Player player);
		void setColor(unsigned int, unsigned int, Player);
		
		virtual ~UserView();
	
	private:
	
		sf::Sprite createStone(sf::Vector2f position) const;
		sf::Sprite createStone(float, float) const;
		unsigned int index(unsigned int x, unsigned int y) const;
		void initBackground();
		void initHexagons();
		void initSide();
	
		sf::RenderWindow * window;
		sf::Thread * rendering;
		Game * game;
		sf::VertexArray vertices;
		unsigned int size;
		mutable sf::Mutex protectVertices;
		unsigned int width;
		unsigned int height;
		float cellSize;
		float hexagonSize;
		sf::Texture backgroundTexture;
		sf::Sprite background;
		sf::Texture cellTexture;
		sf::Texture stoneTexture;
		std::vector<sf::Sprite> stones;
};

#endif
