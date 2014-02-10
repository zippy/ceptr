#include "../../src/ceptr.h"

Symbol _make_star_loc(Receptor *r){
    Xaddr pointPatternSpecXaddr = initPoint(r);
    return preop_new_noun(r, pointPatternSpecXaddr, "STAR_LOCATION");
}

Xaddr _make_constellation(Receptor *r,Symbol STAR_LOCATION,Symbol *CONSTELLATION,int surface[]) {
    Xaddr starLocArray = preop_new_array(r, "STAR_LOCATION_ARRAY", STAR_LOCATION, 0, 0);
    *CONSTELLATION = preop_new_noun(r, starLocArray, "CONSTELLATION");
    return preop_new(r, *CONSTELLATION, surface);
}

Xaddr _make_zodiac(Receptor *r,Symbol CONSTELLATION,Symbol *ZODIAC,void *sky) {
    Xaddr constellationArray = preop_new_array(r,"CONSTELLATION_ARRAY",CONSTELLATION,0,0);
    *ZODIAC = preop_new_noun(r, constellationArray, "ZODIAC");
    return preop_new(r, *ZODIAC, sky);
}

void testStars() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Symbol CONSTELLATION;
    int stars[] = { 3,   1,2,  10,20,  100,200 };
    Symbol STAR_LOC = _make_star_loc(r);
    Xaddr orion = _make_constellation(r,STAR_LOC,&CONSTELLATION,stars);

    int *pointSurface = (int *) preop_get_array_nth(r, 1, orion);
    spec_is_true(*pointSurface == 10);
    spec_is_true(*(pointSurface+1) == 20);
    spec_is_true(preop_get_array_length(r,orion) == 3);

    Symbol ZODIAC;
    int sky[] = {3,   2, 2,3, 40,50,   1,100,101,  4, 11,22, 33,44, 55,66, 77,88 };
    Xaddr myZodiac = _make_zodiac(r,CONSTELLATION,&ZODIAC,sky);
    int *arraySurface = (int *) preop_get_array_nth(r, 2, myZodiac);
    spec_is_true(size_of_named_surface(r,ZODIAC,sky) == 18*sizeof(int));
    spec_is_true(_preop_get_array_length(arraySurface) == 4);
}
