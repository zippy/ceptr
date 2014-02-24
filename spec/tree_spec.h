#include "../src/ceptr.h"

void testNewTreeNode() {
    Tnode *t = _t_new(0,CSTRING_NOUN,"hello",6);
    spec_is_equal(_t_children(t),0);
    spec_is_str_equal((char *)_t_surface(t),"hello");
    spec_is_ptr_equal(_t_parent(t),NULL);
    spec_is_ptr_equal(_t_child(t,1),NULL);
    spec_is_equal(_t_noun(t),CSTRING_NOUN);

    Tnode *t1 = _t_new(t,CSTRING_NOUN,"t1",3);
    spec_is_ptr_equal(_t_parent(t1),t);
    spec_is_equal(_t_children(t),1);
    spec_is_ptr_equal(_t_child(t,1),t1);

    Tnode *t2 = _t_new(t,CSTRING_NOUN,"t2",3);
    spec_is_ptr_equal(_t_parent(t2),t);
    spec_is_equal(_t_children(t),2);
    spec_is_ptr_equal(_t_child(t,2),t2);

    Tnode *t3 = _t_newi(t,99,101);
    spec_is_ptr_equal(_t_parent(t3),t);
    spec_is_equal(_t_children(t),3);
    spec_is_equal(_t_child(t,3)->surface,101);

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

Tnode *_makeTestTree() {
    Tnode *t = _t_new(0,CSTRING_NOUN,"t",2);
    Tnode *t1 = _t_new(t,CSTRING_NOUN,"t1",3);
    Tnode *t2 = _t_new(t,CSTRING_NOUN,"t2",3);
    spec_is_equal(_t_children(t),2);

    Tnode *t11 = _t_new(t1,CSTRING_NOUN,"t11",4);
    Tnode *t12 = _t_new(t1,CSTRING_NOUN,"t12",4);
    Tnode *t21 = _t_new(t2,CSTRING_NOUN,"t21",4);
    Tnode *t211 = _t_new(t21,CSTRING_NOUN,"t211",5);
    return t;
}

void testTreePath() {
    Tnode *t = _makeTestTree();

    int p1[] = {1,TREE_PATH_TERMINATOR};
    spec_is_str_equal((char *)_t_get_surface(t,p1),"t1");

    int p2[] = {2,TREE_PATH_TERMINATOR};
    spec_is_str_equal((char *)_t_get_surface(t,p2),"t2");

    int p3[] = {2,1,1,TREE_PATH_TERMINATOR};
    spec_is_str_equal((char *)_t_get_surface(t,p3),"t211");

    p3[2] = 2;
    spec_is_ptr_equal(_t_get(t,p3),NULL);

    Tnode *tt = _makeTestTree();
    Tnode *t3 = _t_new(t,99,&tt,sizeof(Tnode *)); //99 is a fake symbol

    p1[0] = 3;
    spec_is_ptr_equal(*(Tnode **)_t_get_surface(t,p1),tt);

    int p4[] = {3,0,TREE_PATH_TERMINATOR};
    spec_is_ptr_equal(_t_get(t,p4),tt);

    int p5[] = {3,0,2,1,1,TREE_PATH_TERMINATOR};
    spec_is_str_equal((char *)_t_get_surface(t,p5),"t211");

    _t_free(t);
    _t_free(tt);
}

typedef struct {
    int len;
    char buf[20];
} iterTest;


void iterfunc(void *s,int id,void *param) {
    iterTest *i = param;
    int l = strlen((char *)s);
    strcpy(i->buf+i->len,(char *)s);
    i->len += l;
    i->buf[i->len] = 0;
}

void itersfunc(Tnode *t,int id,void *param) {
    char *s = _t_surface(t);
    iterTest *i = param;
    int l = strlen((char *)s);
    strcpy(i->buf+i->len,(char *)s);
    i->len += l;
    i->buf[i->len] = 0;
}

void testTreeUtils() {
    Tnode *t = _makeTestTree();
    spec_is_str_equal((char *)_t_get_child_surface(t,2),"t2");

    Tnode *t2 = _t_get_child(t,2);
    spec_is_str_equal((char *)_t_surface(t2),"t2");

    iterTest i;
    i.len = 0;
    _t_iter_children_surface(t,iterfunc,&i);
    spec_is_str_equal(i.buf,"t1t2");


    i.len = 0;
    _t_iter_children(t,itersfunc,&i);
    spec_is_str_equal(i.buf,"t1t2");

    _t_free(t);

    t = _t_new_root();
    spec_is_equal(_t_children(t),0);
    _t_free(t);
}

void testTree() {
    testNewTreeNode();
    testTreeRealloc();
    testTreePath();
    testTreeUtils();
}
