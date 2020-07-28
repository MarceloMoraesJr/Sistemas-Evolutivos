# Sistemas Evolutivos - Player's Best Stats

### Autores
- Rodrigo Mendes Andrade
- Marcelo Isaias de Moraes Junior

***

## Projeto

Vídeo:

[![Video](http://img.youtube.com/vi/wIrHEIi1anU/0.jpg)](https://youtu.be/wIrHEIi1anU)


### O Jogo

![SS do jogo](../master/images/game_ss.jpg "Screenshot do jogo")

No jogo, o jogador deve distribuir uma certa quantidade de pontos entre diferentes atributos (ataque, defesa, etc.) de
seu personagem de modo que o mesmo consiga derrotar o maior número possível de inimigos de uma fila gerada aleatoriamente.
Os combates são no formato 1 contra 1 baseado em turnos (o jogador e o inimigo se alternam ao atacar).

**Atributos**
 * Ataque (ATK) : determina o dano que a entidade causa ao oponente, reduzindo a vida deste
 * Defesa (DEF) : determina uma porcentagem de redução do dano sofrido pela entidade
 * Vida (HP) : determina a quantidade máxima de pontos de vida da entidade
 * Regeneração (REG): determina a vida recuperada pela entidade após uma batalha (não tem utlidade para os inimigos)
 * Velocidade (SPD) : determina qual das entidades começa atacando em uma batalha

### O Algoritmo

O algoritmo evolutivo foi aplicado de modo a encontrar a distribuição de pontos que permite que o personagem do jogador
derrote a maior quantidade possível de inimigos.

A população é formada por vários "personagens" que representam jogadores. O fitness de cada indivíduo é dado por uma função do número
de vitórias e do número de turnos que cada batalha levou, de modo que quanto mais vitórias e quanto mais rápidas forem as batalhas, maior
o fitness.

Para cada geração, calcula-se o fitness de cada indivíduo, realiza-se o crossover de maneira "elitista" (todo indivíduo se torna a média entre o mesmo e o
best-fit da geração atual) com mutação. A taxa de mutação aumenta caso o fitness do best-fit continua parecido com o passar das gerações. Quando a taxa de
mutação atinge certo threshold, a população é gerada novamente com distruibação de atributos aleatória (mas o best-fit permanece).

### O Programa

Ao executar, escolhe-se um dos modos de operação:
 * Auto: o algoritmo executa automaticamente para o número de gerações especificado no ``` #define GENERATIONS_NUM ```
 * Interativo: define-se o número de gerações a serem avaliadas por passo e a cada ```enter``` pressionado, o algoritmo é executado
 para tal número de gerações, mostrando-se as características (cromossomos e fitness) do best-fit e a taxa de mutação.
 ``` Ctrl+D ``` encerra a execução do algoritmo.
 
 Por fim, é gerada uma interface gráfica onde simula-se o best-fit encontrado sendo utilizado no jogo.

## Execução

O software deve ser compilado e executado em um ambiente Linux.

Para compilar, é necessário ter instalado a biblioteca [SFML](https://www.sfml-dev.org) e o compilador g++

Para compilar, executar em um terminal:

``` 
make 
```

Para executar:

``` 
make run 
```

## Gráfico de fitness

Os dados do gráfico de fitness x geração são salvos no arquivo [fitness.dat](../master/fitness.dat).
Os dados do gráfico de média x geração são salvos no arquivo [mean.dat](../master/mean.dat).

Para visualizar ambos os gráficos simultaneamente, é possível utilizar o Gnuplot com os comandos descritos em [gnuplot_in](../master/gnuplot_in), executando em um terminal:

```
gnuplot gnuplot_in
```

Exemplo:


![Gráfico de fitness](../master/images/graph.png "Gráfico de fitness")
