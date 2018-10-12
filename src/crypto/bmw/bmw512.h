#ifndef HASHBLOCK_H
#define HASHBLOCK_H

#include "uint256.h"
#include "../common/sph_bmw.h"

#ifndef QT_NO_DEBUG
#include <string>
#endif

#ifdef GLOBALDEFINED
#define GLOBAL
#else
#define GLOBAL extern
#endif

GLOBAL sph_bmw512_context       z_bmw;

#define fillz() do { \
    sph_bmw512_init(&z_bmw); \
} while (0) 

#define ZBMW (memcpy(&ctx_bmw, &z_bmw, sizeof(z_bmw)))

template<typename T1>
inline uint256 Hash_bmw512(const T1 pbegin, const T1 pend)

{
    sph_bmw512_context       ctx_bmw;
    static unsigned char pblank[1];

#ifndef QT_NO_DEBUG
    //std::string strhash;
    //strhash = "";
#endif
    
    uint512 hash[1];

    sph_bmw512_init(&ctx_bmw);
    sph_bmw512 (&ctx_bmw, (pbegin == pend ? pblank : static_cast<const void*>(&pbegin[0])), (pend - pbegin) * sizeof(pbegin[0]));
    sph_bmw512_close(&ctx_bmw, static_cast<void*>(&hash[0]));

    return hash[0].trim256();
}






#endif // HASHBLOCK_H
