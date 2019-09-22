#ifndef GAME_h
#define GAME_h

// OTHER
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include<deque>

#include "donghua_gameStruct.h"
#include "donghua_gameobject.h"
#include "donghua_resource.h"

class Snake {
private:
	std::deque<Position> body;
	float mapWidth, mapHeight;
public:
	Snake(float w, float h){
		this->mapWidth = w;
		this->mapHeight = h;
		body.push_back(Position(w/2, h/2));
		body.push_back(Position(w / 2+1.0, h / 2));
		body.push_back(Position(w / 2+2.0, h / 2));
	}

	void moveUp(float distance = 0.0f) {
		Position head = this->body.front();
		head.x = head.x > distance ? head.x - distance : this->mapHeight - distance;
		this->body.push_front(head);
	}

	void moveDown(float distance = 0.0f) {
		Position head = this->body.front();
		head.x = head.x < this->mapHeight - distance ? head.x + distance : 0;
		this->body.push_front(head);
	}

	void moveLeft(float distance = 0.0f) {
		Position head = this->body.front();
		head.y = head.y >= distance ? head.y - distance : this->mapWidth - distance;
		this->body.push_front(head);
	}

	void moveRight(float distance = 0.0f) {
		Position head = this->body.front();
		head.y = head.y < this->mapWidth - distance ? head.y + distance : 0;
		this->body.push_front(head);
	}

	bool isDead(std::vector<std::vector<int>>& gameMap) {
		Position head = this->body.front();
		Position tail = this->body.back();
		
		if(gameMap[int(head.x)][int(head.y)] == 0 || gameMap[int(head.x)][int(head.y)] == 1){
			gameMap[int(tail.x)][int(tail.y)] = 0;
			this->body.pop_back();
		}
		if(gameMap[int(head.x)][int(head.y)] == 0 || gameMap[int(head.x)][int(head.y)] == 2){
			gameMap[int(head.x)][int(head.y)] = 1;
			return false;
		}
		
		return true;
	}

	bool isDead_Collision(std::vector<Position>& stones, std::vector<Position>& foods) {
		Position head = this->body.front();
		Position tail = this->body.back();
		// 判断是否撞墙
		for (std::vector<Position>::iterator ptr = stones.begin(); ptr != stones.end(); ptr++) {
			if (CheckCollision(head, *ptr)) {
				this->body.pop_front();
				return true;
			}
		}
		// 移除尾部，向前移动
		this->body.pop_back();
		// 判断是否碰到身体
		for (std::deque<Position>::iterator ptr = this->body.begin()+1; ptr != this->body.end(); ptr++) {
			if (CheckCollision(head, *ptr)) {
				return true;
			}
		}
		//判断是否吃到食物
		for (std::vector<Position>::iterator ptr = foods.begin(); ptr != foods.end(); ) {
			if (CheckCollision(head, *ptr)) {
				this->body.push_back(tail);
				ptr = foods.erase(ptr);
			}
			else {
				ptr++;
			}
		}
		return false;
	}

	bool CheckCollision(Position &one, Position &two) {
		// Collision x-axis?
		bool collisionX = one.x + one.size >= two.x &&
			two.x + two.size >= one.x;
		// Collision y-axis?
		bool collisionY = one.y + one.size >= two.y &&
			two.y + two.size >= one.y;
		// Collision only if on both axes
		return collisionX && collisionY;
	}

	Position getHead(){
		return this->body.front();
	}

	std::deque<Position> getSnake() const {
		return this->body;
	}
};

class Game {
private:
	// map: 0空地；1蛇；2食物；3障碍
	std::vector<std::vector<int>> gameMap;
	std::vector<Position> stone;
	std::vector<Position> food;
	Snake* snake;
	ResourceManager resourceManger;
	int width, height;
public:
	bool isFinish;
	int level;

	Game(int w, int h){
		this->width = w;
		this->height = h;
		this->level = 10;
		this->snake = new Snake(w, h);
		isFinish = false;
		this->initMapWithStone();
		this->randomGenFood();
	}

	void reset() {
		delete this->snake;
		this->snake = new Snake(this->width, this->height);
		isFinish = false;
		std::vector<Position> r;
		this->stone = r;
		this->food = r;
		this->initMapWithStone();
		this->randomGenFood();
	}

	void initMapWithStone(){
		std::vector<std::vector<int>> level1Map{
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,3,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,3,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,3,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,0,0,3,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,3,3,3,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		};
		this->gameMap = level1Map;
		for (int x = 0; x < this->height; x++) {
			for (int y = 0; y < this->width; y++) {
				if (this->gameMap[x][y] == 3) {
					this->stone.push_back(Position(x, y));
				}
			}
		}
	}

	void randomGenFood() {
		int x, y;
		std::deque<Position> s = this->snake->getSnake();
		while(true){
			bool flag = true;
			x = rand() % (this->height - 1) + 1;
			y = rand() % (this->width - 1) + 1;
			Position p(x, y);
			for (int i = 0; i < s.size(); i++) {
				if (this->snake->CheckCollision(s[i], p)) {
					flag = false;
					break;
				}
			}
			if(flag){
				this->food.push_back(p);
				break;
			}
		}
	}

	// snake
	void processSnakeMove(MOVE_DIRECTION direction, float distance = 0){
		if(isFinish) return;
		if(direction == UP){
			this->snake->moveUp(distance);
		}
		else if(direction == DOWN){
			this->snake->moveDown(distance);
		}
		else if(direction == LEFT){
			this->snake->moveLeft(distance);
		}
		else if(direction == RIGHT){
			this->snake->moveRight(distance);
		}
		if (this->snake->isDead_Collision(this->stone, this->food)) {
			this->isFinish = true;
		}
	}

	void initShaders(std::string name, const char* vertex, const char* fragment) {
		resourceManger.loadShader(name, vertex, fragment);
	}

	void initTextures(std::string name, char const* file) {
		resourceManger.loadTexture(name, file);
	}

	Shader* getShaderByName(std::string name) {
		return resourceManger.getShader(name);
	}

	Texture* getTextureByName(std::string name) {
		return resourceManger.getTexture(name);
	}

	void setupObjectTexture(Shader& shader, unsigned int depthMap, std::string mode = "floor"){
		if(mode == "floor"){
			// render floor
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, resourceManger.getTexture("floor")->getTexture());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthMap);
		}
		else if(mode == "snakeBody"){
			// render snake body
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, resourceManger.getTexture("snakeBody")->getTexture());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthMap);
		}
		else if(mode == "snakeHead"){
			// render snake head
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, resourceManger.getTexture("snakeHead")->getTexture());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthMap);
		}
		else if(mode == "food"){
			// render food
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, resourceManger.getTexture("food")->getTexture());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthMap);
		}
		else if(mode == "stone"){
			// render stone
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, resourceManger.getTexture("stone")->getTexture());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthMap);
		}
	}
	// render
	std::vector<glm::mat4> getRenderStoneMatrix() {
		std::vector<glm::mat4> result;	
		float x = 0, y = 0;
		for (int i = 0; i < this->stone.size(); i++) {
			calculatePosition(this->stone[i].x, this->stone[i].y, x, y);
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::translate(model, glm::vec3(x, y, 0.0f)) * glm::scale(model, glm::vec3(0.25f));
			result.push_back(model);
		}
		return result;
	}

	std::vector<glm::mat4> getRenderFoodMatrix() {
		std::vector<glm::mat4> result;
		float x = 0, y = 0;
		for (int i = 0; i < this->food.size(); i++) {
			calculatePosition(this->food[i].x, this->food[i].y, x, y);
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::translate(model, glm::vec3(x, y, 0.0f)) * glm::scale(model, glm::vec3(0.25f));
			result.push_back(model);
		}
		return result;
	}

	std::vector<glm::mat4> getRenderSnakeMatrix() {
		std::vector<glm::mat4> result;
		std::deque<Position> s = this->snake->getSnake();
		float x = 0, y = 0;
		for(std::deque<Position>::iterator ptr = s.begin(); ptr != s.end(); ptr++){
			calculatePosition(ptr->x, ptr->y, x, y);
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::translate(model, glm::vec3(x, y, 0.0f)) * glm::scale(model, glm::vec3(0.25f));
			result.push_back(model);
		}
		return result;
	}

	void calculatePosition(float& x, float& y, float& out_x, float& out_y){
		out_x = 0.5 * (y - this->height / 2);
		out_y = 0.5 * ((this->width - x));
	}

	// test
	void printMap(){
		Position a = this->snake->getHead();
		for(int i = 0; i < this->height; i++) {
			for(int j = 0; j < this->width; j++){
				std::cout << this->gameMap[i][j] << " ";
			}
			std::cout << std::endl;
		}
	}
};

#endif // !HW7_h