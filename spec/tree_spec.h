#include "../src/ceptr.h"

void testNewTreeNode() {
    Tnode *t = _t_new(0,CSTRING_NOUN,"hello",6);
    spec_is_equal(_t_children(t),0);
    spec_is_str_equal((char *)_t_surface(t),"hello");
    spec_is_ptr_equal(_t_parent(t),NULL);
    spec_is_ptr_equal(_t_child(t,1),NULL);

    Tnode *t1 = _t_new(t,CSTRING_NOUN,"t1",3);
    spec_is_ptr_equal(_t_parent(t1),t);
    spec_is_equal(_t_children(t),1);
    spec_is_ptr_equal(_t_child(t,1),t1);

    Tnode *t2 = _t_new(t,CSTRING_NOUN,"t2",3);
    spec_is_ptr_equal(_t_parent(t2),t);
    spec_is_equal(_t_children(t),2);
    spec_is_ptr_equal(_t_child(t,2),t2);

    _t_free(t);
}

void testTreeRealloc() {
    Tnode *ts[12];
    Tnode *t = _t_new(0,CSTRING_NOUN,"t",2);
    char tname[3];
    tname[0] = 't';
    tname[2] = 0;
    for (int i=0;i<12;i++){
	tname[1] = 'a'+i;
	ts[i] = _t_new(t,CSTRING_NOUN,tname,3);
    }
    spec_is_str_equal((char *)_t_surface(ts[11]),"tl");
    _t_free(t);
}

void testTreePath() {
    Tnode *t = _t_new(0,CSTRING_NOUN,"t",2);
    Tnode *t1 = _t_new(t,CSTRING_NOUN,"t1",3);
    Tnode *t2 = _t_new(t,CSTRING_NOUN,"t2",3);
    spec_is_equal(_t_children(t),2);

    Tnode *t11 = _t_new(t1,CSTRING_NOUN,"t11",4);
    Tnode *t12 = _t_new(t1,CSTRING_NOUN,"t12",4);
    Tnode *t21 = _t_new(t2,CSTRING_NOUN,"t21",4);
    Tnode *t211 = _t_new(t21,CSTRING_NOUN,"t211",5);

    int p[4];
    p[0] = 1;
    p[1] = TREE_PATH_TERMINATOR;
    spec_is_ptr_equal(_t_get(t,p),t1);
    p[0] = 2;
    spec_is_ptr_equal(_t_get(t,p),t2);
    p[0] = 2;p[1]=1;p[2]=1;p[3]= TREE_PATH_TERMINATOR;
    spec_is_ptr_equal(_t_get(t,p),t211);
    p[0] = 2;p[1]=1;p[2]=2;p[3]= TREE_PATH_TERMINATOR;
    spec_is_ptr_equal(_t_get(t,p),NULL);
    _t_free(t);
}

void testTree() {
    testNewTreeNode();
    testTreeRealloc();
    testTreePath();
}
