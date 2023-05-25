ALESTI_TRABALHO = app
CFLAGS = -c -std=c++11

all:									${ALESTI_TRABALHO}

${ALESTI_TRABALHO}:							main.o
										g++ main.o -o ${ALESTI_TRABALHO}

main.o:									main.cpp
										g++ ${CFLAGS} main.cpp

run:									${ALESTI_TRABALHO}
										./${ALESTI_TRABALHO}

clean:									
										rm -f main.o ${ALESTI_TRABALHO}