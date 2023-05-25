# ALESTI-I-Trabalho-I

Trabalho de Algoritmos e Estruturas de Dados I.

Esse trabalho pediu que um arquivo .csv (Comma Separated Values), que contem informações sobre as sinalizações nas ruas de Porto Alegre, RS, fosse lido por um programa que armazene
os dados relevantes para uma série de consultas que devem serem feitas em listas simplesmente e duplamente encadeadas.

A lista duplamente encadeada armazena, em seus nodos, o nome de uma rua, o tipo de rua (avenida, rua, ou travessa), um ponteiro para uma lista simplesmente encadeada que contem todas as 
sinalizações registradas naquela rua, e ponteiros para o próximo nodo e o nodo anterior.

A lista simplesmente encadeada, por sua vez, armazena em seus nodos dados sobre uma sinalização: a data de instalação e as suas coordenadas geográficas caso elas existam no arquivo .csv.

-------

Algorithms and Data Structures I Assignment.

This assignment was about making a program that would read a .csv (Comma Separated Values) file which contains data on the street signs of Porto Alegre, RS. The program would store
the data on two separate linked lists: one, doubly linked, for the streets themselves, and another, singly linked, for the signs. Once all data was properly read in and stored in these
data structures, a series of consults are made upon user input.

The doubly linked list stores, in its nodes, a street name, the street type, a pointer to a singly linked list of street signs on that street, and pointers to the previous and next nodes
the list.

The singly linked list stores, in its nodes, data about a street sign: date of installation and geographic coordinates if they were registered in the .csv file.
