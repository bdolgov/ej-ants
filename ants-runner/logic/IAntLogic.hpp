#ifndef IANTLOGIC_HPP
#define IANTLOGIC_HPP

#include <string>
#include <stack>
#include <memory>

namespace antlogic
{

    const int MAX_MEMORY = 32; // размер памяти вашего муравья

    struct AntSensor {
        int smell = 0; // оттенок запаха
        int smellIntensity = 0; // интенсивность запаха
        bool isFriend = false; // наличие "своего" муравья
        bool isEnemy = false; // наличие "чужого" муравья
        bool isMyHill = false; // наличие "своего" муравейника
        bool isEnemyHill = false; // наличие "своего" муравейника
        bool isFood = false; // наличие еды
        bool isWall = false; // наличие стены
    };

    // Все возможные действия муравья
    enum AntActionType {
        // передвижение:
        MOVE_UP = 0,
        MOVE_LEFT = 1,
        MOVE_DOWN = 2,
        MOVE_RIGHT = 3,
        // кусание:
        BITE_UP = 4,
        BITE_LEFT = 5,
        BITE_DOWN = 6,
        BITE_RIGHT = 7,
        // манипуляция с едой:
        GET = 8,
        PUT = 9
    };

    // Действие муравья на текущем ходу
    struct AntAction
    {
        AntActionType actionType = MOVE_UP;
        bool putSmell = false; // оставить ли запах на клетке
        int smell = 0; // оттенок оставляемого муравьём запаха
    };

    class Ant
    {
    public:
        virtual char* getMemory() const = 0;
        virtual bool hasFood() const = 0;
        virtual int getTeamId() const = 0;
        virtual ~Ant() {}
    };

    class IAntLogic
    {
    public:
        static std::shared_ptr<IAntLogic> GetAntLogic(int teamId);
        static std::shared_ptr<IAntLogic> GetAntLogic(std::string fileName, std::string id);
        virtual AntAction GetAction(const Ant&, AntSensor[3][3]) = 0;
        virtual ~IAntLogic() {}
    };
}
#endif
