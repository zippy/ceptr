#include "../ceptr.h"

size_t receptor_get_size(Receptor *r) {
    return 0;
}

void proc_receptor_instance_new() {

}

void receptor_init(Receptor *r) {
    Symbol newNoun = data_new_noun(r, r->receptorSpecXaddr, "RECEPTOR");
    size_table_set(newNoun, (sizeFunction)receptor_get_size);
    UntypedProcess processes = {INSTANCE_NEW, (voidVoidFn) proc_receptor_instance_new };
    ElementSurface specSurface = { newNoun, 1, processes};
    r->receptorSpecXaddr = data_new(r, newNoun, &specSurface,  element_header_size((void *)&specSurface));
}