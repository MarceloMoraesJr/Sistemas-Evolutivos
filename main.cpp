#include <algorithm>
#include <vector>
#include <random>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>

#define GENERATIONS_NUM 3
#define POP_SIZE 5
#define ENEMY_POP_SIZE 100
#define TOTAL_STAT_POINTS 200

#define ATTRIB_NUM 5      // Mudar aqui caso adicione/remova um atributo
#define BASE_ATK 40
#define BASE_DEF 10
#define BASE_HP  600
#define BASE_HP_REGEN 200
#define BASE_SPD 10

#define MAX_DEF (BASE_DEF + TOTAL_STAT_POINTS)  // defesa máxima de uma entidade
#define HP_MODIFIER 5    // quanto de vida a entidade ganha ao gastar um ponto em hp máximo
#define ATK_MODIFIER 1
#define HP_REGEN_MODIFIER 3

#define DEBUG_MODE false

using namespace std;

// Taxa de mutação inicial
float MUT_RATE = 0.1;

enum AttribIndex {
    ATK,
    DEF,
    HP_MAX,
    HP_REGEN,
    SPD
};

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
        vector<int> points;

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
            this->atk += points[AttribIndex::ATK];
            this->def += points[AttribIndex::DEF];
            this->hp_max  += HP_MODIFIER*points[AttribIndex::HP_MAX];
            this->hp_current = this->hp_max;
            this->hp_regen += points[AttribIndex::HP_REGEN];
            this->spd += points[AttribIndex::SPD];

            this->points = points;
        }

        bool IsDead(){
            return this->hp_current <= 0;
        }

        void TakeDamage(int dmg){
            // Reduz dano tomado em função da defesa
            // até um máximo de 60% de redução de dano
            hp_current -= dmg * (1 - (0.6/MAX_DEF)*def);
        }

        void RegenerateHP(){
            this->hp_current += HP_REGEN_MODIFIER * this->hp_regen; 
            if(this->hp_current > this->hp_max){
                this->hp_current = this->hp_max;
            }
        }

        void Crossover(Entity other){
            this->points[AttribIndex::ATK] = (this->points[AttribIndex::ATK] + other.points[AttribIndex::ATK])/2;
            this->points[AttribIndex::DEF] = (this->points[AttribIndex::DEF] + other.points[AttribIndex::DEF])/2;
            this->points[AttribIndex::HP_MAX] = (this->points[AttribIndex::HP_MAX] + other.points[AttribIndex::HP_MAX])/2;
            this->points[AttribIndex::HP_REGEN] = (this->points[AttribIndex::HP_REGEN] + other.points[AttribIndex::HP_REGEN])/2;
            this->points[AttribIndex::SPD] = (this->points[AttribIndex::SPD] + other.points[AttribIndex::SPD])/2;

            int sum = 0;
            for(int i=0; i<ATTRIB_NUM; i++){
                sum += this->points[i];
            }
            this->points[rand()%ATTRIB_NUM] += TOTAL_STAT_POINTS - sum;

            // Mutação
            int mutDecStat, mutIncStat;            
            int mutDecValue, mutIncValue;

            mutDecValue = mutIncValue = (int) (MUT_RATE * ((double) (rand()%TOTAL_STAT_POINTS)-TOTAL_STAT_POINTS/2));

            mutDecStat = mutIncStat = rand()%ATTRIB_NUM;
            while(mutDecStat == mutIncStat)
                mutDecStat = rand()%ATTRIB_NUM;
            
            for(int i=mutIncStat; mutIncValue > 0; i=(i+1)%ATTRIB_NUM){
                this->points[i] += mutIncValue;
                mutIncValue = 0;
                if(this->points[i] > TOTAL_STAT_POINTS){
                    mutIncValue = this->points[i] - TOTAL_STAT_POINTS;
                    this->points[i] = TOTAL_STAT_POINTS;
                }
            }

            for(int i=mutDecStat; mutDecValue > 0; i=(i+1)%ATTRIB_NUM){
                this->points[i] -= mutDecValue;
                mutDecValue = 0;
                if(this->points[i] < 0){
                    mutDecValue = -this->points[i];
                    this->points[i] = 0;
                }
            }

            // Calcula status
            this->atk = BASE_ATK + this->points[AttribIndex::ATK];
            this->def = BASE_DEF + this->points[AttribIndex::DEF];
            this->hp_max = BASE_HP + HP_MODIFIER*this->points[AttribIndex::HP_MAX];
            this->hp_current = this->hp_max;
            this->hp_regen = BASE_HP_REGEN + this->points[AttribIndex::HP_REGEN];
            this->spd = BASE_SPD + this->points[AttribIndex::SPD];
            this->score = 0;
        }

        void PrintEntity(){
            cout << "E" << this->index << endl;
            cout << "atk: " << this->atk << endl;
            cout << "def: " << this->def << endl;
            cout << "hp_max: " << this->hp_max << endl;
            cout << "hp_current: " << this->hp_current << endl;
            cout << "hp_regen: " << this->hp_regen << endl;
            cout << "spd: " << this->spd << endl;
            cout << endl;
            cout << "atk points: " << this->points[AttribIndex::ATK] << endl;
            cout << "def points: " << this->points[AttribIndex::DEF] << endl;
            cout << "hp_max points: " << this->points[AttribIndex::HP_MAX] << endl;
            cout << "hp_regen points: " << this->points[AttribIndex::HP_REGEN] << endl;
            cout << "spd points: " << this->points[AttribIndex::SPD] << endl;
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

        bool operator==(Entity other) const{
            return this->index == other.index;
        }
};

// Duelo entre individuo da população e inimigo
bool DuelToDeath(Entity *I, Entity *E, ofstream &battleLog){
    Entity *attacker, *defender, *aux, *winner, *loser,
            a, b;
    
    bool indWon = false;

    // if(DEBUG_MODE)
    //     battleLog << "----------------------------------------------------" << endl;
    
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
        int dmg = ATK_MODIFIER * attacker->atk;
        defender->TakeDamage(dmg);

        // if(DEBUG_MODE){
        //     battleLog << "E" << attacker->index << " ataca " << "E" << defender->index << endl;
        //     battleLog << "Vida de E" << defender->index << ": " << defender->hp_current << endl;
        //     battleLog << endl;
        // }

        // Regenera HP do atacante
        // attacker->RegenerateHP();

        // Troca quem ataca e quem defende
        aux = attacker;
        attacker = defender;
        defender = aux;
    }

    *I = a;

    indWon = !a.IsDead();

    // if(DEBUG_MODE){
    //     battleLog << "E" << winner->index << " venceu E" << loser->index << "!" << endl;
    //     battleLog << "----------------------------------------------------" << endl << endl;
    // }

    return indWon;
}

// Faz com que todos lutem contra todos e contabiliza o numero de
// vitórias de cada entidade
Entity *Evaluate(vector<Entity> &p, vector<Entity> &e, ofstream &battleLog){
    Entity *best, *I, *E;
    int topScore = 0;

    // Organiza lutas e armazena quem obteve mais vitórias
    for(int i=0; i<(int)p.size(); i++){
        bool isWinning = true;
        I = &(p[i]);
        I->score = 0;
        for(int j=0; j<ENEMY_POP_SIZE and isWinning; j++){
            E = &(e[j]);
            isWinning = DuelToDeath(I, E, battleLog);
            I->RegenerateHP();

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

void Crossover(vector<Entity> &p, Entity *best){
    for(int i=0; i<(int)p.size(); i++){
        if(p[i].index == (*best).index)
            continue;
        
        p[i].Crossover(*best);
    }
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
    Entity bestEver;
    Entity *bestCurrent;
    for(int i=0; i<GENERATIONS_NUM; i++){
        bestCurrent = Evaluate(population, enemies, battleLog);

        if(bestCurrent->score > bestEver.score){
            bestEver = *bestCurrent;
        }

        int sum = 0;
        for(int j=0; j<ATTRIB_NUM; j++){
            sum += population[0].points[j];
            cout << population[0].points[j] << " ";
        }
        cout << "sum = " << sum;
        cout << endl;

        Crossover(population, bestCurrent);

        sum = 0;
        for(int j=0; j<ATTRIB_NUM; j++){
            sum += population[0].points[j];
            cout << population[0].points[j] << " ";
        }
        cout << "sum = " << sum;
        cout << endl;
        cout << endl;
    }

    bestEver.PrintEntity();

    // Debug
    if(DEBUG_MODE){
        for(int i=0; i<(int)population.size(); i++){
            population[i].PrintEntity(battleLog);
        }

        battleLog << "Best Entity:" << endl;
        bestEver.PrintEntity(battleLog);
    }

    battleLog.close();

    return 0;
}