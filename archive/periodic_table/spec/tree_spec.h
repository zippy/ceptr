#include "../src/ceptr.h"

void testNewTreeNode() {
    T *t = _t_new(0,CSTRING_NOUN,"hello",6);
    spec_is_long_equal(_t_size(t),(long)6);
    spec_is_equal(_t_children(t),0);
    spec_is_str_equal((char *)_t_surface(t),"hello");
    spec_is_ptr_equal(_t_parent(t),NULL);
    spec_is_ptr_equal(_t_child(t,1),NULL);
    spec_is_equal(_t_noun(t),CSTRING_NOUN);

    T *t1 = _t_new(t,CSTRING_NOUN,"t1",3);
    spec_is_ptr_equal(_t_parent(t1),t);
    spec_is_equal(_t_children(t),1);
    spec_is_ptr_equal(_t_child(t,1),t1);

    T *t2 = _t_new(t,CSTRING_NOUN,"t2",3);
    spec_is_ptr_equal(_t_parent(t2),t);
    spec_is_equal(_t_children(t),2);
    spec_is_ptr_equal(_t_child(t,2),t2);

    T *t3 = _t_newi(t,99,101);
    spec_is_ptr_equal(_t_parent(t3),t);
    spec_is_equal(_t_children(t),3);
    spec_is_equal(*(int *)_t_surface(_t_child(t,3)),101);

    T *t4 = _t_newp(t,99,testNewTreeNode);
    spec_is_ptr_equal(_t_parent(t4),t);
    spec_is_equal(_t_children(t),4);
    spec_is_ptr_equal(*(int **)_t_surface(t4),(int *)testNewTreeNode);

    _t_free(t);
}

void testTreeRealloc() {
    T *ts[12];
    T *t = _t_new(0,CSTRING_NOUN,"t",2);
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

T *_makeTestTree() {
    T *t = _t_new(0,CSTRING_NOUN,"t",2);
    T *t1 = _t_new(t,CSTRING_NOUN,"t1",3);
    T *t2 = _t_new(t,CSTRING_NOUN,"t2",3);
    T *t11 = _t_new(t1,CSTRING_NOUN,"t11",4);
    T *t111 = _t_new(t11,CSTRING_NOUN,"t111",5);
    T *t12 = _t_new(t1,CSTRING_NOUN,"t12",4);
    T *t21 = _t_new(t2,CSTRING_NOUN,"t21",4);
    T *t211 = _t_new(t21,CSTRING_NOUN,"t211",5);
    return t;
}

void testTreePath() {
    T *t = _makeTestTree();

    int p0[] = {TREE_PATH_TERMINATOR};
    spec_is_ptr_equal(_t_get(t,p0),t);

    int p1[] = {1,TREE_PATH_TERMINATOR};
    spec_is_str_equal((char *)_t_get_surface(t,p1),"t1");

    int p2[] = {2,TREE_PATH_TERMINATOR};
    spec_is_str_equal((char *)_t_get_surface(t,p2),"t2");

    int p3[] = {2,1,1,TREE_PATH_TERMINATOR};
    spec_is_str_equal((char *)_t_get_surface(t,p3),"t211");

    p3[2] = 2;
    spec_is_ptr_equal(_t_get(t,p3),NULL);

    T *tt = _makeTestTree();
    T *t3 = _t_new(t,99,&tt,sizeof(T *)); //99 is a fake symbol

    p1[0] = 3;
    spec_is_ptr_equal(*(T **)_t_get_surface(t,p1),tt);

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

void itersfunc(T *t,int id,void *param) {
    char *s = _t_surface(t);
    iterTest *i = param;
    int l = strlen((char *)s);
    strcpy(i->buf+i->len,(char *)s);
    i->len += l;
    i->buf[i->len] = 0;
}

void testTreeChildUtils() {
    T *t = _makeTestTree();
    spec_is_str_equal((char *)_t_get_child_surface(t,2),"t2");

    T *t2 = _t_get_child(t,2);
    spec_is_str_equal((char *)_t_surface(t2),"t2");
    _t_free(t);
}

void testTreeIterate() {
    T *t = _makeTestTree();

    iterTest i;
    i.len = 0;
    _t_iter_children_surface(t,iterfunc,&i);
    spec_is_str_equal(i.buf,"t1t2");


    i.len = 0;
    _t_iter_children(t,itersfunc,&i);
    spec_is_str_equal(i.buf,"t1t2");

    _t_free(t);
}

void testTreeNewRoot(){
    T *t = _t_new_root(-99);
    spec_is_equal(_t_children(t),0);
    spec_is_equal(_t_noun(t),-99);
    _t_free(t);
}

void testTreeBecome() {
    T *t = _t_newi(0,INTEGER_NOUN,314);
    T *x = _t_new(t,CSTRING_NOUN,"goodbye",8);
    T *y = _t_new(t,CSTRING_NOUN,"doggy",6);
    T *z = _t_new(y,CSTRING_NOUN,"fish",5);
    spec_is_equal(_t_children(t),2);
    spec_is_equal(*(int *)_t_surface(t),314);
    spec_is_str_equal((char *)_t_get_child_surface(t,1),"goodbye");
    _t_become(t,y);
    spec_is_equal(_t_children(t),1);
    spec_is_str_equal((char *)_t_surface(t),"doggy");
    spec_is_str_equal((char *)_t_get_child_surface(t,1),"fish");
    _t_free(t);
}

void testTreeBuild() {
    T *i = _t_newi(0,BOOLEAN_NOUN,TRUE_VALUE);
    _t_newi(i,PTR_NOUN,1);
    T *r = _t_newi(0,RUNTREE_NOUN,0);
    _t_newi(r,INTEGER_NOUN,42);

    T *n = _t_build(i,r);

    spec_is_equal(_t_noun(n),BOOLEAN_NOUN);
    spec_is_equal(*(int *)_t_surface(n),TRUE_VALUE);

    T *t = _t_get_child(n,1);
    spec_is_equal(_t_noun(t),INTEGER_NOUN);
    spec_is_equal(*(int *)_t_surface(t),42);
    _t_free(i);
    _t_free(r);
    _t_free(n);
}

void testTreePathNext() {
    T *t = _makeTestTree();
    int p[20];
    T *r;
    p[0] = TREE_PATH_TERMINATOR;
    //t1
    r = _t_next_df(t,p);spec_is_ptr_equal(r,_t_get(t,p));
    spec_is_equal(p[1],TREE_PATH_TERMINATOR);
    //t11
    r = _t_next_df(t,p);spec_is_ptr_equal(r,_t_get(t,p));
    spec_is_equal(p[0],1); spec_is_equal(p[1],1); spec_is_equal(p[2],TREE_PATH_TERMINATOR);
    //t111
    r = _t_next_df(t,p);spec_is_ptr_equal(r,_t_get(t,p));
    spec_is_equal(p[0],1); spec_is_equal(p[1],1);spec_is_equal(p[2],1); spec_is_equal(p[3],TREE_PATH_TERMINATOR);
    //t12
    r = _t_next_df(t,p);spec_is_ptr_equal(r,_t_get(t,p));
    spec_is_equal(p[0],1); spec_is_equal(p[1],2); spec_is_equal(p[2],TREE_PATH_TERMINATOR);
    //t2
    r = _t_next_df(t,p);spec_is_ptr_equal(r,_t_get(t,p));
    spec_is_equal(p[0],2); spec_is_equal(p[1],TREE_PATH_TERMINATOR);
    //t21
    r = _t_next_df(t,p);spec_is_ptr_equal(r,_t_get(t,p));
    spec_is_equal(p[0],2); spec_is_equal(p[1],1); spec_is_equal(p[2],TREE_PATH_TERMINATOR);
    //t211
    r = _t_next_df(t,p);spec_is_ptr_equal(r,_t_get(t,p));
    spec_is_equal(p[0],2); spec_is_equal(p[1],1);spec_is_equal(p[2],1); spec_is_equal(p[3],TREE_PATH_TERMINATOR);
    r = _t_next_df(t,p);
    spec_is_ptr_equal(r,NULL);
}

void testTreeWalk() {
    T *t = _makeTestTree();
    T *i;
    TreeWalker w;
    _t_init_walk(t,&w,WALK_DEPTH_FIRST);
    i = _t_walk(t,&w);
    spec_is_equal(w.p[0],1); spec_is_equal(w.p[1],TREE_PATH_TERMINATOR);
    spec_is_str_equal((char *)_t_surface(i),"t1");
    i = _t_walk(t,&w);
    spec_is_equal(w.p[0],1); spec_is_equal(w.p[1],1); spec_is_equal(w.p[2],TREE_PATH_TERMINATOR);
    spec_is_str_equal((char *)_t_surface(i),"t11");
    i = _t_walk(t,&w);
    spec_is_str_equal((char *)_t_surface(i),"t111");
    i = _t_walk(t,&w);
    spec_is_str_equal((char *)_t_surface(i),"t12");
    i = _t_walk(t,&w);
    spec_is_str_equal((char *)_t_surface(i),"t2");
    i = _t_walk(t,&w);
    spec_is_str_equal((char *)_t_surface(i),"t21");
    i = _t_walk(t,&w);
    spec_is_str_equal((char *)_t_surface(i),"t211");
    i = _t_walk(t,&w);
    spec_is_ptr_equal(i,(void *)0);
/*
    _t_init_walk(t,&w,WALK_BREADTH_FIRST);
    i = _t_walk(t,&w);
    spec_is_str_equal((char *)_t_surface(i),"t1");
    i = _t_walk(t,&w);
    spec_is_str_equal((char *)_t_surface(i),"t2");
     i = _t_walk(t,&w);
    spec_is_str_equal((char *)_t_surface(i),"t11");
    i = _t_walk(t,&w);
    spec_is_str_equal((char *)_t_surface(i),"t12");
    i = _t_walk(t,&w);
    spec_is_str_equal((char *)_t_surface(i),"t21");
    i = _t_walk(t,&w);
    spec_is_str_equal((char *)_t_surface(i),"t111");
    i = _t_walk(t,&w);
    spec_is_str_equal((char *)_t_surface(i),"t211");
    i = _t_walk(t,&w);
    spec_is_ptr_equal(i,(void *)0);*/

}

void testTreeNounEQ() {
    T *t = _makeTestTree();
    T *t1 = _makeTestTree();
    spec_is_true(_t_nouns_eq(t,t1));
    _t_newi(t,PTR_NOUN,1);
    spec_is_true(!_t_nouns_eq(t,t1));
    _t_newi(t1,PTR_NOUN,2);
    spec_is_true(_t_nouns_eq(t,t1));
    _t_newi(t,INTEGER_NOUN,1);
    _t_newi(t1,PTR_NOUN,2);
    spec_is_true(!_t_nouns_eq(t,t1));
    _t_free(t);
    _t_free(t1);
}

void testTreeMap() {
    char *mt  = "(META:INTEGER (META:CSTRING) (META:BOOLEAN (META:INTEGER)))";
    T *t = _t_parse(mt);
    T *t1 = _t_parse(mt);
    spec_is_true(_t_map(t,t1,meta_eq_mapfn));
    _t_newi(t,META_NOUN,1);
    spec_is_true(!_t_map(t,t1,meta_eq_mapfn));
    _t_newi(t1,META_NOUN,1);
    spec_is_true(_t_map(t,t1,meta_eq_mapfn));
    _t_newi(t,META_NOUN,INTEGER_NOUN);
    T *x = _t_newi(t1,META_NOUN,PTR_NOUN);
    spec_is_true(!_t_map(t,t1,meta_eq_mapfn));
    *(int *)(_t_surface(x)) = INTEGER_NOUN;
    spec_is_true(_t_map(t,t1,meta_eq_mapfn));
    x->noun = INTEGER_NOUN;
    spec_is_true(!_t_map(t,t1,meta_eq_mapfn));
    _t_free(t);
    _t_free(t1);
}


void testTree() {
    sys_defs_init();
    testNewTreeNode();
    testTreeRealloc();
    testTreePath();
    testTreeChildUtils();
    testTreeNewRoot();
    testTreeIterate();
    testTreeBecome();
    testTreeBuild();
    testTreePathNext();
    testTreeWalk();
    testTreeNounEQ();
    testTreeMap();
    sys_defs_free();
}
