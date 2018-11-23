#ifndef _COMMON_H_
#define _COMMON_H_
#include <stdlib.h>
#include <stdint.h>

#define SWAP(type,a, b) {type _SWAPTMP=a; a=b; b=_SWAPTMP;}
#define INRANGE(x,a,b)  ((unsigned)((x)-(a)) <= (unsigned)((b)-(a)))

#define ISHELP(s)   (s[0]=='-' && s[1]=='-' && s[2]=='h' && s[3]=='e' && \
                     s[4]=='l' && s[5]=='p' && s[6]==0)
#define ISOPT(s,c)  (s[0]=='-' && s[1]==c && s[2]==0)
#define ISCHAR(s,c) (s[0]==c && s[1]==0)
#define ISDIGIT(c)  INRANGE((c), '0', '9')
#define ISSPACE(c)  ((c)==' ' || INRANGE((c), '\t', '\r'))

union HexDouble {
    double d;       // 1 + 11 + 52 = 64 bits
    uint64_t u;     // imply bit, bias=1023
};

union HexLongDouble {               // non-portable !
    long double ld;                 // 1 + 15 + 64 = 80 bits
    struct {uint64_t mantissa;      // no implied bit
            unsigned bexp:15;       // bias = 16383
            unsigned negative:1; }; // 1 == negative
};
#endif
