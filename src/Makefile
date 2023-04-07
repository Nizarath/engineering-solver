# build options
include mkflg.mk

# fs tree
OUT = slv
OBJS =	\
	cmd/cmd.o \
	err/error.o \
	eval/eval.o \
	fun/fun.o \
	hist/history.o \
	infix/infix.o \
	io/getline.o \
	lexem/getlex.o \
	main.o \
	postfix/posteval.o \
	stack/stack.o \
	types/routines.o

# rules
$(OUT):	$(OBJS)
	$(CC) -o $(OUT) $(CFLAGS) $(OBJS)
clean:
	rm -f $(OUT) *.o */*.o
