all: stream-hmm.c
#gcc -std=c99 stream-hmm.c -o hmm
	g++ -std=c++11 -g stream-hmm.c -o hmm

fp: stream-hmm-fp.c fixed-point.h
	g++ -std=c++11 -g stream-hmm-fp.c -o fp

fp2: stream-hmm-fp2.c fixed-point.h
	g++ -std=c++11 -g stream-hmm-fp2.c -o fp2

test: test.c
	g++ -std=c++11 test.c -o test

clean:
	rm -f hmm test fp
