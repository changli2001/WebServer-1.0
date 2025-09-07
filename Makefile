NAME = webserver
CC = c++
FLAGS = -Wall -Wextra -Werror -std=c++98

HEADERS = ./includes/HttpServer.hpp ./includes/colors.hpp \
			./parsconfig/ConfigParser.hpp ./parsconfig/LocationConfig.hpp ./parsconfig/ServerConfig.hpp
SRCS = main.cpp ./Srcs/ServerBuild.cpp ./Srcs/ServerStart.cpp \
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