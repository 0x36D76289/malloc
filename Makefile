# Variables
NAME = libft_malloc
CC = gcc
CFLAGS = -Wall -Wextra -Werror -fPIC -O2 -Iinclude
LDFLAGS = -shared -lpthread

# Check HOSTTYPE environment variable
ifeq ($(HOSTTYPE),)
HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

# Library names
LIB_NAME = $(NAME)_$(HOSTTYPE).so
LIB_LINK = $(NAME).so

# Directories
SRCDIR = src
INCDIR = include
OBJDIR = obj

# Source and object files
SRCS = malloc.c malloc_bonus.c
SRCFILES = $(addprefix $(SRCDIR)/, $(SRCS))
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))

# Include libft if it exists
LIBFT_DIR = libft
LIBFT = $(LIBFT_DIR)/libft.a

# Check if libft directory exists
ifneq ($(wildcard $(LIBFT_DIR)/Makefile),)
HAS_LIBFT = 1
else
HAS_LIBFT = 0
endif

# Rules
all: $(OBJDIR) $(LIB_LINK)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(LIB_LINK): $(LIB_NAME)
	ln -sf $(LIB_NAME) $(LIB_LINK)

ifeq ($(HAS_LIBFT),1)
$(LIB_NAME): $(LIBFT) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBFT)
else
$(LIB_NAME): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)
endif

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

ifeq ($(HAS_LIBFT),1)
$(LIBFT):
	make -C $(LIBFT_DIR)
endif

clean:
	rm -f $(OBJS)
ifeq ($(HAS_LIBFT),1)
	make -C $(LIBFT_DIR) clean
endif

fclean: clean
	rm -f $(LIB_NAME) $(LIB_LINK)
ifeq ($(HAS_LIBFT),1)
	make -C $(LIBFT_DIR) fclean
endif

re: fclean all

# Test target
test: $(LIB_LINK)
	$(CC) -o test_malloc test_malloc.c -Iinclude -L. -lft_malloc -lpthread
	LD_LIBRARY_PATH=. LD_PRELOAD=./$(LIB_LINK) ./test_malloc

.PHONY: all clean fclean re test
