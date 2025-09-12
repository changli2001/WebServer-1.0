NAME = webserve
CC = c++
FLAGS = -Wall -Wextra -Werror -std=c++98

HEADERS = ./Includes/HttpServer.hpp ./Includes/colors.hpp ./Includes/Client.hpp \
			./parsconfig/ConfigParser.hpp ./parsconfig/LocationConfig.hpp  ./parsconfig/ServerConfig.hpp
SRCS = main.cpp ./Srcs/ServerBuild.cpp ./Srcs/ServerStart.cpp ./Srcs/Client.cpp ./Srcs/errorsGenerator.cpp ./Srcs/RequestParsing.cpp \
				./parsconfig/ConfigParser.cpp ./parsconfig/ConfigParserHelpers.cpp ./parsconfig/ConfigParserLocation.cpp \
				./parsconfig/ConfigParserServer.cpp 

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS) $(HEADERS)
	$(CC) $(FLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp $(HEADERS)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all