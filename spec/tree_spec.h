#include "../src/ceptr.h"


void testCreateTreeNodes() {
    /* test the creation of trees and the various function that give access to created data elements
       and basic tree structure navigation
     */
    Tnode *t = _t_new(0,TEST_SYMBOL,"hello",6);
    spec_is_long_equal(_t_size(t),(long)6);
    spec_is_equal(_t_children(t),0);
    spec_is_str_equal((char *)_t_surface(t),"hello");
    spec_is_ptr_equal(_t_parent(t),NULL);
    spec_is_ptr_equal(_t_root(t),t);
    spec_is_ptr_equal(_t_child(t,1),NULL);
    spec_is_equal(_t_symbol(t),TEST_SYMBOL);

    Tnode *t1 = _t_new(t,TEST_SYMBOL,"t1",3);
    spec_is_ptr_equal(_t_parent(t1),t);
    spec_is_equal(_t_children(t),1);
    spec_is_ptr_equal(_t_child(t,1),t1);
    spec_is_ptr_equal(_t_root(t1),t);

    Tnode *t2 = _t_new(t,TEST_SYMBOL,"t2",3);
    spec_is_ptr_equal(_t_parent(t2),t);
    spec_is_equal(_t_children(t),2);
    spec_is_ptr_equal(_t_child(t,2),t2);
    spec_is_ptr_equal(_t_root(t2),t);

    Tnode *t3 = _t_newi(t,99,101);
    spec_is_ptr_equal(_t_parent(t3),t);
    spec_is_equal(_t_children(t),3);
    spec_is_equal(*(int *)_t_surface(_t_child(t,3)),101);

    spec_is_ptr_equal(_t_next_sibling(t1),t2);
    spec_is_ptr_equal(_t_next_sibling(t),NULL);
    spec_is_ptr_equal(_t_next_sibling(t2),t3);
    spec_is_ptr_equal(_t_next_sibling(t3),NULL);

    Tnode *t4 = _t_new_root(TEST_SYMBOL);
    _t_add(t,t4);
    spec_is_equal(_t_children(t),4);
    spec_is_ptr_equal(_t_child(t,4),t4);

    _t_remove(t,t3);
    spec_is_equal(_t_children(t),3);
    spec_is_ptr_equal(_t_child(t,3),t4);
    spec_is_ptr_equal(_t_child(t,2),t2);

    _t_free(t);

}

void testTreeRealloc() {
    Tnode *ts[12];
    Tnode *t = _t_new(0,TEST_SYMBOL,"t",2);
    char tname[3];
    tname[0] = 't';
    tname[2] = 0;
    for (int i=0;i<12;i++){
	tname[1] = 'a'+i;
	ts[i] = _t_new(t,TEST_SYMBOL,tname,3);
    }
    spec_is_str_equal((char *)_t_surface(ts[11]),"tl");
    _t_free(t);
}

Tnode *_makeTestTree() {
    Tnode *t = _t_new(0,TEST_SYMBOL,"t",2);
    Tnode *t1 = _t_new(t,TEST_SYMBOL,"t1",3);
    Tnode *t2 = _t_new(t,TEST_SYMBOL,"t2",3);
    Tnode *t11 = _t_new(t1,TEST_SYMBOL,"t11",4);
    Tnode *t111 = _t_new(t11,TEST_SYMBOL,"t111",5);
    Tnode *t12 = _t_new(t1,TEST_SYMBOL,"t12",4);
    Tnode *t21 = _t_new(t2,TEST_SYMBOL,"t21",4);
    Tnode *t211 = _t_new(t21,TEST_SYMBOL,"t211",5);
    return t;
}

void testTreePath() {
    Tnode *t = _makeTestTree();

    int p0[] = {TREE_PATH_TERMINATOR};
    spec_is_ptr_equal(_t_get(t,p0),t);

    int p1[] = {1,TREE_PATH_TERMINATOR};
    spec_is_str_equal((char *)_t_get_surface(t,p1),"t1");

    int p2[] = {2,TREE_PATH_TERMINATOR};
    spec_is_str_equal((char *)_t_get_surface(t,p2),"t2");

    int p3[] = {2,1,1,TREE_PATH_TERMINATOR};
    spec_is_str_equal((char *)_t_get_surface(t,p3),"t211");

    int *path;
    path = _t_get_path(_t_get(t,p0));
    spec_is_path_equal(path,p0);
    free(path);
    path = _t_get_path(_t_get(t,p1));
    spec_is_path_equal(path,p1);
    free(path);
    path = _t_get_path(_t_get(t,p3));
    spec_is_path_equal(path,p3);
    free(path);

    p3[2] = 2;
    spec_is_ptr_equal(_t_get(t,p3),NULL);

    Tnode *tt = _makeTestTree();
    Tnode *t3 = _t_new(t,TEST_SYMBOL,&tt,sizeof(Tnode *));

    p1[0] = 3;
    spec_is_ptr_equal(*(Tnode **)_t_get_surface(t,p1),tt);

    int p4[] = {3,0,TREE_PATH_TERMINATOR};
    spec_is_ptr_equal(_t_get(t,p4),tt);

    int p5[] = {3,0,2,1,1,TREE_PATH_TERMINATOR};
    spec_is_str_equal((char *)_t_get_surface(t,p5),"t211");

    int p6[6];
    _t_path_parent(p6,p5);
    for(int i=0;i<3;i++) {
	spec_is_equal(p6[i],p5[i]);
    }
    spec_is_equal(p6[4],TREE_PATH_TERMINATOR);

    _t_path_parent(p6,p1);
    spec_is_equal(p6[0],TREE_PATH_TERMINATOR);

    spec_is_true(_t_path_equal(p3,p3));
    spec_is_true(!_t_path_equal(p4,p3));
    spec_is_true(!_t_path_equal(p0,p3));

    spec_is_equal(_t_path_depth(p3),3);

    char buf[255];
    spec_is_str_equal(_t_sprint_path(p5,buf),"/3/0/2/1/1");

    spec_is_equal(_t_node_index(_t_child(t,1)),1);
    spec_is_equal(_t_node_index(_t_child(t,2)),2);
    spec_is_equal(_t_node_index(_t_child(t,3)),3);
    spec_is_equal(_t_node_index(t),0);

    int pp[10];
    _t_pathcpy(pp,p5);
    spec_is_path_equal(pp,p5);

    _t_free(t);
    _t_free(tt);
}

void testTree() {
    testCreateTreeNodes();
    testTreeRealloc();
    testTreePath();
}
