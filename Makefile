NAME = webserver
CC = c++
FLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = main.cpp ./Srcs/ServerBuild.cpp ./Srcs/ServerStart.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS) ./Includes/SrvConfig.hpp ./Includes/colors.hpp
	$(CC) $(FLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp ./Includes/SrvConfig.hpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all