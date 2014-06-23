typedef struct {
    float x, y, z, r;
} BoundingSphere;

typedef struct {
    float a, b, c, d;
} Plane;

typedef struct {
    BoundingSphere *boundingSphere;
    void *area;
    void *data;
} Node;

typedef struct {
    unsigned int structSize;
    void *(*new)(void *);
    void (*addNode)(Node *, void *);
    void (*removeNode)(Node *);
    void (*updateNode)(Node *);
    void (*doVisible)(void *, Plane *, void (*)(Node *));
} PartitionInterface;

