NAME = philo

CC = cc
CFLAGS = -Wall -Wextra -Werror -g -I.

SOURCES = philo.c creating.c utils.c monitor.c philo_utils.c

SRCS_DIR = srcs
SRCS =	$(addprefix $(SRCS_DIR)/, $(SOURCES))

OBJS_DIR = objs
OBJ_FILES =  $(addprefix $(OBJS_DIR)/, $(SOURCES:.c=.o))

RED = \033[1;37;41m
GREEN_SIMPLE = \033[0;32m
GREEN_ONE = \033[1;30;42m
YELLOW_SIMPLE = \033[0;33m
YELLOW_ONE = \033[1;30;43m
BLUE = \033[0;34m
RESET = \033[0m

all: $(NAME)

$(NAME): $(OBJ_FILES)
	@printf "${YELLOW_ONE}Compiling... ${RESET}\n"
	@printf "${GREEN_ONE}$(NAME) created!${RESET}\n"
	@$(CC) $(CFLAGS) $(OBJ_FILES) -o $(NAME)
	@printf "${BLUE}Run: ${RESET}${GREEN_SIMPLE}./philo ${YELLOW_SIMPLE}<n_philos> <time_die> <time_eat> <time_sleep> ${BLUE}[must_eat]${RESET}\n"

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c | $(OBJS_DIR)
	@$(CC) $(CFLAGS)  -c $< -o $@

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

clean:
	@printf "${RED}Cleaning...${RESET}\n"
	@rm -rf $(OBJS_DIR)

fclean: clean
	@printf "${RED}Deleting...${RESET}\n"
	@rm -f $(NAME)
	@printf "${GREEN_ONE}$(NAME) deleted successfully.${RESET}\n"

re: fclean all

.PHONY: all clean fclean re
