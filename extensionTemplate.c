#include <stdlib.h>
#include <hyperscene.h>


void initX(void **data){
}

void deleteX(void *data){
}

void XpreRender(void *data){
}

void XpostRender(void *data){
}

void XvisibleNode(void *data, HPGnode *node){
}

void Xupdate(void *data){
}

HPGextension x = {initX,
                  XpreRender,
                  XpostRender,
                  XvisibleNode,
                  Xupdate,
                  deleteX};

HPGextension *X = &x; //external 
