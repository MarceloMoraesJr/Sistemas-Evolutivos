#include <algorithm>
#include <vector>
#include <random>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

//Para realizar o fork e executar o processo do Gnuplot a fim de plotar o grafico
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

// SFML (graficos)
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#define GENERATIONS_NUM 100
#define POP_SIZE 50
#define ENEMY_POPS_COUNT 10
#define ENEMY_POP_SIZE 2000
#define TOTAL_STAT_POINTS 300

#define ATTRIB_NUM 5      // Mudar aqui caso adicione/remova um atributo

#define BASE_ATK 30
#define BASE_DEF 10
#define BASE_HP  900
#define BASE_HP_REGEN 450
#define BASE_SPD 10

// Modificadores: quanto de um atributo a entidade ganha por ponto gasto
#define ATK_MODIFIER 0.8
#define DEF_MODIFIER 3
#define HP_MODIFIER 10
#define HP_REGEN_MODIFIER 5

#define MAX_DEF (BASE_DEF + DEF_MODIFIER * TOTAL_STAT_POINTS)  // defesa máxima que uma entidade pode ter

#define BEST_IND_COUNT 5
#define MUT_RATE_INIT 0.02

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
            this->def = BASE_DEF + DEF_MODIFIER * this->points[AttribIndex::DEF];
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
            hp_current -= dmg * (1 - (0.90/MAX_DEF)*def);
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
                    score += 10 + 5/turnCount;
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

//Estados da MEF da luta
enum State{
    INITIALIZING,
    PLAYERS_TURN,
    ENEMYS_TURN,
    END
};

enum EntityType {
    PLAYER,
    ENEMY
};

class GUIEntity {
    public:
    Entity entity;

    sf::Text name;
    sf::Text HPText;
    sf::RectangleShape maxHPBar;
    sf::RectangleShape currentHPBar;
    sf::Sprite sprite;
    sf::Text statsText;
    sf::Text damageTakenText;

    GUIEntity(Entity e, EntityType entityType, sf::Font &font) :
                name(),
                HPText(),
                maxHPBar(),
                currentHPBar(),
                sprite(),
                statsText(),
                damageTakenText()
    {
        sf::Texture texture;
        entity = e;
        if(entityType == EntityType::PLAYER){
            name.setString("player");
            texture.loadFromFile("sprites/player.png");
            sprite.setTexture(texture);

            name.setPosition(195.82, 138.32);
            sprite.setPosition(200, 256);
            HPText.setPosition(204.66, 187.48);
            maxHPBar.setPosition(178, 213);
            currentHPBar.setPosition(180, 215);
            statsText.setPosition(193.82, 367.32);
            damageTakenText.setPosition(214.82, 113.32);
        } else if(entityType == EntityType::ENEMY){
            name.setString("enemy");
            texture.loadFromFile("sprites/enemy.png");
            sprite.setTexture(texture);

            name.setPosition(538.82, 138.32);
            HPText.setPosition(538.66, 187.48);
            maxHPBar.setPosition(513, 213);
            currentHPBar.setPosition(515, 215);
            sprite.setPosition(535, 256);
            statsText.setPosition(528.82, 367.32);
            damageTakenText.setPosition(550, 113.32);
        }        
        name.setFont(font);
        name.setCharacterSize(30);

        name.setFillColor(sf::Color(254, 194, 32, 255));
        name.setOutlineColor(sf::Color(255, 255, 255, 0));
        name.setOutlineThickness(3);

        HPText.setString(to_string(entity.hp_current) + "/" + to_string(entity.hp_max));
        HPText.setFont(font);
        HPText.setCharacterSize(20);

        maxHPBar.setSize(sf::Vector2f(144, 19));
        maxHPBar.setFillColor(sf::Color(0, 0, 0, 255));

        currentHPBar.setSize(sf::Vector2f(144, 19));
        currentHPBar.setFillColor(sf::Color(116, 227, 27, 255));

        statsText.setString("atk: " + to_string(entity.points[AttribIndex::ATK])     + '\n' +
                           "def: " + to_string(entity.points[AttribIndex::DEF])     + '\n' +
                           "hp:  " + to_string(entity.points[AttribIndex::HP_MAX])  + '\n' +
                           "reg: " + to_string(entity.points[AttribIndex::HP_REGEN])+ '\n' +
                           "spd: " + to_string(entity.points[AttribIndex::SPD])     + '\n');
        statsText.setFont(font);
        statsText.setCharacterSize(30);
        statsText.setFillColor(sf::Color(255, 255, 255, 255));

        damageTakenText.setString("");
        damageTakenText.setFont(font);
        damageTakenText.setCharacterSize(30);
        damageTakenText.setFillColor(sf::Color(234,96,20));
        // Oculta dano tomado
        damageTakenText.setScale(sf::Vector2f(0, 0));
    }

    void SetEntity(Entity newEntity){
        entity = newEntity;
        UpdateStats();
    }

    void TakeDamage(int dmg){
        float previous_hp = entity.hp_current;
        entity.TakeDamage(dmg);
        float new_hp = entity.hp_current;

        damageTakenText.setString(to_string((int)(previous_hp - new_hp)));

        if(entity.IsDead()){
            currentHPBar.setScale(sf::Vector2f(0, 1));
        } else {
            currentHPBar.setScale(sf::Vector2f(entity.hp_current/entity.hp_max, 1));
        }
    }

    void HighlightName(){
        name.setOutlineColor(sf::Color(255, 255, 255, 255));
    }

    void NormalizeName(){
        name.setOutlineColor(sf::Color(255, 255, 255, 0));
    }

    void ShowDamageTaken(){
        damageTakenText.setScale(sf::Vector2f(1, 1));
    }

    void HideDamageTaken(){
        damageTakenText.setScale(sf::Vector2f(0, 0));
    }

    bool IsDead(){
        return entity.IsDead();
    }

    void RegenerateHP(){
        entity.RegenerateHP();

        currentHPBar.setScale(sf::Vector2f(entity.hp_current/entity.hp_max, 1));
    }

    void DrawInto(sf::RenderWindow &window){
        window.draw(name);
        window.draw(HPText);
        window.draw(maxHPBar);
        window.draw(currentHPBar);
        window.draw(sprite);
        window.draw(statsText);
        window.draw(damageTakenText);
    }

    private:
    void UpdateStats(){
        statsText.setString("atk: " + to_string(entity.points[AttribIndex::ATK])     + '\n' +
                           "def: " + to_string(entity.points[AttribIndex::DEF])     + '\n' +
                           "hp:  " + to_string(entity.points[AttribIndex::HP_MAX])  + '\n' +
                           "reg: " + to_string(entity.points[AttribIndex::HP_REGEN])+ '\n' +
                           "spd: " + to_string(entity.points[AttribIndex::SPD])     + '\n');
    }
};

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

    pid_t process_id;

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


        //Processo filho GNU_PLOT
        // process_id = fork();

        // if(process_id == -1){ //Erro no fork
        //     cout<<"\n\n erro no fork do processo para criar o gnuplot";
        //     battleLog.close();
        //     fitnessData.close();
        //     exit(EXIT_FAILURE);
        // }else if(process_id == 0) { //No processo filho
        //     char * argv_list[] = {"gnuplot","gnuplot_in",NULL};
        //     if(execvp("gnuplot",argv_list) == -1){ //Executa o gnuplot
        //         cout << "\nErro ao tentar executar o Gnuplot\n";
        //     }
        //     exit(0);
        // }

    } else if(aux == 2){
        int iterations = -1;
        int generations = 1;

        cout << "Gerações por passo: ";
        cin >> generations;       

        do {
            iterations++;
            for(int i=0; i<generations; i++) {
                bestCurrent = Evaluate(population, enemies, battleLog);

                // Guarda o melhor individuo das ultimas gerações
                for(int i=1; i<BEST_IND_COUNT; i++){
                    bestEver[i-1] = bestEver[i];
                }

                bestEver[BEST_IND_COUNT-1] = *bestCurrent;

                Crossover(population, bestCurrent);

                IncreaseMutationRate(bestEver, population);

                // Salva fitness do melhor da geração no arquivo
                fitnessData << iterations*generations + i << " " << bestCurrent->score << endl;
            }

            bestCurrent->PrintEntity();
            cout << "taxa de mutação: " << MUT_RATE << endl;
            

            //Processo filho GNU_PLOT
            if(iterations == 0){ //Apenas para a primeira iteracao faz o fork 
                process_id = fork();

                if(process_id == -1){ //Erro no fork
                    cout<<"\n\n erro no fork do processo para criar o gnuplot";
                    battleLog.close();
                    fitnessData.close();
                    exit(EXIT_FAILURE);
                }else if(process_id == 0) { //No processo filho
                    char * argv_list[] = {"gnuplot","gnuplot_in2",NULL};
                    if(execvp("gnuplot",argv_list) == -1){ //Executa o gnuplot
                        cout << "\nErro ao tentar executar o Gnuplot\n";
                    }
                    exit(0);
                }
            }

        } while(cin.get() == '\n');
    }

    bestEver[BEST_IND_COUNT-1].PrintEntity();

    // Testando SFML
    sf::RenderWindow window(sf::VideoMode(800, 600), "AG");
    window.clear(sf::Color(56, 56, 56, 255));

    // Best Score text GUI
    sf::Font font;
    font.loadFromFile("fonts/upheavtt.ttf");
    sf::Text bestScoreText("best score: " + to_string(bestEver[BEST_IND_COUNT-1].score), font, 30);
    bestScoreText.setPosition(24.83, 16.32);
    bestScoreText.setFillColor(sf::Color(255, 255, 255, 255));    

    sf::Event event;
    
    State state = State::INITIALIZING;
    int enemyIndex = 0;
    GUIEntity Player(bestEver[BEST_IND_COUNT-1], EntityType::PLAYER, font);
    GUIEntity Enemy(enemies[0][enemyIndex], EntityType::ENEMY, font);

    auto time_previous = chrono::high_resolution_clock::now();
    auto time_current = chrono::high_resolution_clock::now();
    // auto duration = chrono::duration_cast<chrono::seconds>( time_current - time_previous ).count();
    int64_t wait_duration = 2;
    
    while(window.isOpen()){
        while(window.pollEvent(event)){
            if(event.type == sf::Event::Closed){
                window.close();
            }
        }

        if(chrono::duration_cast<chrono::seconds>(time_current - time_previous).count() < wait_duration){
            time_current = chrono::high_resolution_clock::now();
        } else {
            switch(state){
                case State::INITIALIZING:
                    Player.RegenerateHP();
                    if(Player.entity.spd >= Enemy.entity.spd){
                        state = State::PLAYERS_TURN;
                    } else {
                        state = State::ENEMYS_TURN;
                    }
                    break;
                case State::PLAYERS_TURN:
                    Player.HighlightName();
                    Enemy.NormalizeName();
                    break;
                case State::ENEMYS_TURN:
                    Enemy.HighlightName();
                    Player.NormalizeName();
                    break;
                case State::END:

                    break;
                }
            time_previous = time_current;        
        }

        // Draw scene
        window.setActive();
        Player.DrawInto(window);
        Enemy.DrawInto(window);
        window.display();
    }

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