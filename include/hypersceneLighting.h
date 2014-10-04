typedef struct material HPGmaterial;

extern HPGextension *hpgLighting;

extern unsigned int hpgMaxLights;

extern unsigned int *hpgNCurrentLights;
extern float *hpgCurrentLightPositions;
extern float *hpgCurrentLightColors;
extern float *hpgCurrentLightIntensities;
extern float *hpgCurrentLightDirections;

HPGnode *hpgAddLight(HPGscene *scene, float* color, float i, float *direction, float spotAngle);

void hpgSetLightColor(HPGnode *node, float* color);

void hpgSetLightIntensity(HPGnode *node, float i);

void hpgSetLightDirection(HPGnode *node, float* dir);

void hpgSetLightSpotAngle(HPGnode *node, float a);

void hpgSetAmbientLight(HPGscene *scene, float* color);

float *hpgAmbientLight(HPGscene *scene);
