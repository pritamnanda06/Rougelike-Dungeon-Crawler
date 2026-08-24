#include <iostream>
#include <string>
#include <vector>

// Abstract Base Class
class Entity {
protected:
    int x, y;
    char symbol;
public:
    Entity(int startX, int startY, char sym) : x(startX), y(startY), symbol(sym) {}
    virtual ~Entity() {}

    int getX() const { return x; }
    int getY() const { return y; }
    void setPosition(int nx, int ny) { x = nx; y = ny; }
    char getSymbol() const { return symbol; }
    virtual void interact() = 0; // Pure virtual function
};

// Abstract Character Class
class Character : public Entity {
protected:
    std::string name;
    int hp;
    int maxHp;
    int attackPower;
public:
    Character(int startX, int startY, char sym, std::string charName, int health, int attack)
        : Entity(startX, startY, sym), name(charName), hp(health), maxHp(health), attackPower(attack) {}

    bool isAlive() const { return hp > 0; }
    int getHp() const { return hp; }

    void takeDamage(int amount) {
        hp -= amount;
        if (hp < 0) hp = 0;
        std::cout << name << " takes " << amount << " damage! HP left: " << hp << "\n";
    }

    virtual void attack(Character& target) {
        target.takeDamage(attackPower);
    }
};

// Player Class
class Player : public Character {
    int gold;
public:
    Player(int startX, int startY) : Character(startX, startY, '@', "Hero", 100, 15), gold(0) {}

    void interact() override {
        std::cout << "You inspect your surroundings.\n";
    }

    void addGold(int amount) {
        gold += amount;
        std::cout << "Picked up " << amount << " gold. Total: " << gold << "\n";
    }

    void move(int dx, int dy) {
        x += dx;
        y += dy;
    }
};

// Monster Class
class Monster : public Character {
public:
    Monster(int startX, int startY) : Character(startX, startY, 'G', "Goblin", 30, 8) {}

    void interact() override {
        std::cout << "A hostile Goblin growls at you!\n";
    }
};

// Grid Dimensions
const int WIDTH = 8;
const int HEIGHT = 5;

void drawGrid(const Player& p, const Monster& m) {
    for (int r = 0; r < HEIGHT; ++r) {
        for (int c = 0; c < WIDTH; ++c) {
            if (c == p.getX() && r == p.getY()) {
                std::cout << p.getSymbol() << " ";
            } else if (m.isAlive() && c == m.getX() && r == m.getY()) {
                std::cout << m.getSymbol() << " ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "\n";
    }
}

// Entry Point
int main() {
    Player player(1, 1);
    Monster goblin(5, 2);
    bool running = true;
    char input;

    std::cout << "--- ROGUELIKE OOP DEMO ---\n";
    std::cout << "Use W/A/S/D to move (@), walk into Goblin (G) to fight, Q to quit.\n\n";

    while (running && player.isAlive()) {
        drawGrid(player, goblin);
        std::cout << "HP: " << player.getHp() << " | Position: (" << player.getX() << "," << player.getY() << ")\n";
        std::cout << "Move: ";
        std::cin >> input;

        int dx = 0, dy = 0;
        if (input == 'w') dy = -1;
        else if (input == 's') dy = 1;
        else if (input == 'a') dx = -1;
        else if (input == 'd') dx = 1;
        else if (input == 'q') break;

        int nextX = player.getX() + dx;
        int nextY = player.getY() + dy;

        // Combat triggers when stepping onto Goblin position
        if (goblin.isAlive() && nextX == goblin.getX() && nextY == goblin.getY()) {
            goblin.interact();
            player.attack(goblin);
            if (goblin.isAlive()) {
                goblin.attack(player);
            } else {
                std::cout << "Goblin defeated!\n";
                player.addGold(25);
            }
        } else if (nextX >= 0 && nextX < WIDTH && nextY >= 0 && nextY < HEIGHT) {
            player.move(dx, dy);
        }
        std::cout << "\n";
    }

    std::cout << "Game Over!\n";
    return 0;
}