NAME = libft.a

SRCDIR = src
INCDIR = include
OBJDIR = obj

SRCFILES = ft_atoi.c \
			ft_calloc.c \
			ft_isalpha.c \
			ft_isdigit.c \
			ft_itoa.c \
			ft_memcmp.c \
			ft_memmove.c \
			ft_putchar_fd.c \
			ft_putnbr_fd.c \
			ft_split.c \
			ft_strcmp.c \
			ft_strdup.c \
			ft_strlen.c \
			ft_strncmp.c \
			ft_substr.c \
			ft_toupper.c \
			ft_bzero.c \
			ft_isalnum.c \
			ft_isascii.c \
			ft_isprint.c \
			ft_memchr.c \
			ft_memcpy.c \
			ft_memset.c \
			ft_putendl_fd.c \
			ft_putstr_fd.c \
			ft_strcat.c \
			ft_strcpy.c \
			ft_strjoin.c \
			ft_strncat.c \
			ft_strncpy.c \
			ft_tolower.c \


SRCS = $(addprefix $(SRCDIR)/, $(SRCFILES))
OBJS = $(addprefix $(OBJDIR)/, $(SRCFILES:.c=.o))

CC = cc
CFLAGS = -Wall -Wextra -Werror
INCLUDES = -I$(INCDIR)

all: $(NAME)

$(NAME): $(OBJS)
	ar rcs $(NAME) $(OBJS)
	@echo -e "libft.a created successfully!"

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@rm -rf $(OBJDIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
