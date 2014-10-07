extern HPSextension *hpsLighting;

extern unsigned int hpsMaxLights;

extern unsigned int *hpsNCurrentLights;
extern float *hpsCurrentLightPositions;
extern float *hpsCurrentLightColors;
extern float *hpsCurrentLightIntensities;
extern float *hpsCurrentLightDirections;

HPSnode *hpsAddLight(HPSscene *scene, float* color, float i, float *direction, float spotAngle);

void hpsSetLightColor(HPSnode *node, float* color);

void hpsSetLightIntensity(HPSnode *node, float i);

void hpsSetLightDirection(HPSnode *node, float* dir);

void hpsSetLightSpotAngle(HPSnode *node, float a);

void hpsSetAmbientLight(HPSscene *scene, float* color);

float *hpsAmbientLight(HPSscene *scene);
