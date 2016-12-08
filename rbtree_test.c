/*
 * Author : Muhammad Falak R Wani (mfrw)
 * This program is adapted from the linux kernel and in no way,
 * I claim I have any copyright.
 */

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<stdint.h>
#include"rbtree.h"
#include"rbtree_augmented.h"
#include<sys/time.h>


#define NODES       10000
#define PERF_LOOPS  100
#define CHECK_LOOPS 10

struct test_node {
	uint32_t key;
	struct rb_node rb;

	/* following fields used for testing augmented rbtree functionality */
	uint32_t val;
	uint32_t augmented;
};

static struct rb_root root = RB_ROOT;
static struct test_node nodes[NODES];


static void insert(struct test_node *node, struct rb_root *root)
{
	struct rb_node **new = &root->rb_node, *parent = NULL;
	uint32_t key = node->key;

	while (*new) {
		parent = *new;
		if (key < rb_entry(parent, struct test_node, rb)->key)
			new = &parent->rb_left;
		else
			new = &parent->rb_right;
	}

	rb_link_node(&node->rb, parent, new);
	rb_insert_color(&node->rb, root);
}

static inline void erase(struct test_node *node, struct rb_root *root)
{
	rb_erase(&node->rb, root);
}

static inline uint32_t augment_recompute(struct test_node *node)
{
	uint32_t max = node->val, child_augmented;
	if (node->rb.rb_left) {
		child_augmented = rb_entry(node->rb.rb_left, struct test_node,
					   rb)->augmented;
		if (max < child_augmented)
			max = child_augmented;
	}
	if (node->rb.rb_right) {
		child_augmented = rb_entry(node->rb.rb_right, struct test_node,
					   rb)->augmented;
		if (max < child_augmented)
			max = child_augmented;
	}
	return max;
}

RB_DECLARE_CALLBACKS(static, augment_callbacks, struct test_node, rb,
		     uint32_t, augmented, augment_recompute)

static void insert_augmented(struct test_node *node, struct rb_root *root)
{
	struct rb_node **new = &root->rb_node, *rb_parent = NULL;
	uint32_t key = node->key;
	uint32_t val = node->val;
	struct test_node *parent;

	while (*new) {
		rb_parent = *new;
		parent = rb_entry(rb_parent, struct test_node, rb);
		if (parent->augmented < val)
			parent->augmented = val;
		if (key < parent->key)
			new = &parent->rb.rb_left;
		else
			new = &parent->rb.rb_right;
	}

	node->augmented = val;
	rb_link_node(&node->rb, rb_parent, new);
	rb_insert_augmented(&node->rb, root, &augment_callbacks);
}

static void erase_augmented(struct test_node *node, struct rb_root *root)
{
	rb_erase_augmented(&node->rb, root, &augment_callbacks);
}

static void init(void)
{
	int i;
	for (i = 0; i < NODES; i++) {
		nodes[i].key = random();
		nodes[i].val = random();
	}
}

static int is_red(struct rb_node *rb)
{
	return !(rb->__rb_parent_color & 1);
}

static int black_path_count(struct rb_node *rb)
{
	int count;
	for (count = 0; rb; rb = rb_parent(rb))
		count += !is_red(rb);
	return count;
}

static void check_postorder_foreach(int nr_nodes)
{
	struct test_node *cur, *n;
	int count = 0;
	rbtree_postorder_for_each_entry_safe(cur, n, &root, rb)
		count++;

}

static void check_postorder(int nr_nodes)
{
	struct rb_node *rb;
	int count = 0;
	for (rb = rb_first_postorder(&root); rb; rb = rb_next_postorder(rb))
		count++;

}

static void check(int nr_nodes)
{
	struct rb_node *rb;
	int count = 0, blacks = 0;
	uint32_t prev_key = 0;

	for (rb = rb_first(&root); rb; rb = rb_next(rb)) {
		struct test_node *node = rb_entry(rb, struct test_node, rb);
		if (!count)
			blacks = black_path_count(rb);
		else
		prev_key = node->key;
		count++;
	}


	check_postorder(nr_nodes);
	check_postorder_foreach(nr_nodes);
}

static void check_augmented(int nr_nodes)
{
	struct rb_node *rb;

	check(nr_nodes);
	for (rb = rb_first(&root); rb; rb = rb_next(rb)) {
		struct test_node *node = rb_entry(rb, struct test_node, rb);
	}
}

int main(void)
{
	srand(time(NULL));
	int i, j;

	struct timeval begin, end;
	double tm;
	
	printf("rbtree testing\n");

	init();
	gettimeofday(&begin, NULL);

	for (i = 0; i < PERF_LOOPS; i++) {
		for (j = 0; j < NODES; j++)
			insert(nodes + j, &root);
		for (j = 0; j < NODES; j++)
			erase(nodes + j, &root);
	}

	/* here, do your tm-consuming job */
	gettimeofday(&end, NULL);



	tm = (end.tv_sec - begin.tv_sec) / 1000.0;
	tm += (end.tv_usec - begin.tv_usec) * 1000.0;
	printf(" -> %.0f ms\n", (double)tm / PERF_LOOPS);

	for (i = 0; i < CHECK_LOOPS; i++) {
		init();
		for (j = 0; j < NODES; j++) {
			check(j);
			insert(nodes + j, &root);
		}
		for (j = 0; j < NODES; j++) {
			check(NODES - j);
			erase(nodes + j, &root);
		}
		check(0);
	}

	printf("augmented rbtree testing\n");

	init();

	gettimeofday(&begin, NULL);
	for (i = 0; i < PERF_LOOPS; i++) {
		for (j = 0; j < NODES; j++)
			insert_augmented(nodes + j, &root);
		for (j = 0; j < NODES; j++)
			erase_augmented(nodes + j, &root);
	}

	gettimeofday(&end, NULL);

	tm = (end.tv_sec - begin.tv_sec) / 1000.0;
	tm += (end.tv_usec - begin.tv_usec) * 1000.0;
	printf(" -> %.0f ms\n", (double)tm / PERF_LOOPS);
	for (i = 0; i < CHECK_LOOPS; i++) {
		init();
		for (j = 0; j < NODES; j++) {
			check_augmented(j);
			insert_augmented(nodes + j, &root);
		}
		for (j = 0; j < NODES; j++) {
			check_augmented(NODES - j);
			erase_augmented(nodes + j, &root);
		}
		check_augmented(0);
	}

	return 0; /* Fail will directly unload the module */
}
