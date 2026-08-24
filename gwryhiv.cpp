#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

// 1. Base Class (Demonstrating Inheritance)
class Entity {
protected:
    int x, y;
    char symbol;
public:
    Entity(int startX, int startY, char sym) : x(startX), y(startY), symbol(sym) {}
    virtual ~Entity() {}

    int getX() const { return x; }
    int getY() const { return y; }
    char getSymbol() const { return symbol; }

    void setPosition(int nx, int ny) {
        x = nx;
        y = ny;
    }
};

// 2. Player Class (Derived from Entity)
class Player : public Entity {
    int hp;
    int gold;
public:
    Player(int startX, int startY) : Entity(startX, startY, '@'), hp(100), gold(0) {}

    int getHp() const { return hp; }
    int getGold() const { return gold; }

    void move(int dx, int dy) {
        x += dx;
        y += dy;
    }

    void addGold(int amt) { gold += amt; }
};

// 3. Simple Map and Game Loop
const int WIDTH = 10;
const int HEIGHT = 5;

void drawMap(const Player& player) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (x == player.getX() && y == player.getY()) {
                std::cout << player.getSymbol() << " ";
            } else {
                std::cout << ". "; // Empty floor
            }
        }
        std::cout << "\n";
    }
}

int main() {
    Player player(2, 2);
    char input;
    bool running = true;

    std::cout << "Use W/A/S/D to move, Q to quit.\n";

    while (running) {
        drawMap(player);
        std::cout << "HP: " << player.getHp() << " | Gold: " << player.getGold() << "\n";
        std::cout << "Move (w/a/s/d): ";
        std::cin >> input;

        int dx = 0, dy = 0;
        if (input == 'w') dy = -1;
        else if (input == 's') dy = 1;
        else if (input == 'a') dx = -1;
        else if (input == 'd') dx = 1;
        else if (input == 'q') running = false;

        // Basic boundary check
        int newX = player.getX() + dx;
        int newY = player.getY() + dy;
        if (newX >= 0 && newX < WIDTH && newY >= 0 && newY < HEIGHT) {
            player.move(dx, dy);
        }
    }

    return 0;
}