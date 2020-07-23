all:
	g++ main.cpp -o main -lm -lsfml-graphics -lsfml-window -lsfml-system
run:
	./main