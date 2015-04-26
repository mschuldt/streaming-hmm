all: stream-hmm.c
#gcc -std=c99 stream-hmm.c -o hmm
	g++ -std=c++11 -g stream-hmm.c -o hmm
test: test.c
	g++ -std=c++11 test.c -o test
