#include <algorithm>
#include <vector>
#include <random>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>

#define POP_SIZE 15
#define ENEMY_POP_SIZE 10
#define TOTAL_STAT_POINTS 200

#define ATTRIB_NUM 5      // Mudar aqui caso adicione/remova um atributo
#define BASE_ATK 10
#define BASE_DEF 10
#define BASE_HP  500
#define BASE_HP_REGEN 5
#define BASE_SPD 10

#define MAX_DEF (BASE_DEF + TOTAL_STAT_POINTS)  // defesa máxima de uma ent
#define HP_MODIFIER 5    // quanto de vida a entidade ganha ao gastar um ponto em hp

#define DEBUG_MODE false

using namespace std;


class Entity {
    public:

        int atk;
        int def;
        int hp_max;
        int hp_current;
        int hp_regen;
        int spd;
        int score;
        int index;

        Entity(){
            // Status base
            atk = BASE_ATK;
            def = BASE_DEF;
            hp_max = hp_current = BASE_HP;
            hp_regen = BASE_HP_REGEN;
            spd = BASE_SPD;
            score = 0;
            index = -1;
        }

        void GenerateRandomStats(){
            vector<int> aux, points(ATTRIB_NUM, 0);

            // Escolhe aleatoriamente a qtd de pontos para cada atributo
            // gerando 4 numeros aleatorios entre 0 e TOTAL_STAT_POINTS,
            // ordenando-os e utilizando os intervalos entre eles
            aux.push_back(0);
            for(int i=0; i<ATTRIB_NUM-1; i++){
                aux.push_back((int)rand()%(TOTAL_STAT_POINTS+1));
            }
            aux.push_back(TOTAL_STAT_POINTS);

            sort(aux.begin(), aux.end());

            for(int i=0; i<ATTRIB_NUM; i++){
                points[i] = aux[i+1] - aux[i];
            }

            // Adiciona os pontos a seus respectivos atributos
            atk += points[0];
            def += points[1];
            hp_max  += HP_MODIFIER*points[2];
            hp_current = hp_max;
            hp_regen += points[3];
            spd += points[4];
        }

        bool IsDead(){
            return hp_current <= 0;
        }

        void TakeDamage(int dmg){
            // Reduz dano tomado em função da defesa
            // até um máximo de 50% de redução de dano
            hp_current -= dmg * (1 - (0.6/MAX_DEF)*def);
        }

        void RegenerateHP(){
            this->hp_current += this->hp_regen; 
            if(this->hp_current > this->hp_max){
                this->hp_current = this->hp_max;
            }
        }

        void PrintEntity(){
            cout << "E" << this->index << endl;
            cout << "atk: " << this->atk << endl;
            cout << "def: " << this->def << endl;
            cout << "max_hp: " << this->hp_max << endl;
            cout << "hp_current: " << this->hp_current << endl;
            cout << "hp_regen: " << this->hp_regen << endl;
            cout << "spd: " << this->spd << endl;
            cout << "score: " << this->score << endl;
            cout << endl;
        }

        void PrintEntity(ofstream &battleLog){
            battleLog << "E" << this->index << endl;
            battleLog << "atk: " << this->atk << endl;
            battleLog << "def: " << this->def << endl;
            battleLog << "hp_max: " << this->hp_max << endl;
            battleLog << "hp_current: " << this->hp_current << endl;
            battleLog << "hp_regen: " << this->hp_regen << endl;
            battleLog << "spd: " << this->spd << endl;
            battleLog << "score: " << this->score << endl;
            battleLog << endl;
        }
};


// // Duelo entre 2 entidades
// Entity* Duel(Entity *A, Entity *B, ofstream &battleLog){
//     Entity *attacker, *defender, *aux, *winner, *loser,
//             a, b;

//     if(DEBUG_MODE)
//         battleLog << "----------------------------------------------------" << endl;
    
//     // Copia entidades para variáveis auxiliares
//     a = *A;
//     b = *B;

//     // Determina quem começa atacando
//     // Se a for mais rápida, ataca primeiro
//     attacker = &a;
//     defender = &b;
//     if(a.spd < b.spd){
//         attacker = &b;
//         defender = &a;
//     }
//     else if(rand()%2){
//         attacker = &b;
//         defender = &a;
//     }

//     // Ambos lutam até a morte
//     while(!a.IsDead() and !b.IsDead()){
//         float dmg = attacker->atk;
//         defender->TakeDamage(dmg);

//         if(DEBUG_MODE){
//             battleLog << "E" << attacker->index << " ataca " << "E" << defender->index << endl;
//             battleLog << "Vida de E" << defender->index << ": " << defender->hp << endl;
//             battleLog << endl;
//         }

//         // Troca quem ataca e quem defende
//         aux = attacker;
//         attacker = defender;
//         defender = aux;
//     }

//     // Checa quem venceu
//     if(a.IsDead()){
//         winner = B;
//         loser = A;
//     } else {
//         winner = A;
//         loser = B;
//     }

//     if(DEBUG_MODE){
//         battleLog << "E" << winner->index << " venceu E" << loser->index << "!" << endl;
//         battleLog << "----------------------------------------------------" << endl << endl;
//     }

//     return winner;
// }

// Duelo entre individuo da população e inimigo
bool DuelToDeath(Entity *I, Entity *E, ofstream &battleLog){
    Entity *attacker, *defender, *aux, *winner, *loser,
            a, b;
    
    bool indWon = false;

    if(DEBUG_MODE)
        battleLog << "----------------------------------------------------" << endl;
    
    // Copia lutadores para variaveis auxiliares
    a = *I;
    b = *E;

    // Determina quem começa atacando
    // Se I for mais rápido, ataca primeiro
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
        int dmg = attacker->atk;
        defender->TakeDamage(dmg);

        if(DEBUG_MODE){
            battleLog << "E" << attacker->index << " ataca " << "E" << defender->index << endl;
            battleLog << "Vida de E" << defender->index << ": " << defender->hp_current << endl;
            battleLog << endl;
        }

        // Regenera HP do atacante
        attacker->RegenerateHP();

        // Troca quem ataca e quem defende
        aux = attacker;
        attacker = defender;
        defender = aux;
    }

    *I = a;

    indWon = !a.IsDead();

    if(DEBUG_MODE){
        battleLog << "E" << winner->index << " venceu E" << loser->index << "!" << endl;
        battleLog << "----------------------------------------------------" << endl << endl;
    }

    return indWon;
}

// Faz com que todos lutem contra todos e contabiliza o numero de
// vitórias de cada entidade
Entity *Evaluate(vector<Entity> &p, vector<Entity> &e, ofstream &battleLog){
    Entity *best, *I, *E;
    int topScore = 0;

    // // Matriz que guarda quem venceu cada luta
    // int fightWinner[(int)p.size()][(int)p.size()];
    // for(int i=0; i<(int)p.size(); i++){
    //     for(int j=0; j<(int)p.size(); j++){
    //         fightWinner[i][j] = 0;
    //     }
    // }

    // Organiza lutas e armazena quem obteve mais vitórias
    for(int i=0; i<(int)p.size(); i++){
        bool isWinning = true;
        I = &(p[i]);
        for(int j=0; j<ENEMY_POP_SIZE and isWinning; j++){
            E = &(e[j]);
            isWinning = DuelToDeath(I, E, battleLog);

            if(isWinning){
                I->score++;
            }

            if(I->score > topScore){
                topScore = I->score;
                best = I;
            }

            //fightWinner[i][j] = winner->index;
        }
    }

    // if(DEBUG_MODE){
    //     battleLog << "Battle Winners: " << endl;
    //     for(int i=0; i<(int)p.size(); i++){
    //         battleLog << "E" << i << ": ";
    //         for(int j=0; j<i; j++){
    //             battleLog << "E" << fightWinner[i][j] << " ";
    //         }
    //         battleLog << endl;
    //     }
    //     battleLog << "   ";
    //     for(int i=0; i<(int)p.size(); i++){
    //         battleLog << " E" << i;
    //     }
    //     battleLog << endl << endl;
    // }

    return best;
}

int main(){
    srand(time(NULL));

    // Salva log de batalhas para debug
    ofstream battleLog("battleLog.txt", ofstream::out);

    vector<Entity> population(POP_SIZE);
    vector<Entity> enemies(ENEMY_POP_SIZE);

    // Gera entidades com status aleatorios
    // e armazena em si seu indice no vetor de população
    for(int i=0; i<POP_SIZE; i++){
        population[i].GenerateRandomStats();
        population[i].index = i;
    }

    for(int i=0; i<ENEMY_POP_SIZE; i++){
        enemies[i].GenerateRandomStats();
        enemies[i].index = i;
        enemies[i].hp_regen = 0;
    }

    // Avalia as entidades
    Entity *best = Evaluate(population, enemies, battleLog);

    best->PrintEntity();

    // Debug
    if(DEBUG_MODE){
        for(int i=0; i<(int)population.size(); i++){
            population[i].PrintEntity(battleLog);
        }

        battleLog << "Best Entity:" << endl;
        best->PrintEntity(battleLog);
    }

    battleLog.close();

    return 0;
}