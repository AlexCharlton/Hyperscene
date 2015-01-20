# Hyperscene
Hyperscene is a scene library – made for placing objects in a shared world for the purpose of rendering – written in C. Hyperscene features a scene graph, cameras with a variety of movement types, frustum culling based on an configurable spatial partitioning system, an extension system – with which general extensions can be added to affect the way the scene is rendered – and a lighting extension. Hyperscene is target agnostic: it is not bound to any particular rendering target and should work equally well for curses, OpenGL, and anything in between.

Hyperscene was written with four principals in mind: It should be efficient, it shouldn’t place any hard limitations on the user, it should be extensible, and it should be easy to bind to other languages. Safety was certainly not a goal – user-supplied functions are regularly called, making it possible to screw things up in any number of ways – so consider yourself warned.

Hyperscene uses the [Hypermath](https://github.com/AlexCharlton/hypermath) library (included with the Hyperscene source). Using Hypermath may be desirable for working with the quaternions that control the rotation of Hyperscene nodes and cameras. Otherwise, you are free to use any other library for manipulating quaternions.

## Installation
`make install` will install libhyperscene in the `PREFIX` defaulting to `/usr/local`.

Various debugging statements are printed (in case you’re wondering how many things are being drawn, or what the partitioning system is doing) when `DEBUG` is defined. E.g. `make -DDEBUG`.

Some rendering options are also defined at compile time: `NO_REVERSE_PAINTER` `ROUGH_ALPHA`, and `VOLUMETRIC_ALPHA`. For an explanation of these options, see `hpsRenderCamera`.

## Requirements
None

## Documentation
Hyperscene’s scenes rely on a number of elements to be in place before a scene can be rendered. First is the scene itself. A scene could be thought of as the world or coordinate system that serves as the base for all the rendering operations. Second is a node. A node is the “physical” thing that can being rendered. Nodes can be added to scenes, or can be added to each other if you want a node to be defined in terms of its relation to another (hence the scene “graph”). Third is a camera. Cameras have a position and orientation in a scene, as well as a projection. Cameras can be rendered, which renders the part of the scene that they are pointing at. The fourth element that must be present is a pipeline. Pipelines are the collection of functions that explain how to render a node. If a node is to be rendered, it must have a pipeline associated with it.

These four elements are represented as opaque structs:

     struct HPSscene;
     struct HPSnode;
     struct HPScamera;
     struct HPSpipeline;

The basic use of Hyperscene is as follows: First you create pipelines and a scene. Then you add nodes to that scene (or to nodes that are already in the scene). These nodes are assigned a pipeline that knows how it can draw them. Then you create a camera associated with that scene, that has a particular position, orientation, and projection. Cameras are then called upon to render the scene.

Before any other functions can be used (except for pipeline creation), Hyperscene must be initialized:

     void hpsInit();


### Scenes
Scenes can be either active or inactive. The only difference is that active scenes are updated with a call to `hpsUpdateScenes`.

     HPSscene *hpsMakeScene();

Create a new scene. The scene’s space is partitioned with the [partition interface](#spatial-partitioning) given by `hpsPartitionInterface`. New scenes are automatically activated.

     void hpsDeleteScene(HPSscene *scene);

Delete the given scene. 

     void hpsActivateScene(HPSscene *scene);

Activate the given scene. If the scene is already active, this has no effect.

     void hpsDeactivateScene(HPSscene *scene);

Deactivate the given scene.

     void hpsUpdateScenes();

Update all active scenes. This must be called every frame in order to make sure all nodes are positioned correctly.

### Nodes
Nodes are the elements that are rendered in Hyperscene. They have five primary properties:

- Their parent: which can either be a scene, or another node
- Their position and orientation: how they are positioned relative to their parent and the world
- Their data: user-specified data
- Their pipeline: the set of functions that accepts the data, and renders the node
- Their bounding sphere radius: the radius that defines a sphere that encloses the entire visible space of the node

The following functions are used create, delete, and work with nodes:

     HPSnode *hpsAddNode(HPSnode *parent, void *data,
                         HPSpipeline *pipeline,
                         void (*deleteFunc)(void *));

Create a new node with the given parent. `parent` can either be a scene or another node. `data` is a user supplied pointer to some data which is then passed to the `pipeline` functions as well as `deleteFunc` when the node is deleted. If `pipeline` is `NULL`, the node will be invisible. `deleteFunc` may also be `NULL`.

     void hpsDeleteNode(HPSnode *node);

Delete the given node, removing it from the scene and calling `deleteFunc` on its `data`.

     HPSscene *hpsGetScene(HPSnode *node);

Return the scene that the node belongs to.

     void hpsSetNodeBoundingSphere(HPSnode *node, float radius);

Set the radius of the node’s bounding sphere. This is important to set so that Hyperscene knows when the node is inside a camera’s bounding volume or not. When a node is created, the bounding sphere radius is initially set to `1`.

     float *hpsNodeBoundingSphere(HPSnode *node);

Return the `(x y z radius)` bounding sphere of the node. The bounding sphere is positioned in world coordinates. This array returned should not be modified.

     void hpsSetNodePosition(HPSnode *node, float *position);

Set the `(x y z)` position of the node relative to its parent.

     void hpsMoveNode(HPSnode *node, float *vec);

Move the node by the given vector `(x y z)`.

     float* hpsNodePosition(HPSnode *node);

Return the `(x y z)` position of the node relative to its parent. If you want to modify the array returned, make sure to call `hpsNodeNeedsUpdate`.

     float* hpsNodeRotation(HPSnode *node);

Return the quaternion `(x y z w)` that describes the rotation of the node relative to its parent. Modifying this quaternion will rotate the node. Make sure to call `hpsNodeNeedsUpdate` after modifying the returned quaternion.

     void hpsNodeNeedsUpdate(HPSnode *node);

Nodes need to be informed when they have been modified in such a way that they need to be updated. Most node modification functions (`hpsSetNodePosition`, `hpsMoveNode`, `hpsSetNodeBoundingSphere`) call this automatically, but Hyperscene cannot tell when a node’s rotation quaternion has been modified. Make sure to call `hpsNodeNeedsUpdate` after modifying `hpsNodeRotation`’s return value.

     float* hpsNodeTransform(HPSnode *node);

Return the 4x4 transform matrix that describes the position and orientation of the node in world space. Consecutive elements of the matrix represent columns. Any modifications to the transform matrix will be lost when the scene is updated.

     void* hpsNodeData(HPSnode *node);

Return the node’s user supplied data.

#### Memory management
Hyperscene uses memory pools to store its data relating to nodes, which makes creation and deletion of nodes and scenes quick. For best performance, set `hpsNodePoolSize`:

    unsigned int hpsNodePoolSize;

to be as large as the greatest number of nodes that will be needed for a scene. Defaults to `4096`.


### Pipelines
Pipelines are structures consisting of three functions: a pre-render function, a render function, and a post-render function. When a scene (camera) is rendered, the visible nodes are sorted by their pipelines before they are drawn. Then, for every group of pipelines, the pre-render function is called with the first node as an argument. Every node is then passed to the render function. Finally, the post-render function is called to clean up. The sorting is done – and the pre/post-render functions are only called once – in order to minimize the amount of state changes that need to occur during rendering.

The exception to this is when a pipeline represents an element that could be partially transparent. “Alpha” pipelines get drawn after all the other ones, and the nodes that are associate with alpha pipelines are always drawn in order of decreasing distance from the camera. This ensures that the transparent parts can be rendered correctly.

When targeting OpenGL, one pipeline per shader program is generally desirable. The pre-render function should therefore call `glUseProgram`, while the render function should not. The post-render function can reset any state (e.g. `glUseProgram(0)`, etc).

     HPSpipeline *hpsAddPipeline(void (*preRender)(void *),
			                     void (*render)(void *),
			                     void (*postRender)(),
                                 bool isAlpha);

Create a new pipeline with the given functions. `isAlpha` indicates whether or not the pipeline can render any transparent elements.

     void hpsDeletePipeline(HPSpipeline *pipeline);

Delete the given pipeline.


### Cameras
Cameras, aside from having an orientation and position within a given scene, have two main properties. Their *type* is the sort of [projection](http://en.wikipedia.org/wiki/Graphical_projection) that the camera uses: either orthographic or perspective. The *style* of the camera indicates the way in which the camera can be moved (see [Movement and rotation](#movement-and-rotation) for details of each function):

- A *position* camera is one where the position and rotation of the camera is explicitly set. Movement functions: `hpsMoveCamera`, `hpsSetCameraPosition`, `hpsCameraRotation`.
- A *look-at* camera is given a position, an up-vector, and a point that it is looking at. The rotation of the camera is determined from these vectors. Movement functions: `hpsMoveCamera`, `hpsSetCameraPosition`, `hpsSetCameraUp`, `hpsCameraLookAt`.
- An *orbit* camera is given a point to look at, a distance from that object, and a yaw (rotation around the vertical axis), pitch (rotation around the side-to-side axis), and roll (rotation around the front-to-back axis). Movement functions: `hpsCameraLookAt`, `hpsYawCamera`, `hpsSetCameraYaw`, `hpsPitchCamera`, `hpsSetCameraPitch`, `hpsRollCamera`, `hpsSetCameraRoll`, `hpsZoomCamera`, `hpsSetCameraZoom`.
- A *first-person* camera has a position, yaw, pitch, and roll, and has special functions to move it forward and back, left and right, and up and down. Movement functions: `hpsMoveCamera`, `hpsSetCameraPosition`, `hpsYawCamera`, `hpsSetCameraYaw`, `hpsPitchCamera`, `hpsSetCameraPitch`, `hpsRollCamera`, `hpsSetCameraRoll`, `hpsMoveCameraForward`, `hpsMoveCameraUp`, `hpsStrafeCamera`.

The following functions are used create, delete, and work with cameras:

    HPScamera *hpsMakeCamera(HPScameraType type, HPScameraStyle style, HPSscene *scene, float width, float height);

Create a new camera associated with the given scene. `type` must be one of `HPS_ORTHO` or `HPS_PERSPECTIVE` for an orthographic or a perspective camera, respectively. `style` must be one of `HPS_POSITION`, `HPS_LOOK_AT`, `HPS_ORBIT`, or `HPS_FIRST_PERSON`. `width` and `height` are the width and height of the camera viewport, which may be modified with calls to `hpsSetCameraClipPlanes`, `hpsSetCameraViewAngle`, `hpsSetCameraViewportRatio`, `hpsSetCameraViewportDimensions`, `hpsSetCameraViewportScreenPosition`, and `hpsSetCameraViewportOffset`. New cameras are automatically activated.

     void hpsDeleteCamera(HPScamera *camera);

Delete the given camera.

     void hpsRenderCamera(HPScamera *camera);

Render the given camera. When cameras are rendered, all of the visible nodes are sorted: first into groups of nodes that have an alpha pipline or that don’t.

Alpha nodes are sorted by decreasing distance from the camera and rendered last. There are two sorting schemes that may be employed. The first, and default, scheme is useful when working with one-dimensional alpha objects. It sorts the distance of nodes based only on their origin, not taking into account their bounding sphere. The second scheme, enabled by defining `VOLUMETRIC_ALPHA` during compilation, is useful when working with three-dimensional alpha objects, and sorts distance while taking the bounding sphere into account. Each of these schemes has an accurate sorting version (the default) and a rougher but faster sorting version, which can be enabled by defining `ROUGH_ALPHA` during compilation.

Non-alpha nodes are sorted by pipeline. Each pipeline is then sorted again by increasing distance from the camera before they are rendered. By doing so, the things that are closest to the camera are drawn first (“reverse painter” sorting) which can help graphics hardware determine when later bits of the scene are hidden, thus saving some rendering time. Not all applications will benefit from this extra step, though, and it can be disabled by defining `NO_REVERSE_PAINTER` at compilation time.

     void hpsUpdateCamera(HPScamera *camera);

Update the given camera. This updates the view matrix of the camera to reflect any changes that may have occurred. This should always be done before rendering.

     void hpsActivateCamera(HPScamera *camera);

Add the camera to the list of active cameras (or push it to the back of the list, thus setting it to be rendered last). New cameras are automatically activated.

     void hpsDeactivateCamera(HPScamera *camera);

Remove the camera from the list of active cameras.

     void hpsRenderCameras();

Render all the active cameras.

     void hpsUpdateCameras();

Update all the active cameras.

     void hpsResizeCameras(float width, float height);

Modify the projection matrix of all cameras, based on the viewport dimensions `width` and `hight`. Should be called whenever the window is resized. The viewport dimensions of a camera are scaled by any values passed to `hpsSetCameraViewportRatio`. If `hpsSetCameraViewportDimensions` has been called on a camera, this function has no effect on it.

     void hpsSetCameraClipPlanes(HPScamera *camera, float near, float far);

Set the near and far clip planes of the camera. Nodes closer to or further away from these plans will not be visible. Defaults to `1` and `10000`.

     void hpsSetCameraViewAngle(HPScamera *camera, float angle);

Set the viewing angle of the perspective camera to `angle` degrees. Defaults to `70`. This doesn’t have any effect on orthographic cameras.

    void hpsSetCameraViewportRatio(HPScamera *camera, float width, float height);

Scales the camera’s viewport (its view frustum’s near plane) in the width and height direction. The effects of the scaling persist after `hpsResizeCameras` is called.

    void hpsSetCameraViewportDimensions(HPScamera *camera, float width, float height);

Sets the camera’s viewport (its view frustum’s near plane) dimensions to the given width and height, and fixes these dimensions such that they will not be changed when `hpsResizeCameras` is called.

    void hpsSetCameraViewportScreenPosition(HPScamera *camera, float left, float right, float bottom, float top);

Set the area of the screen that the camera renders to, defined by the rectangle of `left`, `right`, `bottom`, `top`. These default to the full screen, which is represented by values of `-1, 1, -1, 1`, respectively.

    void hpsSetCameraViewportOffset(HPScamera *camera, float x, float y);

Moves the camera’s viewport (its view frustum’s near plane) by `(x, y)` expressed as a fraction of the viewport’s width and height. This is generally only useful for a perspective projection, when lines should converge not to the middle of the screen, but to another point. Setting `x` to `0.5`, for example, moves the focal centre to the right edge of the viewport.

#### Movement and rotation
     void hpsMoveCamera(HPScamera *camera, float *vec);

Move the position of the camera by the vector `(x y z)`. Cannot be called with an *orbit* camera.

     void hpsSetCameraPosition(HPScamera *camera, float *point);

Set the position of the camera to the `(x y z)` point . Cannot be called with an *orbit* camera.

     float *hpsCameraPosition(HPScamera *camera);

Return the `(x y z)` position of the camera. Modifying this array will move the camera (although this will have no effect on an *orbit* camera).

     float *hpsCameraRotation(HPScamera *camera);

Return the `(x y z w)` quaternion that represents the camera’s rotation. Modifying this quaternion will rotate *position* cameras. The returned quaternion must not be modified for any other camera styles.

     void hpsCameraLookAt(HPScamera *camera, float *point);

Set the `(x y z)` point the *look-at* or *orbit* cameras are looking at.

     void hpsSetCameraUp(HPScamera *camera, float *up);

Set the camera’s `(x y z)` up-vector. Cannot be called with a non-*look-at* camera.

     void hpsYawCamera(HPScamera *camera, float angle);

Add `angle` radians to the *orbit* or *first-person* camera’s yaw.

     void hpsSetCameraYaw(HPScamera *camera, float angle);

Set the yaw of the *orbit* or *first-person* camera to `angle` radians.

     void hpsPitchCamera(HPScamera *camera, float angle);

Add `angle` radians to the *orbit* or *first-person* camera’s pitch.

     void hpsSetCameraPitch(HPScamera *camera, float angle);

Set the pitch of the *orbit* or *first-person* camera to `angle` radians.

     void hpsRollCamera(HPScamera *camera, float angle);

Add `angle` radians to the *orbit* or *first-person* camera’s roll.

     void hpsSetCameraRoll(HPScamera *camera, float angle);

Set the roll of the *orbit* or *first-person* camera to `angle` radians.

     void hpsZoomCamera(HPScamera *camera, float distance);

Add `distance` to the *orbit* camera’s zoom.

     void hpsSetCameraZoom(HPScamera *camera, float distance);

Set the zoom of the *orbit* camera to `distance`.

     void hpsMoveCameraForward(HPScamera *camera, float distance);

Move the *first-person* camera forward by `distance`. Only the camera’s yaw is taken into account for the movement.

     void hpsMoveCameraUp(HPScamera *camera, float distance);

Move the *first-person* camera up by `distance`.

     void hpsStrafeCamera(HPScamera *camera, float distance);

Move the *first-person* camera to the right by `distance`. Only the camera’s yaw is taken into account for the movement.

#### Camera matrix and position access
     float *hpsCameraProjection(HPScamera *camera);

Returns a pointer to the projection matrix of the camera.

     float *hpsCameraView(HPScamera *camera);

Returns a pointer to the view matrix of the camera.

     float *hpsCameraViewProjection(HPScamera *camera);

Returns a pointer to the `projection * view` matrix of the camera.

##### Currently rendering camera
While rendering, it can be desirable to have pointers to various matrices relating to the camera and node being rendered (e.g. to be used as uniform values). These pointers always point to the relevant value of the camera currently being rendered.

     float *hpsCurrentCamera();

Return a pointer to the camera currently being rendered.

     float *hpsCurrentCameraPosition();

Returns a pointer to the `(x y z)` position of the camera currently being rendered.

     float *hpsCurrentCameraView();

Returns a pointer to the view matrix of the camera currently being rendered.

     float *hpsCurrentCameraProjection();

Returns a pointer to the projection matrix of the camera currently being rendered.

     float *hpsCurrentCameraViewProjection();

Returns a pointer to the `projection * view` matrix of the camera currently being rendered.

     float *hpsCurrentCameraModelViewProjection();

Returns a pointer to the `projection * view * model` matrix of the node currently being rendered.

     float *hpsCurrentInverseTransposeModel();

Returns a pointer to the inverse transpose model matrix of the node currently being rendered. This matrix is useful for lighting. If it is not wanted, the calculation of this value can be omitted by defining `NO_INVERSE_TRANSPOSE` at compile time.

#### Distance sorting
A number of functions are defined to be used to sort two objects relative to the distance to a camera.

    int hpsCloserToCamera(const HPScamera *camera, const float *a, const float *b);

Compares the two three element `(X Y Z)` float positions, `a` and `b`, relative to their position to the `camera`. Returns `-1` when `a` is closer to the camera than `b`, `0` when the two are the same distance from the camera, and `1` when `a` is further from the camera. This is a rough-sorting function that compares distance on a dominant-axis basis, which will not always be accurate.

    int hpsFurtherFromCamera(const HPScamera *camera, const float *a, const float *b);

Compares the two three element `(X Y Z)` float positions, `a` and `b`, relative to their position to the `camera`. Returns `1` when `a` is closer to the camera than `b`, `0` when the two are the same distance from the camera, and `-1` when `a` is further from the camera.

    int hpsFurtherFromCameraRough(const HPScamera *camera, const float *a, const float *b);

Compares the two three element `(X Y Z)` float positions, `a` and `b`, relative to their position to the `camera`. Returns `1` when `a` is closer to the camera than `b`, `0` when the two are the same distance from the camera, and `-1` when `a` is further from the camera. This is a rough-sorting function that compares distance on a dominant-axis basis, which will not always be accurate.

    int hpsBSCloserToCamera(const HPScamera *camera, const float *a, const float *b);

Compares the two four element `(X Y Z R)` float positions, `a` and `b`, relative to their position to the `camera`, taking into account the radius of their bounding sphere `R`. Returns `-1` when `a` is closer to the camera than `b`, `0` when the two are the same distance from the camera, and `1` when `a` is further from the camera. This is a rough-sorting function that compares distance on a dominant-axis basis, which will not always be accurate.

    int hpsBSFurtherFromCamera(const HPScamera *camera, const float *a, const float *b);

Compares the two four element `(X Y Z R)` float positions, `a` and `b`, relative to their position to the `camera`, taking into account the radius of their bounding sphere `R`. Returns `1` when `a` is closer to the camera than `b`, `0` when the two are the same distance from the camera, and `-1` when `a` is further from the camera.

    int hpsBSFurtherFromCameraRough(const HPScamera *camera, const float *a, const float *b);

Compares the two four element `(X Y Z R)` float positions, `a` and `b`, relative to their position to the `camera`, taking into account the radius of their bounding sphere `R`. Returns `1` when `a` is closer to the camera than `b`, `0` when the two are the same distance from the camera, and `-1` when `a` is further from the camera. This is a rough-sorting function that compares distance on a dominant-axis basis, which will not always be accurate.


### Spatial Partitioning
Hyperscene only renders nodes that are within the bounds of a camera (i.e. it performs view frustum culling). In order for it to efficiently sort through the nodes, a [spatial partitioning](http://en.wikipedia.org/wiki/Space_partitioning) system is used. Different spatial partitioning systems can be used on a per-scene basis. Scenes are initialized with whatever spatial partitioning interface is pointed to by 

     void *hpsPartitionInterface;

By default this is 

     void *hpsAABBpartitionInterface;

`hpsAABBpartitionInterface` is a hybrid AABB ternary tree/nonatree/isoceptree inspired heavily by [Dynamic Spatial Partitioning for Real-Time Visibility Determination](http://www.cs.nmsu.edu/~joshagam/Solace/papers/master-writeup-print.pdf). 
When trees split, they try to split only along those axis where the nodes are most well dispersed. For example, if you have a 2D game, chances are nodes will be arranged along the `X` and `Y` axes, with little separation on the `Z` axis. In this situation, when enough nodes are added to a given AABB tree, it will only split along those two axes. In doing so, it avoids extraneous tree creation. This can be taken advantage of most in 2D situations by not using too much (`Z`) distance between layers.

The memory pool size of the `AABBpartitionInterface` can be set with

     unsigned int hpsAABBpartitionPoolSize;

which defaults to `4096`.

If you wish to write a new partition interface, create a `partitionIterface` struct with the relevant function pointers:  [`partition.h`](https://github.com/AlexCharlton/Hyperscene/blob/master/src/partition.h).

### Extensions
Hyperscene features an extension system, so that the rendering of a scene can be augmented in new and exciting ways.

Extensions can add special nodes to scenes. `hpsSetNodeExtension` is used to associate a node with an extension, so that it can trigger special actions. This node may be invisible if it was initialized with a null pipeline.

     void hpsActivateExtension(HPSscene *scene, HPSextension *extension);

Before an extension can be used in a given scene, it must be activated.

     void *hpsExtensionData(HPSscene *scene, HPSextension *extension);

Each scene stores a pointer that corresponds to the data of a given extension. This function will return it.

    void hpsSetNodeExtension(HPSnode *node, HPSextension *extension);

Set the extension that the node is associated with.

#### Lights
Hyperscene supplies an extension that provides a generic lighting system:

     HPSextension *hpsLighting;

Before lights can be used in a scene, `hpsLighting` must be activated with `hpsActivateExtension`.

     unsigned int hpsMaxLights;

Only `hpsMaxLights` visible lights can be used at time in a scene (defaults to `8`). `hpsMaxLights` must not be changed after the first scene with lighting is initialized.

    void hpsSetAmbientLight(HPSscene *scene, float* color);

Scenes that use the lighting extension have an `(r g b)` ambient light associated with them, set by this function.

     HPSnode *hpsAddLight(HPSnode *node, float* color, float intensity, float *direction, float spotAngle);

Adds a new light to the given node (or scene) with `(r g b)` `color`. `intensity` is the value associated with the brightness of the light. `direction` is an `(x y z)` vector that indicates the direction that the light is pointing. `spotAngle` indicates the angle that the light is spread over (defaulting to `0`, representing a non-spotlight source). An `HPSnode` is returned that can be moved, rotated, and sized like any other node.

    void hpsSetLightColor(HPSnode *node, float *color);

Sets the `(r g b)` color of the light.

    float *hpsLightColor(HPSnode *node);

Returns the `(r g b)` color of the light.

    void hpsSetLightIntensity(HPSnode *node, float intensity);

Sets the intensity of the light.

    float hpsLightIntensity(HPSnode *node);

Returns the intensity of the light.

    void hpsSetLightDirection(HPSnode *node, float *dir);

Sets the `(x y z)` direction of the light.

    float *hpsLightDirection(HPSnode *node);

Returns the `(x y z)` direction of the light.

    void hpsSetLightSpotAngle(HPSnode *node, float a);

Sets the angle over which the light is spread.

    float hpsLightSpotAngle(HPSnode *node);

Returns the angle over which the light is spread.

    float *hpsCurrentAmbientLight;

A pointer to the the `(r g b)` ambient light color of the scene currently being rendered.

     unsigned int *hpsNCurrentLights;

A pointer to the number of visible lights in the scene currently being rendered.

     float *hpsCurrentLightPositions;

A pointer to the array of packed `(x y z)` positions of the visible lights in the scene currently being rendered.

     float *hpsCurrentLightColors;

A pointer to the array of packed `(r g b)` colors of the visible lights in the scene currently being rendered.

     float *hpsCurrentLightIntensities;

A pointer to the array of intensities of the visible lights in the scene currently being rendered.

     float *hpsCurrentLightDirections;

A pointer to the array of packed `(x y z spotAngle)` directions and angles of the visible lights in the scene currently being rendered.

     unsigned int hpsLightPoolSize;

Every scene is given a pool from which to allocate lights, the size of which (at initialization) can be modified by setting `hpsLightPoolSize` (defaults to `1024`).


#### Writing your own extensions
New extensions can be created by making an HPSextension struct:

    struct HPSextension {
        void (*init)(void **);
        void (*preRender)(void *);
        void (*postRender)(void *);
        void (*visibleNode)(void *, HPSnode *node);
        void (*updateNode)(void *);
        void (*delete)(void *);
    };

All of these function pointers *must* be set. `NULL` pointers will be dereferenced with the expected consequences. See the file [`extensionTemplate.c`](https://github.com/AlexCharlton/Hyperscene/blob/master/extensionTemplate.c) for a bare-bones extension file.

`init` is called every time a scene is initialized and is passed a pointer to a pointer that is stored with the scene: `init` may set the value of this pointer to be some scene dependant data.

`preRender` is called with the scene’s extension data, during rendering, after which nodes are visible are determined but before they are rendered.

`postRender` is called with the scene’s extension data, after nodes have been rendered.

`visibleNode` is called with the scene’s extension data and a node that was created with a pointer to the extension in the place of the pipeline. This is called before rendering, when node visibility is being determined, on each visible node.

`updateNode` is called with the scene’s extension data and a node that was created with a pointer to the extension in the place of the pipeline. This is called while nodes are being updated. Only nodes that need to be updated (i.e., the have been moved, resized, or have had `hpsNodeNeedsUpdate` called) are passed to `updateNode`.

`delete` is called with the scene’s extension data, when the scene is deleted.


## Version history
### Version 0.4.0
* More control over camera viewport size, ratio, and position

### Version 0.3.0
* Improve distance-to-camera filtering
* Prefix camera style enum elements
* Turn `hpsCurrentCamera***` variables into functions
* Actually compile the lighting extension

### Version 0.2.0
* Separate camera updating from rendering
* `hpsAmbientLight` -> `hpsCurrentAmbientLight`
* Bug fixes

### Version 0.1.0
* Initial release

## Source repository
Source available on [GitHub](https://github.com/AlexCharlton/Hyperscene).

Bug reports and patches welcome! Bugs can be reported via GitHub or to alex.n.charlton at gmail.

## Author
Alex Charlton

## Licence
BSD
