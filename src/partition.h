// The position and size of a node
typedef struct {
    float x, y, z, r;
} BoundingSphere;

// Struct passed to doVisible, defining a plane
typedef struct {
    float a, b, c, d;
} Plane;

typedef struct {
    BoundingSphere *boundingSphere;
    void *area; // For use by the partition: what area is this node in?
    void *data; // Data used by Hyperscene
} Node;

typedef struct partitionInterface{
    void *(*new)(); // Create and return a new partition for a scene
    void (*delete)(void *); // Delete the given partition
    void (*addNode)(Node *, void *); // Add a node to a scene
    void (*removeNode)(Node *); // Remove a node
    void (*updateNode)(Node *); // Called when a node has moved
    // For the given partition (arg 1) and a set of six planes (arg 2), call the given function (arg 3) with every node that is inside all six planes
    void (*doVisible)(void *, Plane *, void (*)(Node *));
} PartitionInterface;
