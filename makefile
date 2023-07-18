CFLAGS = -g -O2

rbtest: liburb.so rbtree_test.o
	gcc $(CFLAGS) -L. -Wl,-rpath=. -o rbtest rbtree_test.o -lurb

liburb.so: rbtree.o
	gcc -shared -o liburb.so rbtree.o

rbtree.o: rbtree.c rbtree.h rbtree_augmented.h
	gcc $(CFLAGS) -c -Wall -Werror -fpic rbtree.c

rbtree_test.o: rbtree_test.c rbtree.h rbtree_augmented.h

ctags: rbtree.c rbtree_test.c rbtree.h rbtree_augmented.h
	ctags rbtree.c rbtree_test.c rbtree.h rbtree_augmented.h
clean:
	rm -f *.o rbtest liburb.so tags
