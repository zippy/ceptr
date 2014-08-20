#include "../src/ceptr.h"
#include "../src/receptor.h"

void testCreateTreeNodes() {
    /* test the creation of trees and the various function that give access to created data elements
       and basic tree structure navigation
     */
    Tnode *t, *t1, *t2, *t3, *t4;

    t = _t_new(0,TEST_SYMBOL,"hello",6);
    spec_is_long_equal(_t_size(t),(long)6);
    spec_is_equal(_t_children(t),0);
    spec_is_str_equal((char *)_t_surface(t),"hello");
    spec_is_ptr_equal(_t_parent(t),NULL);
    spec_is_ptr_equal(_t_root(t),t);
    spec_is_ptr_equal(_t_child(t,1),NULL);
    spec_is_equal(_t_symbol(t),TEST_SYMBOL);

    t1 = _t_new(t,TEST_SYMBOL,"t1",3);
    spec_is_ptr_equal(_t_parent(t1),t);
    spec_is_equal(_t_children(t),1);
    spec_is_ptr_equal(_t_child(t,1),t1);
    spec_is_ptr_equal(_t_root(t1),t);

    t2 = _t_new(t,TEST_SYMBOL,"t2",3);
    spec_is_ptr_equal(_t_parent(t2),t);
    spec_is_equal(_t_children(t),2);
    spec_is_ptr_equal(_t_child(t,2),t2);
    spec_is_ptr_equal(_t_root(t2),t);

    t3 = _t_newi(t,99,101);
    spec_is_ptr_equal(_t_parent(t3),t);
    spec_is_equal(_t_children(t),3);
    spec_is_equal(*(int *)_t_surface(_t_child(t,3)),101);

    spec_is_ptr_equal(_t_next_sibling(t1),t2);
    spec_is_ptr_equal(_t_next_sibling(t),NULL);
    spec_is_ptr_equal(_t_next_sibling(t2),t3);
    spec_is_ptr_equal(_t_next_sibling(t3),NULL);

    t4 = _t_new_root(TEST_SYMBOL);
    _t_add(t,t4);
    spec_is_equal(_t_children(t),4);
    spec_is_ptr_equal(_t_child(t,4),t4);

    _t_remove(t,t3);
    _t_free(t3);  // remove doesn't free the memory of the removed node
    spec_is_equal(_t_children(t),3);
    spec_is_ptr_equal(_t_child(t,3),t4);
    spec_is_ptr_equal(_t_child(t,2),t2);

    _t_free(t);

}

void testTreeOrthogonal() {
    Tnode *t = _t_newi(0,TEST_SYMBOL,1234);
    Tnode *t2 = _t_new_root(TEST_SYMBOL2);
    Tnode *o = _t_newt(t,TEST_TREE_SYMBOL,t2);
    char buf[2000];
    __t_dump(0,t,0,buf);
    spec_is_str_equal(buf," (TEST_SYMBOL:1234 (TEST_TREE_SYMBOL:{ (TEST_SYMBOL2)}))");
    _t_free(t);
}


void testTreeRealloc() {
    Tnode *ts[12];
    Tnode *t = _t_new(0,TEST_SYMBOL,"t",2);
    char tname[3];
    int i;
    tname[0] = 't';
    tname[2] = 0;
    for (i=0;i<12;i++){
	tname[1] = 'a'+i;
	ts[i] = _t_new(t,TEST_SYMBOL,tname,3);
    }
    spec_is_str_equal((char *)_t_surface(ts[11]),"tl");
    _t_free(t);
}

Tnode *_makeTestTree() {
    Tnode *t = _t_new(0,TEST_STR_SYMBOL,"t",2);
    Tnode *t1 = _t_new(t,TEST_STR_SYMBOL,"t1",3);
    Tnode *t2 = _t_new(t,TEST_STR_SYMBOL,"t2",3);
    Tnode *t11 = _t_new(t1,TEST_STR_SYMBOL,"t11",4);
    Tnode *t111 = _t_new(t11,TEST_STR_SYMBOL,"t111",5);
    Tnode *t12 = _t_new(t1,TEST_STR_SYMBOL,"t12",4);
    Tnode *t21 = _t_new(t2,TEST_STR_SYMBOL,"t21",4);
    Tnode *t211 = _t_new(t21,TEST_STR_SYMBOL,"t211",5);
    return t;
}

void testTreePath() {
    Tnode *t = _makeTestTree();
    int *path;
    int i;
    char buf[255];

    int p0[] = {TREE_PATH_TERMINATOR};
    int p1[] = {1,TREE_PATH_TERMINATOR};
    int p2[] = {2,TREE_PATH_TERMINATOR};
    int p3[] = {2,1,1,TREE_PATH_TERMINATOR};
    int p4[] = {3,0,TREE_PATH_TERMINATOR};
    int p5[] = {3,0,2,1,1,TREE_PATH_TERMINATOR};
    int p6[6];
    int pp[10];

    Tnode *tt, *t3;

    spec_is_ptr_equal(_t_get(t,p0),t);

    spec_is_str_equal((char *)_t_get_surface(t,p1),"t1");

    spec_is_str_equal((char *)_t_get_surface(t,p2),"t2");

    spec_is_str_equal((char *)_t_get_surface(t,p3),"t211");

    path = _t_get_path(_t_get(t,p0));
    spec_is_path_equal(path,p0);
    free(path);
    path = _t_get_path(_t_get(t,p1));
    spec_is_path_equal(path,p1);
    free(path);
    path = _t_get_path(_t_get(t,p3));
    spec_is_path_equal(path,p3);
    free(path);

    int px[] = {2,1,TREE_PATH_TERMINATOR};
    path = _t_get_path(_t_get(t,px));
    spec_is_path_equal(path,px);
    free(path);

    p3[2] = 2;
    spec_is_ptr_equal(_t_get(t,p3),NULL);

    tt = _makeTestTree();
    t3 = _t_new(t,TEST_SYMBOL,&tt,sizeof(Tnode *));

    p1[0] = 3;
    spec_is_ptr_equal(*(Tnode **)_t_get_surface(t,p1),tt);

    spec_is_ptr_equal(_t_get(t,p4),tt);

    spec_is_str_equal((char *)_t_get_surface(t,p5),"t211");

    _t_path_parent(p6,p5);
    for(i=0;i<3;i++) {
	spec_is_equal(p6[i],p5[i]);
    }
    spec_is_equal(p6[4],TREE_PATH_TERMINATOR);

    _t_path_parent(p6,p1);
    spec_is_equal(p6[0],TREE_PATH_TERMINATOR);

    spec_is_true(_t_path_equal(p3,p3));
    spec_is_true(!_t_path_equal(p4,p3));
    spec_is_true(!_t_path_equal(p0,p3));

    spec_is_equal(_t_path_depth(p3),3);

    spec_is_str_equal(_t_sprint_path(p5,buf),"/3/0/2/1/1");

    spec_is_equal(_t_node_index(_t_child(t,1)),1);
    spec_is_equal(_t_node_index(_t_child(t,2)),2);
    spec_is_equal(_t_node_index(_t_child(t,3)),3);
    spec_is_equal(_t_node_index(t),0);

    _t_pathcpy(pp,p5);
    spec_is_path_equal(pp,p5);

    _t_free(t);
    _t_free(tt);
}

void testTreeClone() {
    Tnode *t = _makeTestTree();
    Tnode *c = _t_clone(t);

    spec_is_true(t!=c);
    spec_is_equal(_t_children(c),_t_children(t));

    _t_free(t);
    _t_free(c);
}

void testTreeModify() {
    Tnode *t = _makeTestTree();
    Tnode *x = _t_newi(0,TEST_SYMBOL,123);
    char buf[2000];
    _t_replace(t,1,x);
    spec_is_ptr_equal(_t_child(t,1),x);
    spec_is_ptr_equal(_t_parent(x),t);

    __t_dump(0,t,0,buf);
    spec_is_str_equal(_td(0,t)," (TEST_STR_SYMBOL:t (TEST_SYMBOL:123) (TEST_STR_SYMBOL:t2 (TEST_STR_SYMBOL:t21 (TEST_STR_SYMBOL:t211))))");
    Tnode *y = _t_detach(t,1);
    spec_is_ptr_equal(_t_parent(y),NULL);
    __t_dump(0,t,0,buf);
    spec_is_str_equal(buf," (TEST_STR_SYMBOL:t (TEST_STR_SYMBOL:t2 (TEST_STR_SYMBOL:t21 (TEST_STR_SYMBOL:t211))))");
    spec_is_ptr_equal(x,y);

    Tnode *z = _t_new(0,TEST_STR_SYMBOL,"fish",5);
    _t_morph(x,z);
    __t_dump(0,x,0,buf);
    spec_is_str_equal(buf," (TEST_STR_SYMBOL:fish)");

    int i = 789;
    __t_morph(x,TEST_SYMBOL,&i,sizeof(int),0);
    __t_dump(0,x,0,buf);
    spec_is_str_equal(buf," (TEST_SYMBOL:789)");

    _t_free(t);
    _t_free(x);
    _t_free(z);
}

void testTree() {
    testCreateTreeNodes();
    testTreeOrthogonal();
    testTreeRealloc();
    testTreePath();
    testTreeClone();
    testTreeModify();
}
