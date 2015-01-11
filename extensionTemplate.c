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

void XvisibleNode(void *data, HPSnode *node){
}

void XupdateNode(void *data, HPSnode *node){
}

HPSextension x = {initX,
                  XpreRender,
                  XpostRender,
                  XvisibleNode,
                  XupdateNode,
                  deleteX};

HPSextension *X = &x; //external 
