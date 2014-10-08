extern HPSextension *hpsLighting;

extern unsigned int hpsLightPoolSize;
extern unsigned int hpsMaxLights;

extern unsigned int *hpsNCurrentLights;
extern float *hpsCurrentLightPositions;
extern float *hpsCurrentLightColors;
extern float *hpsCurrentLightIntensities;
extern float *hpsCurrentLightDirections;

HPSnode *hpsAddLight(HPSscene *scene, float* color, float i, float *direction, float spotAngle);

void hpsSetLightColor(HPSnode *node, float *color);

float *hpsLightColor(HPSnode *node);

void hpsSetLightIntensity(HPSnode *node, float i);

float hpsLightIntensity(HPSnode *node);

void hpsSetLightDirection(HPSnode *node, float *dir);

float *hpsLightDirection(HPSnode *node);

void hpsSetLightSpotAngle(HPSnode *node, float a);

float hpsLightSpotAngle(HPSnode *node);

void hpsSetAmbientLight(HPSscene *scene, float* color);

float *hpsAmbientLight(HPSscene *scene);
