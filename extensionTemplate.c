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

void XupdateNode(void *data, HPGnode *node){
}

HPGextension x = {initX,
                  XpreRender,
                  XpostRender,
                  XvisibleNode,
                  XupdateNode,
                  deleteX};

HPGextension *X = &x; //external 
