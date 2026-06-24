#pragma once

#include <string>

class GameState {
public:
    void setup(int totalTargets);
    bool update(int defeatedTargets);

    int defeatedTargets() const;
    int totalTargets() const;
    bool cleared() const;
    std::string title() const;

private:
    int defeated = 0;
    int total = 0;
};
