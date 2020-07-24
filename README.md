# Sistemas-Evolutivos

### Autores
- Rodrigo Mendes Andrade
- Marcelo Isaias de Moraes Junior

***

## Projeto

### O Jogo

![SS do jogo](../master/images/game_ss.jpg "Screenshot do jogo")

No jogo, o jogador deve distribuir uma certa quantidade de pontos entre diferentes atributos (ataque, defesa, etc.) de
seu personagem de modo que o mesmo consiga derrotar o maior número possível de inimigos de uma fila gerada aleatoriamente.
Os combates são no formato 1 contra 1 baseado em turnos (o jogador e o inimigo se alternam ao atacar).

### O Algoritmo

O algoritmo evolutivo foi aplicado de modo a encontrar a distribuição de pontos que permite que o personagem do jogador
derrote a maior quantidade de possível inimigos.

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

 O fitness é uma função do número de
vitórias do indivíduo e do número de turnos que cada batalha levou. Os dados do gráfico de fitness x geração
são salvos no arquivo [fitness.dat](../master/fitness.dat).

Para visualizar o gráfico, é possível utilizar o Gnuplot com os comandos descritos em [gnuplot_in](../master/gnuplot_in), executando em um terminal

```
gnuplot gnuplot_in
```

Exemplo:
![Gráfico de fitness](../master/images/graph.png "Gráfico de fitness")
