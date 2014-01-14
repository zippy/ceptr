////FIXME: this is needs to be implemented as a scape, not a linear scan of all Xaddrs!!

//
//int getOffset(ElementSurface *ps, Symbol name) {
//    Offset *o = PATTERN_GET_CHILDREN(ps);
//    while (!NULL_XADDR(o->noun)) {
//        if (o->noun.key == name) {
//            return o->offset;
//        }
//        o++;
//    }
//    raise_error2("offset not found for: %d in getOffset for patternSpec %d\n", name, ps->name);
//}
//





// #define SYMBOL_PATH_TERMINATOR 0xFFFF



//
//ElementSurface *walk_path(Receptor *r, Xaddr xaddr, Symbol *path, int *offset) {
//    ElementSurface *ps = _get_noun_pattern_spec(r, xaddr.noun);
//    *offset = 0;
//    int i = 0;
//    while (path[i] != SYMBOL_PATH_TERMINATOR) {
//        *offset += getOffset(ps, path[i]);
//        ps = _get_noun_pattern_spec(r, path[i]);
//        i++;
//    }
//    return ps;
//}


//
//void *preop_set_by_path(Receptor *r, Xaddr xaddr, Symbol *path, void *value) {
//    int offset;
//    ElementSurface *ps = walk_path(r, xaddr, path, &offset);
//    void *surface = &r->data.cache[xaddr.key + offset];
//    return memcpy(surface, value, pattern_get_size(ps));
//}
//
//void *preop_get_by_path(Receptor *r, Xaddr xaddr, Symbol *path) {
//    int offset;
//    walk_path(r, xaddr, path, &offset);
//    return &r->data.cache[xaddr.key + offset];
//}



//
//void testSymbolPath() {
//    Receptor tr; init(&tr); Receptor *r = &tr;
//    int myLine[4] = {1, 2, 3, 4};
//    Symbol inTheSand = preop_new_noun(r, r->linePatternSpecXaddr, "in the sand");
//    Xaddr itsLine = preop_new(r, inTheSand, &myLine);
//    Symbol B = getSymbol(r, "B");
//    Symbol Y = getSymbol(r, "Y");
//    Symbol path[3] = {B, Y, SYMBOL_PATH_TERMINATOR};
//    int *val;
//    int seven = 7;
//
//    val = preop_get_by_path(r, itsLine, path);
//    spec_is_true(*val == 4);
//
//    preop_set_by_path(r, itsLine, path, &seven);
//    val = preop_get_by_path(r, itsLine, path);
//    spec_is_true(*val == 7);
//
//    void *surface = preop_get(r, itsLine);
//    spec_is_true(*(((int *) surface) + 3) == 7);
//}


//    testSymbolPath();
