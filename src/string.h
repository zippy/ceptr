#define STRING_TERMINATOR 0xFFFFFFFF
#define ESCAPE_STRING_TERMINATOR 0xFFFFFFFE

int proc_string_get_size(Receptor *r, Symbol noun,ElementSurface *spec_surface, void *surface) {
    //TODO: handle strings of arrays
    ElementSurface *es = _get_reps_pattern_spec(r,spec_surface);
    int rep_size = pattern_get_size(es);
    int size = 0;
    while(*(int *)surface != STRING_TERMINATOR) {
        if (*(int *)surface == ESCAPE_STRING_TERMINATOR) {
            surface += sizeof(int);
            size +=sizeof(int);
        }
        size += rep_size;
        surface += rep_size;
    }
    return sizeof(int) + size;
}


Xaddr preop_new_string(Receptor *r, char *label, Symbol repsNoun, int processCount, Process *processes){
    return _preop_new_rep(r,r->stringSpecXaddr,r->stringNoun,label,repsNoun,processCount,processes);
}

void dump_string_value(Receptor *r, ElementSurface *rs, void *surface) {
    int count = 0;
    ElementSurface *ps = _get_reps_pattern_spec(r,rs);
    Symbol repsNoun = REPS_GET_NOUN(rs);
    printf(" %s(%d) string of %s(%d)\n",noun_label(r,rs->name),rs->name,noun_label(r,repsNoun),repsNoun);
    while (*(int *)surface != STRING_TERMINATOR) {
        if (*(int *)surface == ESCAPE_STRING_TERMINATOR) {
            surface += sizeof(int);
        }
        printf("    ");
        dump_pattern_value(r,ps,surface);
        printf("\n");
        surface += pattern_get_size(ps);
        count++;
    }
    printf(" %d elements found\n",count);
}


Symbol ZODIAC;
int sky[] = {3,   2, 2,3, 40,50,   1,100,101,  4, 11,22, 33,44, 55,66, 77,88 };
Xaddr myZodiac = _make_zodiac(r,CONSTELLATION,&ZODIAC,sky);
_make_string(r,SL,&CONSTELLATION);



void testString() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Symbol CONSTELLATION;
    int * orionSurface = _make_string(r,_make_star_loc(r),&CONSTELLATION);
    spec_is_true(*orionSurface == 1);
    spec_is_true(*(orionSurface+8) == STRING_TERMINATOR);
    spec_is_true(size_of_named_surface(r,CONSTELLATION,orionSurface) == sizeof(int)*9);
}


int *_make_string(Receptor *r,Symbol STAR_LOCATION,Symbol *CONSTELLATION){
    Xaddr starLocString = preop_new_string(r, "STAR_LOCATION_STRING", STAR_LOCATION, 0, 0);
    *CONSTELLATION = preop_new_noun(r, starLocString, "CONSTELLATION");

    struct {
        int point1X;
        int point1Y;
        int escape;
        int point2X;
        int point2Y;
        int escape2;
        int point3X;
        int point3Y;
        int term;
    } orion = {  1,2,  ESCAPE_STRING_TERMINATOR,-1, 20,  ESCAPE_STRING_TERMINATOR,-2, 200, STRING_TERMINATOR };
    Xaddr orionXaddr = preop_new(r, *CONSTELLATION, &orion);
    return (int *)surface_for_xaddr(r, orionXaddr);
}
