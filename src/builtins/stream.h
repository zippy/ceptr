#include "../ceptr.h"


typedef struct {
    int children_count;
    Offset children;
} StreamBody;

#define STREAM_GET_CHILDREN_COUNT(str) (((StreamBody *)SKIP_ELEM_HEADER(str))->children_count)
#define STREAM_SET_CHILDREN_COUNT(str,count) (((StreamBody *)SKIP_ELEM_HEADER(str))->children_count=count)
#define STREAM_GET_CHILDREN(str) (&(((StreamBody *)SKIP_ELEM_HEADER(str))->children))

size_t stream_get_size(Receptor *r) {
    return 0;
}


void proc_stream_instance_new(Receptor *r) {

}

//
//Xaddr preop_new_stream(Receptor *r, char *label, int child_count, Xaddr *children, int processCount, Process *processes) {
//    char ss[BUFFER_SIZE];
//    memset(ss, 0, BUFFER_SIZE);
//    Symbol newNoun = init_element(r, label, r->patternSpecXaddr, (ElementSurface *) ss, processCount, processes);
//
//    Xaddr foo;
//    return foo;
//
//}



void stream_init(Receptor *r){
    Symbol newNoun = data_new_noun(r, r->cspecXaddr, "STREAM");
    size_table_set(newNoun, (sizeFunction)stream_get_size);
    UntypedProcess processes = {INSTANCE_NEW, (voidVoidFn) proc_stream_instance_new };
    ElementSurface specSurface = { newNoun, 1, processes};
    r->streamSpecXaddr = data_new(r, newNoun, &specSurface,  element_header_size((void *)&specSurface));
}
