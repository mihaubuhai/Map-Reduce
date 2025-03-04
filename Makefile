build: utils.hpp
		g++ reducer.cpp mapper.cpp main.cpp -o tema1 -lpthread -Wall -Werror
clean:
		rm tema1
