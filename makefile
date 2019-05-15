all: main.cpp TextQuery.cpp Query.cpp TextQuery.h Query.h QueryResult.h
	g++ -std=c++11 main.cpp TextQuery.cpp Query.cpp
	./a.out phone.txt

clean: 
	rm a.out
