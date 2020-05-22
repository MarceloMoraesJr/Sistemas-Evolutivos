#include <algorithm>
#include <vector>
#include <random>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>

#define POP_SIZE 15
#define TOTAL_STAT_POINTS 200
#define BASE_ATK 10.0
#define BASE_DEF 10.0
#define BASE_HP  500.0
#define BASE_SPD 10.0

#define MAX_DEF (BASE_DEF + TOTAL_STAT_POINTS)  // defesa máxima de uma ent
#define HP_MODIFIER 5    // quanto de vida a entidade ganha ao gastar um ponto em hp

using namespace std;

class Entity {
    public:

    float atk;
    float def;
    float hp;
    float spd;
    int score;
    int index;

    Entity(){
        // Status base
        atk = BASE_ATK;
        def = BASE_DEF;
        hp = BASE_HP;
        spd = BASE_SPD;
        score = 0;
        index = -1;
    }

    bool IsDead(){
        return hp <= 0;
    }

    void TakeDamage(float dmg){
        // Reduz dano tomado em função da defesa
        // até um máximo de 50% de redução de dano
        hp -= dmg * (1 - (0.5/MAX_DEF)*def);
    }
};

void PrintEntity(Entity e){
    cout << "E" << e.index << endl;
    cout << "atk: " << e.atk << endl;
    cout << "def: " << e.def << endl;
    cout << "hp: " << e.hp << endl;
    cout << "spd: " << e.spd << endl;
    cout << "score: " << e.score << endl;
    cout << endl;
}

void PrintEntity(Entity e, ofstream &battleLog){
    battleLog << "E" << e.index << endl;
    battleLog << "atk: " << e.atk << endl;
    battleLog << "def: " << e.def << endl;
    battleLog << "hp: " << e.hp << endl;
    battleLog << "spd: " << e.spd << endl;
    battleLog << "score: " << e.score << endl;
    battleLog << endl;
}

// Distribui os pontos entre os atributos da entidade
void Distribute(int total, Entity &e){
    int aux, points = total;
    vector<float> attribs(4, 0);

    // Escolhe aleatoriamente a qtd de pontos para cada atributo
    for(int i=0; i<3 and points > 0; i++){
        aux = ((int)rand() % points)+1;
        attribs[i] = (float) aux;
        points -= aux;
    }
    // Usa os pontos restantes
    attribs[3] = (float) points;

    // Adiciona os pontos a seus respectivos atributos
    e.atk += attribs[0];
    e.def += attribs[1];
    e.hp += HP_MODIFIER*attribs[2];
    e.spd += attribs[3];
}

// Distribui pontos nos atributos de cada entidade
// e armazena em si seu indice no vetor de população
void InitializePop(vector<Entity> &p){    
    for(int i=0; i<POP_SIZE; i++){
        Distribute(TOTAL_STAT_POINTS, p[i]);
        p[i].index = i;
    }
}

// Duelo entre 2 entidades
Entity* Duel(Entity *A, Entity *B, ofstream &battleLog){
    Entity *attacker, *defender, *aux, *winner, *loser,
            a, b;

    battleLog << "----------------------------------------------------" << endl;
    // Copia entidades para variáveis auxiliares
    a = *A;
    b = *B;

    // Determina quem começa atacando
    // Se a for mais rápida, ataca primeiro
    attacker = &a;
    defender = &b;
    if(a.spd < b.spd){
        attacker = &b;
        defender = &a;
    }
    else if(rand()%2){
        attacker = &b;
        defender = &a;
    }

    // Ambos lutam até a morte
    while(!a.IsDead() and !b.IsDead()){
        float dmg = attacker->atk;
        defender->TakeDamage(dmg);

        battleLog << "E" << attacker->index << " ataca " << "E" << defender->index << endl;
        battleLog << "Vida de E" << defender->index << ": " << defender->hp << endl;
        battleLog << endl;

        // Troca quem ataca e quem defende
        aux = attacker;
        attacker = defender;
        defender = aux;
    }

    // Checa quem venceu
    if(a.IsDead()){
        winner = B;
        loser = A;
    } else {
        winner = A;
        loser = B;
    }

    battleLog << "E" << winner->index << " venceu E" << loser->index << "!" << endl;
    battleLog << "----------------------------------------------------" << endl << endl;
    return winner;
}

// Faz com que todos lutem contra todas e contabiliza o numero de
// vitórias de cada entidade
Entity *Evaluate(vector<Entity> &p, ofstream &battleLog){
    Entity *winner, *best, *A, *B;
    int topScore = 0;

    // Matriz que guarda quem venceu cada luta
    int fightWinner[(int)p.size()][(int)p.size()];
    for(int i=0; i<(int)p.size(); i++){
        for(int j=0; j<(int)p.size(); j++){
            fightWinner[i][j] = 0;
        }
    }

    // Organiza lutas e armazena quem obteve mais vitórias
    for(int i=0; i<(int)p.size(); i++){
        A = &(p[i]);
        for(int j=0; j<i; j++){
            B = &(p[j]);
            winner = Duel(A, B, battleLog);
            (*winner).score++;

            if((*winner).score > topScore){
                topScore = (*winner).score;
                best = winner;
            }

            fightWinner[i][j] = winner->index;
        }
    }

    // Debug
    battleLog << "Battle Winners: " << endl;
    for(int i=0; i<(int)p.size(); i++){
        battleLog << "E" << i << ": ";
        for(int j=0; j<i; j++){
            battleLog << "E" << fightWinner[i][j] << " ";
        }
        battleLog << endl;
    }
    battleLog << "   ";
    for(int i=0; i<(int)p.size(); i++){
        battleLog << " E" << i;
    }
    battleLog << endl << endl;

    return best;
}

int main(){
    srand(time(NULL));

    // Salva log de batalhas para debug
    ofstream battleLog("battleLog.txt", ofstream::out);

    vector<Entity> population(POP_SIZE);

    // Gera entidades com status aleatorios
    InitializePop(population);

    // Avalia as entidades
    Entity *best = Evaluate(population, battleLog);

    PrintEntity(*best);

    // Debug
    for(int i=0; i<(int)population.size(); i++){
        PrintEntity(population[i], battleLog);
    }

    battleLog << "Best Entity:" << endl;
    PrintEntity(*best, battleLog);

    battleLog.close();

    return 0;
}