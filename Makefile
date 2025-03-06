build: *.cpp utils.hpp
		g++ *.cpp -o mapred -lpthread -Wall -Werror
clean:
		rm mapred
