all:
	g++ server.cpp components/database.cpp components/command_parser.cpp  -g -Wall -o server -std=c++11;
	g++ subscriber.cpp components/command_parser.cpp -g -Wall -o subscriber -std=c++11;

server:
	g++ server.cpp components/database.cpp components/command_parser.cpp -g -Wall -o server -std=c++11;

subscriber:
	g++ subscriber.cpp components/command_parser.cpp -g -Wall -o subscriber -std=c++11;

clean:
	rm -rf subscriber server