#include <algorithm>
#include <vector>
#include <random>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>

#define GENERATIONS_NUM 1000
#define POP_SIZE 50
#define ENEMY_POPS_COUNT 10
#define ENEMY_POP_SIZE 2000
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

#define BEST_IND_COUNT 5
#define MUT_RATE_INIT 0.002

#define DEBUG_MODE false

using namespace std;

// Taxa de mutação inicial
float MUT_RATE = MUT_RATE_INIT;

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
        float score;
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

        void CalculateStats(){
            this->atk = BASE_ATK + ATK_MODIFIER * this->points[AttribIndex::ATK];
            this->def = BASE_DEF + this->points[AttribIndex::DEF];
            this->hp_max  = BASE_HP + HP_MODIFIER * this->points[AttribIndex::HP_MAX];
            this->hp_current = this->hp_max;
            this->hp_regen = BASE_HP_REGEN + HP_REGEN_MODIFIER * this->points[AttribIndex::HP_REGEN];
            this->spd = BASE_SPD + this->points[AttribIndex::SPD];
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
            this->points = points;
            CalculateStats();
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
            this->hp_current += this->hp_regen; 
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

            // Conserta erro de arredondamento somando
            // os pontos faltantes em um atributo aleatorio
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
            CalculateStats();
            this->score = 0;
        }

        void PrintEntity(){
            cout << "E" << this->index << endl;
            // cout << "atk: " << this->atk << endl;
            // cout << "def: " << this->def << endl;
            // cout << "hp_max: " << this->hp_max << endl;
            // cout << "hp_current: " << this->hp_current << endl;
            // cout << "hp_regen: " << this->hp_regen << endl;
            // cout << "spd: " << this->spd << endl;
            // cout << endl;
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
float DuelToDeath(Entity *I, Entity *E, ofstream &battleLog){
    Entity *attacker, *defender, *aux, *winner, *loser,
            a, b;
    float turnCount = 0;

    // if(DEBUG_MODE)
    //     battleLog << "----------------------------------------------------" << endl;
    
    // Copia lutadores para variaveis auxiliares
    a = *I;
    b = *E;

    // Determina quem começa atacando
    // Se I for mais rápido, ataca primeiro
    attacker = &a;
    defender = &b;
    if(a.spd <= b.spd){
        attacker = &b;
        defender = &a;
    }
    // else if(rand()%2){
    //     attacker = &b;
    //     defender = &a;
    // }

    // Ambos lutam até a morte
    while(!a.IsDead() and !b.IsDead()){
        defender->TakeDamage(attacker->atk);

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

        turnCount++;
    }

    *I = a;

    if(a.IsDead()){
        turnCount = -1;
    }

    // if(DEBUG_MODE){
    //     battleLog << "E" << winner->index << " venceu E" << loser->index << "!" << endl;
    //     battleLog << "----------------------------------------------------" << endl << endl;
    // }

    return turnCount;
}

// Faz com que todos lutem contra todos e contabiliza o numero de
// vitórias de cada entidade
Entity *Evaluate(vector<Entity> &p, vector<vector<Entity>> &e, ofstream &battleLog){
    Entity *best, *I, *E;
    float topScore = 0, score, finalScore;
    bool isWinning;
    float turnCount;
    // best = &(p[0]);

    // Organiza lutas e armazena quem obteve mais vitórias
    for(int i=0; i<(int)p.size(); i++){
        I = &(p[i]);
        finalScore = 0;
        for(int j=0; j<ENEMY_POPS_COUNT; j++){
            isWinning = true;
            score = 0;
            for(int k=0; k<ENEMY_POP_SIZE and isWinning; k++){
                E = &(e[j][k]);
                turnCount = DuelToDeath(I, E, battleLog);
                I->RegenerateHP();

                if(turnCount > 0){
                    score += 10 + 8/turnCount;
                } else {
                    isWinning = false;
                }
            }

            finalScore += score;
            I->hp_current = I->hp_max;
        }

        I->score = finalScore/(ENEMY_POPS_COUNT);

        if(i == 0 or I->score > topScore){
            topScore = I->score;
            best = I;
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
        if(p[i].index == (*best).index){
            continue;
        }
        
        p[i].Crossover(*best);
    }
}

void Genocide(Entity &best, vector<Entity> &population){
    for(int i=0; i<POP_SIZE; i++){
        if(population[i].index != best.index){
            population[i].GenerateRandomStats();
        }
    }
}

void IncreaseMutationRate(vector<Entity> &bestEver, vector<Entity> &population){
    if(bestEver[BEST_IND_COUNT-1].score - bestEver[0].score < 0.1){
        MUT_RATE *= 1.1;
    } else {
        MUT_RATE = MUT_RATE_INIT;
    }

    if(MUT_RATE >= 1){
        Genocide(bestEver[BEST_IND_COUNT-1], population);
        MUT_RATE = MUT_RATE_INIT;
    }
}

int main(){
    srand(time(NULL));

    // Salva log de batalhas para debug
    ofstream battleLog("battleLog.txt", ofstream::out);
    ofstream fitnessData("fitness.dat", ofstream::out);

    vector<Entity> population(POP_SIZE);
    vector<vector<Entity>> enemies;
    vector<Entity> bestEver(BEST_IND_COUNT);
    Entity *bestCurrent;

    for(int i=0; i<ENEMY_POPS_COUNT; i++){
        vector<Entity> enemy_pop(ENEMY_POP_SIZE);
        enemies.push_back(enemy_pop);
    }

    // Gera entidades com status aleatorios
    // e armazena em si seu indice no vetor de população
    for(int i=0; i<POP_SIZE; i++){
        population[i].GenerateRandomStats();
        population[i].index = i;
    }

    // Inicializa vetor de melhores individuos
    for(int i=0; i<BEST_IND_COUNT; i++){
        bestEver[i] = population[0];
    }

    // Gera filas de inimigos
    for(int i=0; i<ENEMY_POPS_COUNT; i++){
        for(int j=0; j<ENEMY_POP_SIZE; j++){
            enemies[i][j].GenerateRandomStats();
            enemies[i][j].index = i;
            enemies[i][j].hp_regen = 0;
        }
    }

    // Modo de operacao
    int aux;
    cout << "1: Auto" << endl << "2: Interactive" << endl;
    cin >> aux;

    if(aux == 1){
        // Avalia as entidades
        for(int i=0; i<GENERATIONS_NUM; i++){
            bestCurrent = Evaluate(population, enemies, battleLog);

             // Guarda o melhor individuo das ultimas gerações
            for(int i=1; i<BEST_IND_COUNT; i++){
                bestEver[i-1] = bestEver[i];
            }

            bestEver[BEST_IND_COUNT-1] = *bestCurrent;            

            // Salva fitness do melhor da geração no arquivo
            fitnessData << i << " " << bestCurrent->score << endl;

            Crossover(population, bestCurrent);

            IncreaseMutationRate(bestEver, population);
        }
    } else if(aux == 2){
        int generations = 1;
        cout << "Gerações por passo: ";
        cin >> generations;        
        do {
            for(int i=0; i<generations; i++) {
                bestCurrent = Evaluate(population, enemies, battleLog);

                // Guarda o melhor individuo das ultimas gerações
                for(int i=1; i<BEST_IND_COUNT; i++){
                    bestEver[i-1] = bestEver[i];
                }

                bestEver[BEST_IND_COUNT-1] = *bestCurrent;

                Crossover(population, bestCurrent);

                IncreaseMutationRate(bestEver, population);
            }

            bestCurrent->PrintEntity();
            cout << "taxa de mutação: " << MUT_RATE << endl;

        } while(cin.get() == '\n');
    }

    // Ola

    bestEver[BEST_IND_COUNT-1].PrintEntity();

    // Debug
    if(DEBUG_MODE){
        for(int i=0; i<(int)population.size(); i++){
            population[i].PrintEntity(battleLog);
        }

        battleLog << "Best Entity:" << endl;
        bestEver[BEST_IND_COUNT-1].PrintEntity(battleLog);
    }

    battleLog.close();
    fitnessData.close();

    return 0;
}