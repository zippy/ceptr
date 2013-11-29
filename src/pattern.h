#include "ceptr.h"

#define PATTERN_GET_CHILDREN_COUNT(pat) (((PatternBody *)SKIP_ELEM_HEADER(pat))->children_count)
#define PATTERN_SET_CHILDREN_COUNT(pat,count) (((PatternBody *)SKIP_ELEM_HEADER(pat))->children_count=count)
#define PATTERN_GET_CHILDREN(pat) (&(((PatternBody *)SKIP_ELEM_HEADER(pat))->children))
//#define PATTERN_GET_SIZE(pat) (((PatternBody *)skip_elem_header(pat))->size)
//#define PATTERN_SET_SIZE(pat,s) (((PatternBody *)skip_elem_header(pat))->size = s)


size_t pattern_get_size(void *pattern_surface) {
//printf("pattern_get_size \n");
//    ElementSurface *es = (ElementSurface *)pattern_surface;
//    PatternBody *pb = (PatternBody *)skip_elem_header(pattern_surface);
//
//printf("  - name %d \n", es->name);
//printf("  - process_count %d \n", es->process_count);
//
//printf("  - size %d \n", pb->size);
//printf("  - child_count %d \n", pb->children_count);

    return ((PatternBody *) skip_elem_header(pattern_surface))->size;
}

void pattern_set_size(void *pattern_surface, size_t size) {
    ((PatternBody *) skip_elem_header(pattern_surface))->size = size;
}

size_t get_pattern_spec_size(ElementSurface *es) {
    return ELEMENT_HEADER_SIZE(es) + sizeof(PatternBody) - sizeof(Offset) + sizeof(Offset) * PATTERN_GET_CHILDREN_COUNT(es);
}
