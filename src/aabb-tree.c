// AABB tree implementation based off of:
// http://www.cs.nmsu.edu/~joshagam/Solace/papers/master-writeup-print.pdf 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "partition.h"
#include "memory.h"

#define SPLIT_X 1
#define SPLIT_Y 2
#define SPLIT_Z 4

#define SPLIT_DIR_THRESHOLD 4
#define SPLIT_THRESHOLD 30 
#define TREE_NODES 45

#define ALL_PLANES 63 // bx111111

typedef enum {
    INSIDE, OUTSIDE, INTERSECT
} Intersection;

typedef struct {
    float x, y, z;
} Point;

typedef struct aabbTree {
    struct aabbTree *parent;
    struct aabbTree *children[27];
    HPGpool pool;
    unsigned short split, lastChecked;
    Point splitPoint;
    Point min;
    Point max;
    bool extentsCorrect;
    HPGvector nodes;
    Node *nodesData[TREE_NODES];
} AABBtree;

AABBtree *hpgAABBnewTree(HPGpool *pool);
AABBtree *hpgAABBfindNode(Node *node, AABBtree *tree);
void hpgAABBaddNode(Node *node, AABBtree *tree);
void hpgAABBremoveNode(Node *node);
void hpgAABBupdateNode(Node *node);
void hpgAABBdoVisible(AABBtree *tree, Plane *planes, void (*func)(Node *));
static void getAABBtreeExtents(AABBtree *tree, Point *min, Point *max);
static AABBtree *newTree(HPGpool pool, AABBtree *parent);
static void splitTree(AABBtree *tree);
static void updateExtents(AABBtree *tree);
static AABBtree *whichBranch(AABBtree *tree, BoundingSphere *bs);
static void growExtents(AABBtree *tree, BoundingSphere *bs);
static void shrinkExtents(AABBtree *tree, BoundingSphere *bs);
static void maybeKillTree(AABBtree *tree);
static void deleteTree(AABBtree *tree);
static bool contains(AABBtree *t, BoundingSphere *bs);

#ifdef DEBUG
void printTree(AABBtree *tree){
    printf("Tree %p [(%f %f) (%f %f) (%f %f)]", tree,
           tree->min.x, tree->max.x,
           tree->min.y, tree->max.y,
           tree->min.z, tree->max.z);
}
#endif 

PartitionInterface partitionInterface = {sizeof(AABBtree),
                                         (void *(*)(HPGpool)) hpgAABBnewTree,
                                         (void (*)(Node *, void *)) hpgAABBaddNode,
                                         (void (*)(Node *)) hpgAABBremoveNode,
                                         (void (*)(Node *)) hpgAABBupdateNode,
                                         (void (*)(void *, Plane *, void (*)(Node *))) 
                                           hpgAABBdoVisible};

void *hpgAABBpartitionInterface = (void *) &partitionInterface;

AABBtree *hpgAABBnewTree(HPGpool *pool){
    return newTree(pool, NULL);
}

static AABBtree *newTree(HPGpool pool, AABBtree *parent){
    AABBtree *tree = hpgAllocateFrom(pool);
    hpgInitStaticVector(&tree->nodes, tree->nodesData, TREE_NODES);
    tree->parent = parent;
    tree->pool = pool;
    tree->split = 0;
    tree->lastChecked = 0;
    tree->extentsCorrect = false;
    memset(tree->children, 0, 27 * sizeof(void *));
    return tree;
}

AABBtree *hpgAABBfindNode(Node *node, AABBtree *tree){
    AABBtree *t = tree;
    AABBtree *u = NULL;
    while (u != t){
	u = t;
	t = whichBranch(t, node->boundingSphere);
    }
    return t;
}

void addNode(Node *node, AABBtree *tree){
    hpgPush(&tree->nodes, node);
    node->area = (void *) tree;
#ifdef DEBUG
    printf("Added node %p to tree %p\n", node->data, tree);
#endif
}

void hpgAABBaddNode(Node *node, AABBtree *tree){
    AABBtree *t = hpgAABBfindNode(node, tree);
    addNode(node, t);
    growExtents(tree, node->boundingSphere);
}

void hpgAABBremoveNode(Node *node){
    AABBtree *tree = (AABBtree *) node->area;
    if (hpgRemove(&tree->nodes, (void *) node)){
	shrinkExtents(tree, node->boundingSphere);
	maybeKillTree(tree);
    } else {
	fprintf(stderr, "Warning, tried to remove node %p from an AABB tree that it did not belong to\n", node->data);
#ifdef DEBUG
        int i;
        printTree(tree);
        printf("\nNodes: ");
        for (i = 0; i < tree->nodes.size; i++)
            printf("%p ", ((Node *) tree->nodes.data[i])->data);
        printf("\n");
        AABBtree *topLevel = tree;
        while (topLevel->parent){
            topLevel = topLevel->parent;
        }
        printf("Maybe child of: ");
        AABBtree *parent = hpgAABBfindNode(node, topLevel);
        printTree(parent);
        printf("\nNodes: ");
        for (i = 0; i < parent->nodes.size; i++)
            printf("%p ", ((Node *) tree->nodes.data[i])->data);
        printf("\n");
        exit(EXIT_FAILURE);
#endif
    }
}

void hpgAABBupdateNode(Node *node){
    AABBtree *tree = (AABBtree *) node->area;
    AABBtree *t = tree;
    while (t->parent && !contains(t, node->boundingSphere)){
	t = t->parent;
    }
    t = hpgAABBfindNode(node, t);
    if (t != tree){
	hpgAABBremoveNode(node);
	addNode(node, t);
        growExtents(tree, node->boundingSphere);
    } else {
	growExtents(t, node->boundingSphere);
	shrinkExtents(t, node->boundingSphere);
    }
}

static void getAABBtreeExtents(AABBtree *tree, Point *min, Point *max){
    if (!tree->extentsCorrect)
	updateExtents(tree);
    if (!tree->split && (tree->nodes.size >= SPLIT_THRESHOLD))
	splitTree(tree);
    *min = tree->min;
    *max = tree->max;
}

static bool contains(AABBtree *t, BoundingSphere *bs){
    return (bs->x - bs->r >= t->min.x &&
	    bs->y - bs->r >= t->min.y &&
	    bs->z - bs->r >= t->min.z &&
	    bs->x + bs->r <= t->max.x &&
	    bs->y + bs->r <= t->max.y &&
	    bs->z + bs->r <= t->max.z);
}

static void growExtents(AABBtree *tree, BoundingSphere *bs){
    AABBtree *t = tree;
    do {
	t->max.x = fmax(t->max.x, bs->x + bs->r);
	t->max.y = fmax(t->max.y, bs->y + bs->r);
	t->max.z = fmax(t->max.z, bs->z + bs->r);
	t->min.x = fmin(t->min.x, bs->x - bs->r);
	t->min.y = fmin(t->min.y, bs->y - bs->r);
	t->min.z = fmin(t->min.z, bs->z - bs->r);
    } while ((t = t->parent));
   
}

static void shrinkExtents(AABBtree *tree, BoundingSphere *bs){
    AABBtree *t = tree;
    do {
	if (bs->x - bs->r <= t->min.x ||
	    bs->y - bs->r <= t->min.y ||
	    bs->z - bs->r <= t->min.z ||
	    bs->x + bs->r >= t->max.x ||
	    bs->y + bs->r >= t->max.y ||
	    bs->z + bs->r >= t->max.z)
	    t->extentsCorrect = false;
    } while ((t = t->parent));
}

static void removeChild(AABBtree *tree, AABBtree *c){
    bool unsplit = true;
    int i;
    for (i = 0; i < 27; i++){
	AABBtree *child = tree->children[i];
	if (child){
	    if (child == c)
		tree->children[i] = NULL;
	    else
		unsplit = false;
	}
    }
    if (unsplit) tree->split = 0;
    maybeKillTree(tree);
}

static void maybeKillTree(AABBtree *tree){
    int i;
    if (tree->parent && tree->nodes.size == 0){
	for (i = 0; i < 27; i++)
	    if (tree->children[i]) return;
#ifdef DEBUG
        printf("Killing tree ");
        printTree(tree);
        printf("\n");
#endif
	removeChild(tree->parent, tree);
	deleteTree(tree);
    }
}

static AABBtree *whichBranch(AABBtree *tree, BoundingSphere *bs){
    float radius = bs->r;
    float ov = radius * radius * radius;
    float bv = (tree->max.x - tree->min.x) *
	(tree->max.y - tree->min.y) * (tree->max.z - tree->min.z);
    if (!tree->split ||
	(tree->extentsCorrect && (ov > bv/8) &&
	 (tree->nodes.size != tree->nodes.capacity)))
	return tree;
    int x = 0, y =0, z = 0;
    if (tree->split & SPLIT_X){
	if (bs->x + radius < tree->splitPoint.x)
	    x = 0;
	else if (bs->x - radius > tree->splitPoint.x)
	    x = 2;
	else x = 1;
    }
    if (tree->split & SPLIT_Y){
	if (bs->y + radius < tree->splitPoint.y)
	    y = 0;
	else if (bs->y - radius > tree->splitPoint.y)
	    y = 2;
	else y = 1;
    }
    if (tree->split & SPLIT_Z){
	if (bs->z + radius < tree->splitPoint.z)
	    z = 0;
	else if (bs->z - radius > tree->splitPoint.z)
	    z = 2;
	else z = 1;
    }
    int i = x + y*3 + z*9;
    if (tree->children[i])
	return tree->children[i];
    else
	return tree->children[i] = newTree(tree->pool, tree);
}

static void setSplitLocation(AABBtree *tree){
    float x = 0, y = 0, z= 0;
    int i;
    for (i = 0; i < tree->nodes.size; i++){
	BoundingSphere *bs =
	    ((Node *) tree->nodes.data[i])->boundingSphere;
	x += bs->x;
	y += bs->y;
	z += bs->z;
    }
    x /= i; y /= i; z /= i;
    Point p = {x, y, z};
    tree->splitPoint = p;
}

static void setSplitDirection(AABBtree *tree){
    float x = tree->max.x - tree->min.x;
    float y = tree->max.y - tree->min.y;
    float z = tree->max.z - tree->min.z;
    unsigned int dir = SPLIT_X | SPLIT_Y | SPLIT_Z;
    if ((x < y/SPLIT_DIR_THRESHOLD) || (x < z/SPLIT_DIR_THRESHOLD))
	dir &= ~SPLIT_X;
    if ((y < x/SPLIT_DIR_THRESHOLD) || (y < z/SPLIT_DIR_THRESHOLD))
	dir &= ~SPLIT_Y;
    if ((z < x/SPLIT_DIR_THRESHOLD) || (z < y/SPLIT_DIR_THRESHOLD))
	dir &= ~SPLIT_Z;
    tree->split = dir;
}

static void deleteTree(AABBtree *tree){
    hpgDeleteVector(&tree->nodes);
    hpgDeleteFrom(tree, tree->pool);
}

static void updateExtents(AABBtree *tree){
    HPGvector *nodes = &tree->nodes;
    int nCurrentNodes = nodes->size;
    Point max = {-INFINITY, -INFINITY, -INFINITY};
    Point min = {INFINITY, INFINITY, INFINITY};
    int i;
    for (i = 0; i < nCurrentNodes; i++){
	Node *node = nodes->data[i];
	BoundingSphere *bs = node->boundingSphere;
	max.x = fmax(max.x, bs->x + bs->r);
	max.y = fmax(max.y, bs->y + bs->r);
	max.z = fmax(max.z, bs->z + bs->r);
	min.x = fmin(min.x, bs->x - bs->r);
	min.y = fmin(min.y, bs->y - bs->r);
	min.z = fmin(min.z, bs->z - bs->r);
    }
    for (i = 0; i < 27; i++){
	AABBtree *child = tree->children[i];
	if (child){
	    if (!child->extentsCorrect)
		updateExtents(child);
	    max.x = fmax(max.x, child->max.x);
	    max.y = fmax(max.y, child->max.y);
	    max.z = fmax(max.z, child->max.z);
	    min.x = fmin(min.x, child->min.x);
	    min.y = fmin(min.y, child->min.y);
	    min.z = fmin(min.z, child->min.z);
	}
    }
    tree->max = max;
    tree->min = min;
    setSplitLocation(tree);
    tree->extentsCorrect = true;
}

static void splitTree(AABBtree *tree){
    HPGvector *nodes = &tree->nodes;
    int nCurrentNodes = nodes->size;
    int i;
    setSplitLocation(tree);
    setSplitDirection(tree);
    for (i = 0; i < nCurrentNodes; i++){
	Node *node = nodes->data[i];
	AABBtree *branch = whichBranch(tree, node->boundingSphere);
	if (branch !=tree){
            addNode(node, branch);
            nodes->data[i] = NULL;
	}
    }
#ifdef DEBUG
    printf("Split tree %p at (%f, %f, %f) along: ", tree, tree->splitPoint.x,
           tree->splitPoint.y, tree->splitPoint.x);
    if (tree->split & SPLIT_X) printf("x-axis ");
    if (tree->split & SPLIT_Y) printf("y-axis ");
    if (tree->split & SPLIT_Z) printf("z-axis ");
    printf("\n");
#endif
    for (i = 0; i < nodes->size;){
	if (!nodes->data[i]){
	    hpgRemoveNth(nodes, i);
	} else {
	    i++;
	}
    }
    for (i = 0; i < 27; i++){
	AABBtree *child = tree->children[i];
	if (child){
	    if (child->nodes.size == nCurrentNodes)
		goto abort;
	}
    }
    for (i = 0; i < 27; i++){
	AABBtree *child = tree->children[i];
	if (child){
	    updateExtents(child);
	}
    }
    return;
abort:
#ifdef DEBUG
    printf("Aborting split of tree %p\n", tree);
#endif
    for (i = 0; i < 27; i++){
	AABBtree *child = tree->children[i];
	if (child){
	    Node *n;
	    while ((n = hpgPop(&child->nodes))){
                addNode(n, tree);
	    }
	}
    }
}

/* Visibility testing */
/*
  Based on algorithm described in this paper:
    http://jesper.kalliope.org/blog/library/vfcullbox.pdf
  Older version (some bits explained more in-depth) of the same:
    http://www.cse.chalmers.se/~uffe/vfc.pdf
  Third-party examination of algorithm:
    http://www.cescg.org/CESCG-2002/DSykoraJJelinek/#s6
 */

#ifdef DEBUG
int nTrees = 0;
bool modified = false;
#endif 

static void setPNvectors(Plane *plane, Point *p, Point *n, Point *min, Point *max){
    if (plane->a < 0.0) { p->x = min->x; n->x = max->x; }
    else                { p->x = max->x; n->x = min->x; }
    if (plane->b < 0.0) { p->y = min->y; n->y = max->y; }
    else                { p->y = max->y; n->y = min->y; }
    if (plane->c < 0.0) { p->z = min->z; n->z = max->z; }
    else                { p->z = max->z; n->z = min->z; }
}

static Intersection inPlanes(AABBtree *t, Plane *planes, int inMask, int *outMask){
    float a, b; int i, k = 1 << t->lastChecked;
    Point p, n;
    Intersection result = INSIDE;
    Plane plane = planes[t->lastChecked];
    Point min, max;
    getAABBtreeExtents(t, &min, &max);
    if (k & inMask) {
        setPNvectors(&plane, &p, &n, &min, &max);
        a = (plane.a * p.x) + (plane.b * p.y) + (plane.c * p.z) + plane.d;
        if (a < 0) return OUTSIDE;
        b = (plane.a * n.x) + (plane.b * n.y) + (plane.c * n.z) + plane.d;
        if (b < 0) { *outMask |= k; result = INTERSECT; }
    }
    for (i = 0, k = 1; k <= inMask; i++, k += k){
        if ((i != t->lastChecked) && (k & inMask)){
            plane = planes[i];
            setPNvectors(&plane, &p, &n, &min, &max);
            a = (plane.a * p.x) + (plane.b * p.y) + (plane.c * p.z) + plane.d;
            if (a < 0) { t->lastChecked = i; return OUTSIDE; }
            b = (plane.a * n.x) + (plane.b * n.y) + (plane.c * n.z) + plane.d;
            if (b < 0) { *outMask |= k; result = INTERSECT; }
        }
    }
    return result;
}

static void treeMap(AABBtree *tree, void (*func)(Node *)){
#ifdef DEBUG
    nTrees++;
#endif 
    int i;
    for (i = 0; i < tree->nodes.size; i++)
	func(tree->nodes.data[i]);
    for (i = 0; i < 27; i++){
	AABBtree *child = tree->children[i];
	if (child)
	    treeMap(child, func);
    }
}

static void doVisible(AABBtree *tree, Plane *planes, void (*func)(Node *), int planeMask){
    int nextMask = 0;
    int inView = inPlanes(tree, planes, planeMask, &nextMask);
    int i;
    if (inView == INSIDE)
	treeMap(tree, func);
    else if (inView == INTERSECT){
#ifdef DEBUG
        nTrees++;
#endif 
	for (i = 0; i < tree->nodes.size; i++)
	    func(tree->nodes.data[i]);
	for (i = 0; i < 27; i++){
	    AABBtree *child = tree->children[i];
	    if (child)
		doVisible(child, planes, func, nextMask);
	}
    }
}

void hpgAABBdoVisible(AABBtree *tree, Plane *planes, void (*func)(Node *)){
#ifdef DEBUG
    int oldNTrees = nTrees;
    nTrees = 0;
#endif 
    doVisible(tree, planes, func, ALL_PLANES);
#ifdef DEBUG
    if ((nTrees != oldNTrees)){
        printf("%d trees were visible\n", nTrees);
        modified = true;
    } else {
        modified = false;
    }
#endif 
}
