CFLAGS = -g -O2
rbtest: rbtree.o rbtree_test.o
	gcc -o rbtest rbtree.o rbtree_test.o
rbtree.o: rbtree.c rbtree.h rbtree_augmented.h
rbtree_test.o: rbtree_test.c rbtree.h rbtree_augmented.h

clean:
	rm -f *.o rbtest
