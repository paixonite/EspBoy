#ifndef GAME_H
#define GAME_H

class Game {
public:
    // Estas são funções virtuais puras. O '= 0' significa que a classe
    // base não as implementa, mas obriga qualquer classe filha a implementá-las.
    
    virtual void setup() = 0;
    virtual void loop() = 0;
    virtual bool shouldExit() const = 0;

    // --- Destrutor Virtual ---
    // Garante que, ao deletar um jogo através de um ponteiro Game*,
    // o destrutor correto (de SnakeGame, TetrisGame, etc.) seja chamado.
    // Isso evita vazamentos de memória.
    virtual ~Game() {}
};

#endif // GAME_H