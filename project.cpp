
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

// ---------------------------------------------------------------- util
namespace util
{
    inline int sign(int v) { return (v > 0) - (v < 0); } // inline function
    template <typename T>
    T clampTo(T v, T lo, T hi) { return v < lo ? lo : (v > hi ? hi : v); } // function template
    string toText(int n)                                                   // C-style buffer + sprintf
    {
        char buf[20];
        sprintf(buf, "%d", n);
        return string(buf);
    }
    string banner(const char *title, int width = 40) // C strings, default argument
    {
        char buf[81];
        if (width > 80)
            width = 80;
        memset(buf, '=', width);
        buf[width] = '\0';
        int len = (int)strlen(title);
        if (len > width)
            len = width;
        memcpy(buf + (width - len) / 2, title, len);
        return string(buf);
    }
    string lower(string s)
    {
        transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }
}
// ---------------------------------------------------------------- game namespace
namespace rl
{
    using util::toText;
    const char *GAME_TITLE = " SHADOWS OF THE KEEP ";
    // Dice: static members + overloading + default argument
    class Dice
    {
    private:
        static int rolls;

    public:
        static int roll(int sides)
        {
            rolls++;
            return rand() % sides;
        }
        static int roll(int lo, int hi)
        {
            rolls++;
            return lo + rand() % (hi - lo + 1);
        } // overload
        static bool chance(int percent = 50)
        {
            rolls++;
            return rand() % 100 < percent;
        }
        static int getRolls() { return rolls; }
    };
    int Dice::rolls = 0;
    // Vec2: 2D position with overloaded operators
    class Vec2
    {
    private:
        int x, y;

    public:
        Vec2(int px = 0, int py = 0) : x(px), y(py) {}
        int getX() const { return x; }
        int getY() const { return y; }
        Vec2 operator+(const Vec2 &o) const { return Vec2(x + o.x, y + o.y); } // binary, member
        Vec2 operator-(const Vec2 &o) const { return Vec2(x - o.x, y - o.y); }
        Vec2 operator-() const { return Vec2(-x, -y); } // unary
        Vec2 &operator+=(const Vec2 &o)
        {
            x += o.x;
            y += o.y;
            return *this;
        } // reference return
        bool operator==(const Vec2 &o) const { return x == o.x && y == o.y; }
        bool operator!=(const Vec2 &o) const { return !(*this == o); }
        int manhattan() const { return abs(x) + abs(y); }
        friend ostream &operator<<(ostream &os, const Vec2 &v) { return os << "(" << v.x << "," << v.y << ")"; }
    };
    const Vec2 ZERO(0, 0); // const object
    const Vec2 DIRS[4] = {Vec2(1, 0), Vec2(-1, 0), Vec2(0, 1), Vec2(0, -1)};
    // Gold: data conversion int <-> Gold
    class Gold
    {
    private:
        int amount;

    public:
        Gold(int a = 0) : amount(a) {}          // int -> Gold
        operator int() const { return amount; } // Gold -> int
        Gold &operator+=(int a)
        {
            amount += a;
            return *this;
        }
    };
    // Trace: records constructor / destructor order
    class Trace
    {
    private:
        static vector<string> events;

    public:
        static void log(const string &s)
        {
            events.push_back(s);
            if (events.size() > 80)
                events.erase(events.begin());
        }
        static const vector<string> &get() { return events; }
    };
    vector<string> Trace::events;
    // MessageLog: text shown under the map each turn
    class MessageLog
    {
    private:
        static string text;

    public:
        static void add(const string &s) { text += s + " "; }
        static void clear() { text.clear(); }
        static const string &get() { return text; }
    };
    string MessageLog::text;
    // Exception hierarchy
    class GameError : public exception
    {
    protected:
        string msg;

    public:
        explicit GameError(const string &m) : msg(m) {}
        virtual ~GameError() noexcept {}
        const char *what() const noexcept override { return msg.c_str(); }
    };
    class MoveError : public GameError
    {
    public:
        explicit MoveError(const string &m) : GameError(m) {}
    };
    class BackpackError : public GameError
    {
    public:
        explicit BackpackError(const string &m) : GameError(m) {}
    };
    class FileError : public GameError
    {
    public:
        explicit FileError(const string &m) : GameError(m) {}
    };
    // Object slicing demo
    class Report
    {
    public:
        virtual ~Report() {}
        virtual string text() const { return "basic report"; }
    };
    class DetailedReport : public Report
    {
    private:
        string extra;

    public:
        explicit DetailedReport(const string &e) : extra(e) {}
        string text() const { return "detailed report (" + extra + ")"; }
    };
    string reportByValue(Report r) { return r.text(); }            // slices the object
    string reportByReference(const Report &r) { return r.text(); } // keeps late binding
    // Plain data for files: struct, union, binary + text streams
    struct SaveData
    {
        int version, depth, hp, maxHp, attack, gold, kills;
        unsigned int checksum;
    };
    union Word // an int viewed as a number or as its bytes
    {
        int asInt;
        unsigned char bytes[sizeof(int)];
    };
    unsigned int checksumOf(const SaveData &s)
    {
        int fields[7] = {s.version, s.depth, s.hp, s.maxHp, s.attack, s.gold, s.kills};
        unsigned int sum = 0;
        for (int i = 0; i < 7; i++)
        {
            Word w;
            w.asInt = fields[i];
            for (size_t b = 0; b < sizeof(int); b++)
                sum = sum * 31 + w.bytes[b];
        }
        return sum;
    }
    struct ScoreEntry
    {
        int depth, kills, gold;
        int points() const { return depth * 100 + kills * 10 + gold; }
    };
    bool operator>(const ScoreEntry &a, const ScoreEntry &b) { return a.points() > b.points(); }
    struct SumPoints // function object for std::accumulate
    {
        int operator()(int acc, const ScoreEntry &e) const { return acc + e.points(); }
    };
    class SaveManager
    {
    public:
        static void saveBinary(const string &path, const SaveData &s)
        {
            ofstream out(path.c_str(), ios::binary);
            if (!out)
                throw FileError("Cannot open " + path + " for writing.");
            SaveData copy = s;
            copy.checksum = checksumOf(copy);
            out.write(reinterpret_cast<const char *>(&copy), sizeof(copy));
            if (!out)
                throw FileError("Write to " + path + " failed.");
        }
        static SaveData loadBinary(const string &path)
        {
            ifstream in(path.c_str(), ios::binary);
            if (!in)
                throw FileError("No save file found (" + path + ").");
            SaveData s;
            in.read(reinterpret_cast<char *>(&s), sizeof(s));
            if (in.gcount() != (streamsize)sizeof(s))
                throw FileError("Save file is truncated.");
            if (s.checksum != checksumOf(s))
                throw FileError("Save file is corrupt (checksum mismatch).");
            return s;
        }
        static void appendScore(const string &path, const ScoreEntry &e)
        {
            ofstream out(path.c_str(), ios::app);
            if (!out)
                throw FileError("Cannot open " + path + " for appending.");
            out << "Depth: " << e.depth << " Kills: " << e.kills << " Gold: " << e.gold << "\n";
        }
        static vector<ScoreEntry> loadScores(const string &path)
        {
            vector<ScoreEntry> list;
            ifstream in(path.c_str());
            ScoreEntry e;
            string l1, l2, l3; // skip the "Depth:" "Kills:" "Gold:" labels
            while (in >> l1 >> e.depth >> l2 >> e.kills >> l3 >> e.gold)
                list.push_back(e);
            return list;
        }
    };
    // Class templates: Stack<T> (linked list) and Grid<T,COLS,ROWS>
    template <typename T>
    class Stack
    {
    private:
        struct Node
        {
            T value;
            Node *next;
            Node(const T &v, Node *n) : value(v), next(n) {}
        };
        Node *head;
        int count;

    public:
        Stack() : head(NULL), count(0) {}
        Stack(const Stack &) = delete; // owns raw nodes: no copying
        Stack &operator=(const Stack &) = delete;
        ~Stack()
        {
            while (head != NULL)
                pop();
        }
        void push(const T &v)
        {
            head = new Node(v, head);
            count++;
        }
        void pop()
        {
            if (head == NULL)
                throw underflow_error("Stack is empty");
            Node *old = head;
            head = head->next;
            delete old;
            count--;
        }
        T &top()
        {
            if (head == NULL)
                throw underflow_error("Stack is empty");
            return head->value;
        }
        bool empty() const { return head == NULL; }
        int size() const { return count; }
    };
    template <typename T, int COLS, int ROWS>
    class Grid // non-type template parameters
    {
    private:
        T cells[ROWS][COLS];

    public:
        Grid() { fill(T()); }
        void fill(T v)
        {
            for (int y = 0; y < ROWS; y++)
                for (int x = 0; x < COLS; x++)
                    cells[y][x] = v;
        }
        bool inBounds(int x, int y) const { return x >= 0 && y >= 0 && x < COLS && y < ROWS; }
        T &at(int x, int y)
        {
            if (!inBounds(x, y))
                throw out_of_range("Grid index out of range");
            return cells[y][x];
        }
        const T &at(int x, int y) const
        {
            if (!inBounds(x, y))
                throw out_of_range("Grid index out of range");
            return cells[y][x];
        }
    };
    // Capability classes sharing a VIRTUAL base (diamond inheritance)
    class Trait
    {
    protected:
        string label;

    public:
        Trait(const string &l = "creature") : label(l) { Trace::log("  Trait ctor (virtual base: built once)"); }
        virtual ~Trait() { Trace::log("  Trait dtor"); }
        const string &getLabel() const { return label; }
    };
    class Flying : virtual public Trait
    {
    public:
        Flying() { Trace::log("  Flying ctor"); }
        virtual ~Flying() { Trace::log("  Flying dtor"); }
    };
    class Poisonous : virtual public Trait
    {
    public:
        Poisonous() { Trace::log("  Poisonous ctor"); }
        virtual ~Poisonous() { Trace::log("  Poisonous dtor"); }
        int poisonTurns() const { return 3; }
    };
    // Entity: abstract base class for everything on the map
    class Entity
    {
    private:
        static int liveCount; // static members
        static int nextId;
        const int id; // const member
        Vec2 pos;
        char symbol;

    protected:
        void setSymbol(char s) { symbol = s; }

    public:
        Entity(int sx, int sy, char sym) : id(nextId++), pos(sx, sy), symbol(sym)
        {
            liveCount++;
            Trace::log("Entity ctor #" + toText(id));
        }
        Entity(const Entity &o) : id(nextId++), pos(o.pos), symbol(o.symbol) { liveCount++; }
        virtual ~Entity()
        {
            liveCount--;
            Trace::log("Entity dtor #" + toText(id));
        }
        int getId() const { return id; }
        int getX() const { return pos.getX(); }
        int getY() const { return pos.getY(); }
        const Vec2 &getPos() const { return pos; }
        char getSymbol() const { return symbol; }
        Entity &setPosition(int nx, int ny)
        {
            pos = Vec2(nx, ny);
            return *this;
        } // chainable (this)
        Entity &moveBy(const Vec2 &d)
        {
            pos += d;
            return *this;
        }
        static int getLiveCount() { return liveCount; }
        virtual void interact() = 0;         // pure virtual
        virtual string describe() const = 0; // pure virtual
        virtual bool blocksMovement() const { return false; }
        virtual bool isActive() const { return true; } // false = dead or used up
    };
    int Entity::liveCount = 0;
    int Entity::nextId = 1;
    struct IsActive // functor
    {
        bool operator()(const Entity *e) const { return e->isActive(); }
    };
    template <typename T>
    T *findActive(const vector<T *> &v, int x, int y)
    {
        for (typename vector<T *>::const_iterator it = v.begin(); it != v.end(); ++it)
            if ((*it)->isActive() && (*it)->getX() == x && (*it)->getY() == y)
                return *it;
        return NULL;
    }
    template <typename T>
    void purgeInactive(vector<T *> &v) // deletes dead / used objects
    {
        typename vector<T *>::iterator mid = stable_partition(v.begin(), v.end(), IsActive());
        for (typename vector<T *>::iterator it = mid; it != v.end(); ++it)
            delete *it;
        v.erase(mid, v.end());
    }
    // Actor: an Entity with health that can fight
    class Actor : public Entity
    {
    protected:
        int hp, maxHp, attack;

    public:
        Actor(int x, int y, char sym, int health, int atk)
            : Entity(x, y, sym), hp(health), maxHp(health), attack(atk)
        {
            Trace::log("  Actor ctor");
        }
        virtual ~Actor() { Trace::log("  Actor dtor"); }
        int getHp() const { return max(hp, 0); }
        int getMaxHp() const { return maxHp; }
        int getAttack() const { return attack; }
        bool isAlive() const { return hp > 0; }
        bool isActive() const override { return isAlive(); }
        Actor &takeDamage(int d)
        {
            hp -= d;
            return *this;
        }
        Actor &heal(int n)
        {
            hp = min(maxHp, hp + n);
            return *this;
        }
        Actor &boostAttack(int n)
        {
            attack += n;
            return *this;
        }
        bool blocksMovement() const override { return true; }
        friend ostream &operator<<(ostream &os, const Actor &a)
        {
            return os << "HP " << a.getHp() << "/" << a.maxHp << "  ATK " << a.attack;
        }
    };
    // Backpack: owns a dynamic array -> rule of three
    class Backpack
    {
    private:
        int capacity;
        int count;
        int *potions;
        void grow()
        {
            int newCap = capacity * 2;
            int *bigger = new int[newCap];
            for (int i = 0; i < count; i++)
                bigger[i] = potions[i];
            delete[] potions;
            potions = bigger;
            capacity = newCap;
        }

    public:
        Backpack(int cap = 2) : capacity(cap < 1 ? 1 : cap), count(0), potions(new int[capacity]) {}
        Backpack(const Backpack &o) : capacity(o.capacity), count(o.count), potions(new int[o.capacity])
        {
            for (int i = 0; i < count; i++)
                potions[i] = o.potions[i];
            Trace::log("  Backpack copy constructor (deep copy)");
        }
        Backpack &operator=(const Backpack &o)
        {
            if (this == &o)
                return *this; // self-assignment guard
            int *fresh = new int[o.capacity];
            for (int i = 0; i < o.count; i++)
                fresh[i] = o.potions[i];
            delete[] potions;
            potions = fresh;
            capacity = o.capacity;
            count = o.count;
            return *this;
        }
        ~Backpack() { delete[] potions; }
        Backpack &operator+=(int strength)
        {
            if (count == capacity)
                grow();
            potions[count++] = strength;
            return *this;
        }
        int operator[](int i) const
        {
            if (i < 0 || i >= count)
                throw BackpackError("Backpack slot out of range.");
            return potions[i];
        }
        int take()
        {
            if (count == 0)
                throw BackpackError("Your backpack is empty.");
            return potions[--count];
        }
        int size() const { return count; }
        bool empty() const { return count == 0; }
        friend ostream &operator<<(ostream &os, const Backpack &b)
        {
            os << "[";
            for (int i = 0; i < b.count; i++)
                os << (i > 0 ? " " : "") << "+" << b.potions[i];
            return os << "]";
        }
    };
    // Player
    class Player : public Actor
    {
    private:
        Gold gold;
        int kills, depth, poisoned;
        Backpack pack;
        map<string, int> killsByType;

    public:
        Player() : Actor(0, 0, '@', 20, 3), gold(0), kills(0), depth(1), poisoned(0) {}
        void addGold(int g) { gold += g; }
        void addKill(const string &type)
        {
            kills++;
            killsByType[type]++;
        }
        void descend() { depth++; }
        int getGold() const { return gold; } // Gold -> int
        int getKills() const { return kills; }
        int getDepth() const { return depth; }
        const map<string, int> &getKillsByType() const { return killsByType; }
        Backpack &getPack() { return pack; }
        const Backpack &getPack() const { return pack; }
        void poison(int turns)
        {
            if (turns > poisoned)
                poisoned = turns;
        }
        bool isPoisoned() const { return poisoned > 0; }
        void tickStatus()
        {
            if (poisoned > 0)
            {
                takeDamage(1);
                poisoned--;
                MessageLog::add("Poison burns (-1 HP).");
            }
        }
        void drinkPotion() // may throw BackpackError
        {
            int strength = pack.take();
            heal(strength);
            poisoned = 0;
            MessageLog::add("You drink a potion: +" + toText(strength) + " HP, poison cured.");
        }
        void restore(const SaveData &s)
        {
            hp = s.hp;
            maxHp = s.maxHp;
            attack = s.attack;
            gold = Gold(s.gold);
            kills = s.kills;
            depth = s.depth;
            poisoned = 0;
        }
        void interact() override
        {
            heal(1);
            MessageLog::add("You catch your breath (+1 HP).");
        }
        string describe() const override { return "You, the adventurer"; }
    };
    // World: interface letting enemies ask the game about the map
    class World
    {
    public:
        virtual ~World() {}
        virtual bool isFree(int x, int y) const = 0;
    };
    // Enemy: abstract Actor with AI. interact() = the player attacks it.
    class Enemy : public Actor
    {
    protected:
        Player &player;
        int damage;
        int sightRange;
        string name;
        int dist() const { return (player.getPos() - getPos()).manhattan(); }
        virtual bool attackPlayer() // true when the attack hit
        {
            if (Dice::chance(75))
            {
                int d = damage + Dice::roll(2);
                player.takeDamage(d);
                MessageLog::add(name + " hits you for " + toText(d) + ".");
                return true;
            }
            MessageLog::add(name + " misses.");
            return false;
        }
        void chase(const World &w) // greedy step toward the player
        {
            Vec2 d = player.getPos() - getPos();
            Vec2 options[2];
            if (abs(d.getX()) >= abs(d.getY()))
            {
                options[0] = Vec2(util::sign(d.getX()), 0);
                options[1] = Vec2(0, util::sign(d.getY()));
            }
            else
            {
                options[0] = Vec2(0, util::sign(d.getY()));
                options[1] = Vec2(util::sign(d.getX()), 0);
            }
            for (int k = 0; k < 2; k++)
            {
                if (options[k] == ZERO)
                    continue;
                Vec2 target = getPos() + options[k];
                if (w.isFree(target.getX(), target.getY()))
                {
                    setPosition(target.getX(), target.getY());
                    return;
                }
            }
        }
        void standardAI(const World &w)
        {
            int d = dist();
            if (d == 1)
                attackPlayer();
            else if (d <= sightRange)
                chase(w);
        }

    public:
        Enemy(int x, int y, char sym, int health, int dmg, Player &p, const string &n)
            : Actor(x, y, sym, health, dmg), player(p), damage(dmg), sightRange(8), name(n)
        {
            Trace::log("  Enemy ctor (" + name + ")");
        }
        virtual ~Enemy() { Trace::log("  Enemy dtor (" + name + ")"); }
        virtual void takeTurn(const World &w) = 0;
        void interact() override // the player bumps into the enemy -> attacks it
        {
            int d = player.getAttack() + Dice::roll(3);
            takeDamage(d);
            MessageLog::add("You hit " + name + " for " + toText(d) + ".");
            if (!isAlive())
            {
                int g = 2 + Dice::roll(5) + player.getDepth();
                player.addKill(name);
                player.addGold(g);
                MessageLog::add(name + " slain! +" + toText(g) + " gold.");
            }
        }
        string describe() const override { return name + " (HP " + toText(getHp()) + "/" + toText(getMaxHp()) + ")"; }
    };
    class Goblin : public Enemy
    {
    public:
        Goblin(int x, int y, int depth, Player &p)
            : Enemy(x, y, 'g', 5 + depth * 2 + Dice::roll(3), 1 + depth / 2, p, "Goblin")
        {
            Trace::log("  Goblin ctor");
        }
        virtual ~Goblin() { Trace::log("  Goblin dtor"); }
        void takeTurn(const World &w) override { standardAI(w); }
    };
    class GoblinChief : public Goblin // multilevel inheritance
    {
    private:
        int tick;

    public:
        GoblinChief(int x, int y, int depth, Player &p) : Goblin(x, y, depth + 2, p), tick(0)
        {
            name = "Goblin Chief";
            setSymbol('G');
            hp += 8;
            maxHp += 8;
            attack += 1;
            damage += 1;
            Trace::log("  GoblinChief ctor");
        }
        virtual ~GoblinChief() { Trace::log("  GoblinChief dtor"); }
        void takeTurn(const World &w) override
        {
            tick++;
            if (tick % 3 == 0 && dist() <= sightRange)
            {
                heal(2);
                MessageLog::add("The Goblin Chief roars and recovers 2 HP!");
            }
            Goblin::takeTurn(w); // extend inherited behaviour
        }
    };
    class Orc : public Enemy // tough, acts every other turn
    {
    private:
        int tick;

    public:
        Orc(int x, int y, int depth, Player &p)
            : Enemy(x, y, 'O', 10 + depth * 3 + Dice::roll(4), 3 + depth / 2, p, "Orc"), tick(0)
        {
            Trace::log("  Orc ctor");
        }
        virtual ~Orc() { Trace::log("  Orc dtor"); }
        void takeTurn(const World &w) override
        {
            tick++;
            if (tick % 2 == 0)
                standardAI(w);
        }
    };
    class Bat : public Enemy, public Flying // multiple inheritance
    {
    public:
        Bat(int x, int y, int depth, Player &p)
            : Trait("flier"), Enemy(x, y, 'b', 3 + depth, 1, p, "Bat"), Flying()
        {
            Trace::log("  Bat ctor");
        }
        virtual ~Bat() { Trace::log("  Bat dtor"); }
        void takeTurn(const World &w) override
        {
            if (dist() == 1)
            {
                attackPlayer();
                return;
            }
            if (dist() > sightRange)
                return;
            if (Dice::chance(50)) // flutter randomly
            {
                Vec2 t = getPos() + DIRS[Dice::roll(4)];
                if (w.isFree(t.getX(), t.getY()))
                    setPosition(t.getX(), t.getY());
            }
            else
                chase(w);
        }
    };
    // Hybrid inheritance: Bat (Enemy + Flying) + Poisonous, sharing ONE virtual Trait
    class VenomBat : public Bat, public Poisonous
    {
    public:
        VenomBat(int x, int y, int depth, Player &p) : Trait("venomous flier"), Bat(x, y, depth, p), Poisonous()
        {
            name = "Venom Bat";
            setSymbol('B');
            hp += 2;
            maxHp += 2;
            Trace::log("  VenomBat ctor");
        }
        virtual ~VenomBat() { Trace::log("  VenomBat dtor"); }
        void takeTurn(const World &w) override
        {
            if (dist() == 1)
            {
                if (attackPlayer())
                {
                    player.poison(poisonTurns());
                    MessageLog::add("You are poisoned!");
                }
            }
            else
                Bat::takeTurn(w);
        }
        string describe() const override { return Bat::describe() + " [" + getLabel() + "]"; }
    };
    // Item: pick-ups. interact() = the player uses the item.
    class Item : public Entity
    {
    protected:
        Player &player;
        bool used;

    public:
        Item(int x, int y, char sym, Player &p) : Entity(x, y, sym), player(p), used(false) {}
        bool isActive() const override { return !used; }
    };
    class Potion : public Item
    {
    private:
        int strength;

    public:
        Potion(int x, int y, Player &p, int s = 8) : Item(x, y, '!', p), strength(s) {} // default arg
        void interact() override
        {
            player.getPack() += strength;
            used = true;
            MessageLog::add("Potion stowed (" + toText(player.getPack().size()) + " carried). Press e to drink.");
        }
        string describe() const override { return "Healing potion (+" + toText(strength) + " HP)"; }
    };
    class GoldPile : public Item
    {
    private:
        int amount;

    public:
        GoldPile(int x, int y, Player &p) : Item(x, y, '$', p), amount(Dice::roll(10, 19)) {}
        void interact() override
        {
            player.addGold(amount);
            used = true;
            MessageLog::add("Found " + toText(amount) + " gold.");
        }
        string describe() const override { return "Pile of " + toText(amount) + " gold"; }
    };
    class Sword : public Item
    {
    public:
        Sword(int x, int y, Player &p) : Item(x, y, '/', p) {}
        void interact() override
        {
            player.boostAttack(1);
            used = true;
            MessageLog::add("Sword! Attack is now " + toText(player.getAttack()) + ".");
        }
        string describe() const override { return "Sword (+1 ATK)"; }
    };
    // Dungeon: tile map (rooms + corridors + stairs + fog of war)
    struct Room
    {
        int x, y, w, h;
        int centerX() const { return x + w / 2; }
        int centerY() const { return y + h / 2; }
    };
    class Dungeon
    {
    public:
        enum
        {
            W = 40,
            H = 24,
            WALL = 0,
            FLOOR = 1,
            STAIRS = 2
        };

    private:
        Grid<int, W, H> grid;
        Grid<bool, W, H> seen;
        vector<Room> rooms;
        void carveRoom(const Room &r)
        {
            for (int y = r.y; y < r.y + r.h; y++)
                for (int x = r.x; x < r.x + r.w; x++)
                    grid.at(x, y) = FLOOR;
        }
        void carveCorridor(int x1, int y1, int x2, int y2)
        {
            int x = x1, y = y1;
            while (x != x2)
            {
                grid.at(x, y) = FLOOR;
                x += (x2 > x) ? 1 : -1;
            }
            while (y != y2)
            {
                grid.at(x, y) = FLOOR;
                y += (y2 > y) ? 1 : -1;
            }
            grid.at(x, y) = FLOOR;
        }
        static bool overlaps(const Room &a, const Room &b)
        {
            return a.x <= b.x + b.w + 1 && a.x + a.w + 1 >= b.x && a.y <= b.y + b.h + 1 && a.y + a.h + 1 >= b.y;
        }

    public:
        void generate()
        {
            do
            {
                rooms.clear();
                grid.fill(WALL);
                seen.fill(false);
                for (int t = 0; t < 60 && rooms.size() < 9; t++)
                {
                    Room r;
                    r.w = 4 + Dice::roll(6);
                    r.h = 3 + Dice::roll(4);
                    r.x = 1 + Dice::roll(W - r.w - 2);
                    r.y = 1 + Dice::roll(H - r.h - 2);
                    bool ok = true;
                    for (size_t i = 0; i < rooms.size(); i++)
                        if (overlaps(r, rooms[i]))
                            ok = false;
                    if (!ok)
                        continue;
                    carveRoom(r);
                    if (!rooms.empty())
                    {
                        const Room &p = rooms.back();
                        carveCorridor(p.centerX(), p.centerY(), r.centerX(), r.centerY());
                    }
                    rooms.push_back(r);
                }
            } while (rooms.size() < 3);
            grid.at(rooms.back().centerX(), rooms.back().centerY()) = STAIRS;
        }
        const vector<Room> &getRooms() const { return rooms; }
        int tile(int x, int y) const { return grid.at(x, y); }
        bool isWalkable(int x, int y) const { return grid.inBounds(x, y) && grid.at(x, y) != WALL; }
        static bool isVisible(int x, int y, int px, int py) // vision radius 6
        {
            int dx = x - px, dy = y - py;
            return dx * dx + dy * dy <= 36;
        }
        void updateSeen(int px, int py)
        {
            for (int y = 0; y < H; y++)
                for (int x = 0; x < W; x++)
                    if (isVisible(x, y, px, py))
                        seen.at(x, y) = true;
        }
        bool wasSeen(int x, int y) const { return seen.at(x, y); }
        bool isWallEdge(int x, int y) const // only walls touching floor are drawn
        {
            if (grid.at(x, y) != WALL)
                return false;
            for (int dy = -1; dy <= 1; dy++)
                for (int dx = -1; dx <= 1; dx++)
                    if (grid.inBounds(x + dx, y + dy) && grid.at(x + dx, y + dy) != WALL)
                        return true;
            return false;
        }
    };
    struct LevelRecord // pushed onto Stack<LevelRecord> when leaving a level
    {
        int depth, kills, gold;
    };
    // Game: owns everything and runs the main loop
    class Game : public World
    {
    private:
        Dungeon dungeon;
        Player player;
        vector<Enemy *> enemies; // base-class pointers -> late binding
        vector<Item *> items;
        Stack<LevelRecord> history;
        Backpack snapshot; // copy of the pack taken at the last stairs
        bool running;
        void clearEntities()
        {
            for (size_t i = 0; i < enemies.size(); i++)
                delete enemies[i]; // virtual dtor chain
            for (size_t i = 0; i < items.size(); i++)
                delete items[i];
            enemies.clear();
            items.clear();
        }
        Entity *entityAt(int x, int y) const
        {
            Enemy *e = findActive(enemies, x, y);
            if (e != NULL)
                return e;
            return findActive(items, x, y);
        }
        bool isFree(int x, int y) const override // World interface
        {
            return dungeon.isWalkable(x, y) && findActive(enemies, x, y) == NULL && !(Vec2(x, y) == player.getPos());
        }
        bool randomSpot(const Room &r, int &x, int &y)
        {
            for (int tries = 0; tries < 200; tries++)
            {
                x = r.x + Dice::roll(r.w);
                y = r.y + Dice::roll(r.h);
                if (dungeon.tile(x, y) == Dungeon::FLOOR && entityAt(x, y) == NULL && !(Vec2(x, y) == player.getPos()))
                    return true;
            }
            return false;
        }
        void spawnEnemy(const Room &r, bool boss = false)
        {
            int x, y;
            if (!randomSpot(r, x, y))
                return;
            int depth = player.getDepth();
            if (boss)
            {
                enemies.push_back(new GoblinChief(x, y, depth, player));
                return;
            }
            int roll = Dice::roll(100);
            if (depth >= 4 && roll < 12)
                enemies.push_back(new VenomBat(x, y, depth, player));
            else if (depth >= 3 && roll < 30)
                enemies.push_back(new Orc(x, y, depth, player));
            else if (depth >= 2 && roll < 55)
                enemies.push_back(new Bat(x, y, depth, player));
            else
                enemies.push_back(new Goblin(x, y, depth, player));
        }
        void newLevel()
        {
            dungeon.generate();
            clearEntities();
            const vector<Room> &rooms = dungeon.getRooms();
            player.setPosition(rooms[0].centerX(), rooms[0].centerY());
            int depth = player.getDepth();
            for (size_t i = 1; i < rooms.size(); i++)
            {
                int count = min(4, 1 + Dice::roll(2 + depth / 2));
                for (int k = 0; k < count; k++)
                    spawnEnemy(rooms[i]);
                int x, y;
                if (Dice::chance(50) && randomSpot(rooms[i], x, y))
                    items.push_back(new Potion(x, y, player, 5 + Dice::roll(6)));
                if (Dice::chance(60) && randomSpot(rooms[i], x, y))
                    items.push_back(new GoldPile(x, y, player));
                if (Dice::chance(25) && randomSpot(rooms[i], x, y))
                    items.push_back(new Sword(x, y, player));
            }
            if (depth % 3 == 0)
                spawnEnemy(rooms.back(), true); // a chief guards every third staircase
            dungeon.updateSeen(player.getX(), player.getY());
        }
        void descend()
        {
            LevelRecord rec = {player.getDepth(), player.getKills(), player.getGold()};
            history.push(rec);
            snapshot = player.getPack(); // assignment operator (deep copy)
            player.descend();
            newLevel();
            player.heal(5);
            MessageLog::add("You descend to depth " + toText(player.getDepth()) + ".");
        }
        // One player turn: rest, attack, pick up, or move. Throws MoveError on a wall.
        void playerAct(int dx, int dy)
        {
            if (dx == 0 && dy == 0)
            {
                player.interact();
                return;
            }
            Vec2 target = player.getPos() + Vec2(dx, dy);
            Entity *hit = entityAt(target.getX(), target.getY());
            if (hit != NULL)
            {
                bool blocked = hit->blocksMovement(); // read before interact()
                hit->interact();                      // polymorphism: attack OR pick up
                if (blocked)
                    return; // fighting: stay in place
            }
            else if (!dungeon.isWalkable(target.getX(), target.getY()))
                throw MoveError("You bump into a wall.");
            player.moveBy(Vec2(dx, dy));
            if (dungeon.tile(player.getX(), player.getY()) == Dungeon::STAIRS)
                descend();
        }
        void enemiesAct()
        {
            for (size_t i = 0; i < enemies.size(); i++)
                if (enemies[i]->isActive())
                    enemies[i]->takeTurn(*this); // late binding
        }
        void saveGame()
        {
            SaveData s;
            memset(&s, 0, sizeof(s));
            s.version = 1;
            s.depth = player.getDepth();
            s.hp = player.getHp();
            s.maxHp = player.getMaxHp();
            s.attack = player.getAttack();
            s.gold = player.getGold();
            s.kills = player.getKills();
            SaveManager::saveBinary("savegame.dat", s);
            MessageLog::add("Game saved to savegame.dat (carried potions are not saved).");
        }
        void loadGame()
        {
            SaveData s = SaveManager::loadBinary("savegame.dat");
            player.restore(s);
            newLevel();
            MessageLog::add("Game loaded: depth " + toText(s.depth) + ".");
        }
        // Drawing (never clears the screen)
        void render() const
        {
            cout << "\n"
                 << util::banner(GAME_TITLE, 40) << "\n";
            cout << "Depth: " << player.getDepth() << "   Kills: " << player.getKills() << "   Gold: " << player.getGold() << "\n";
            int bars = util::clampTo(player.getHp() * 20 / player.getMaxHp(), 0, 20);
            cout << player << "  [";
            for (int i = 0; i < 20; i++)
                cout << (i < bars ? '#' : '-');
            cout << "]" << (player.isPoisoned() ? " POISONED" : "") << "\n";
            cout << "Pack: " << player.getPack() << "   Enemies left: "
                 << count_if(enemies.begin(), enemies.end(), IsActive()) << "\n\n";
            int px = player.getX(), py = player.getY();
            for (int y = 0; y < Dungeon::H; y++)
            {
                for (int x = 0; x < Dungeon::W; x++)
                {
                    char c = ' ';
                    if (dungeon.wasSeen(x, y))
                    {
                        int t = dungeon.tile(x, y);
                        if (t == Dungeon::WALL)
                            c = dungeon.isWallEdge(x, y) ? '#' : ' ';
                        else
                            c = (t == Dungeon::STAIRS) ? '>' : '.';
                    }
                    if (Dungeon::isVisible(x, y, px, py))
                    {
                        Item *it = findActive(items, x, y);
                        if (it != NULL)
                            c = it->getSymbol();
                        Enemy *en = findActive(enemies, x, y);
                        if (en != NULL)
                            c = en->getSymbol();
                    }
                    if (x == px && y == py)
                        c = player.getSymbol();
                    cout << c;
                }
                cout << "\n";
            }
            cout << "\n"
                 << MessageLog::get() << "\n";
            cout << "w/a/s/d move, x rest, e drink, v save, l load, i info, q quit\n> " << flush;
        }
        void showInfo() const
        {
            cout << "\n"
                 << util::banner(" INFO ", 40) << "\n";
            cout << "Live entities (static counter): " << Entity::getLiveCount() << "\n";
            cout << "Dice rolled so far (static counter): " << Dice::getRolls() << "\n";
            cout << "You are at " << player.getPos() << ", entity id #" << player.getId() << "\n";
            cout << "Pack " << player.getPack() << ", snapshot at last stairs " << snapshot << "\n";
            Backpack copy(player.getPack()); // copy constructor
            if (!copy.empty())
                copy.take();
            cout << "Deep-copy test: copy holds " << copy.size() << ", original still holds " << player.getPack().size() << "\n";
            cout << "Enemies in view (describe() is a virtual call):\n";
            int shown = 0;
            for (vector<Enemy *>::const_iterator it = enemies.begin(); it != enemies.end(); ++it)
            {
                const Enemy *e = *it;
                if (!e->isActive() || !Dungeon::isVisible(e->getX(), e->getY(), player.getX(), player.getY()))
                    continue;
                cout << "  " << e->getPos() << " " << e->describe();
                if (dynamic_cast<const Flying *>(e) != NULL)
                    cout << " [flies]"; // cross-cast via RTTI
                if (dynamic_cast<const Poisonous *>(e) != NULL)
                    cout << " [venomous]";
                cout << "\n";
                shown++;
            }
            if (shown == 0)
                cout << "  (none)\n";
            DetailedReport dr("depth " + toText(player.getDepth()));
            cout << "Slicing demo: by value -> " << reportByValue(dr) << " | by reference -> " << reportByReference(dr) << "\n";
            cout << "Recent constructor/destructor trace (oldest first):\n";
            const vector<string> &ev = Trace::get();
            size_t start = ev.size() > 14 ? ev.size() - 14 : 0;
            for (size_t i = start; i < ev.size(); i++)
                cout << "  " << ev[i] << "\n";
        }
        void endScreen()
        {
            cout << "\n"
                 << util::banner(" GAME OVER ", 40) << "\n";
            if (!player.isAlive())
                cout << "*** YOU DIED ***\n";
            int kills = player.getKills(), gold = player.getGold();
            cout << "Final score -> Depth: " << player.getDepth() << "  Kills: " << kills << "  Gold: " << gold << "\n";
            cout << "Kills by type:";
            const map<string, int> &k = player.getKillsByType();
            if (k.empty())
                cout << " none";
            for (map<string, int>::const_iterator it = k.begin(); it != k.end(); ++it)
                cout << "  " << it->first << " x" << it->second;
            cout << "\n";
            LevelRecord now = {player.getDepth(), kills, gold};
            history.push(now);
            cout << "Level history (stack, deepest first):\n";
            while (!history.empty())
            {
                const LevelRecord &r = history.top();
                cout << "  depth " << r.depth << ": kills " << r.kills << ", gold " << r.gold << "\n";
                history.pop();
            }
            try
            {
                ScoreEntry me = {player.getDepth(), kills, gold};
                SaveManager::appendScore("scores.txt", me);
                vector<ScoreEntry> all = SaveManager::loadScores("scores.txt");
                sort(all.begin(), all.end(), greater<ScoreEntry>());
                cout << "High scores (scores.txt, " << all.size() << " runs, total "
                     << accumulate(all.begin(), all.end(), 0, SumPoints()) << " points):\n";
                int rank = 1;
                for (vector<ScoreEntry>::const_iterator it = all.begin(); it != all.end() && rank <= 5; ++it, ++rank)
                    cout << "  " << rank << ". " << it->points() << " pts (depth " << it->depth
                         << ", kills " << it->kills << ", gold " << it->gold << ")\n";
            }
            catch (const FileError &e)
            {
                cout << "Could not update high scores: " << e.what() << "\n";
            }
        }

    public:
        Game() : running(true)
        {
            newLevel();
            MessageLog::add("Welcome, adventurer! Find the stairs (>). Type i for info.");
        }
        ~Game() { clearEntities(); }
        void run()
        {
            while (running && player.isAlive())
            {
                render();
                string line;
                if (!getline(cin, line))
                    break; // input closed
                MessageLog::clear();
                line = util::lower(line);
                // every character is its own command, so "dddd" moves 4 steps right
                for (size_t k = 0; k < line.size() && running && player.isAlive(); k++)
                {
                    char c = line[k];
                    if (c == ' ')
                        continue;
                    try
                    {
                        if (c == 'q')
                        {
                            running = false;
                            break;
                        }
                        if (c == 'i')
                        {
                            showInfo();
                            continue;
                        }
                        if (c == 'v')
                        {
                            saveGame();
                            continue;
                        }
                        if (c == 'l')
                        {
                            loadGame();
                            continue;
                        }
                        if (c == 'e')
                            player.drinkPotion(); // uses a turn
                        else
                        {
                            int dx = (c == 'd') - (c == 'a');
                            int dy = (c == 's') - (c == 'w');
                            if (dx == 0 && dy == 0 && c != 'x')
                            {
                                MessageLog::add("Unknown key.");
                                continue;
                            }
                            playerAct(dx, dy);
                        }
                    }
                    // a failed command (wall, empty pack, file problem) uses no turn and stops the rest of the line
                    catch (const MoveError &e)
                    {
                        MessageLog::add(e.what());
                        break;
                    }
                    catch (const BackpackError &e)
                    {
                        MessageLog::add(e.what());
                        break;
                    }
                    catch (const FileError &e)
                    {
                        MessageLog::add(string("File problem: ") + e.what());
                        break;
                    }
                    catch (const GameError &e)
                    {
                        MessageLog::add(string("Error: ") + e.what());
                        break;
                    }
                    purgeInactive(enemies);
                    purgeInactive(items);
                    enemiesAct();
                    player.tickStatus();
                    dungeon.updateSeen(player.getX(), player.getY());
                }
            }
            render();
            endScreen();
        }
    };
} // namespace rl
int main()
{
    srand((unsigned)time(NULL));
    try
    {
        rl::Game game;
        game.run();
    } // Game destroyed here: destructors free everything
    catch (const exception &e)
    {
        cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    cout << "\nEntities still alive after cleanup: " << rl::Entity::getLiveCount() << "\n";
    return 0;
}