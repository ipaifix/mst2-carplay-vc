#include "../native/p0468-c314-envelope/p0468_c314_envelope.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned checks;
#define CHECK(x) do { ++checks; assert(x); } while (0)
static unsigned checksum(const unsigned char *p, unsigned n) {
    unsigned h = 2166136261U, i;
    for (i = 0; i < n; ++i) { h ^= p[i]; h *= 16777619U; }
    return h;
}
static void put(unsigned char *p, unsigned v) {
    p[0]=(unsigned char)(v>>24); p[1]=(unsigned char)(v>>16);
    p[2]=(unsigned char)(v>>8); p[3]=(unsigned char)v;
}
static unsigned get(const unsigned char *p) {
    return ((unsigned)p[0]<<24)|((unsigned)p[1]<<16)|((unsigned)p[2]<<8)|p[3];
}
static void clear_check(const unsigned char *p) {
    unsigned i; for (i=0;i<456;++i) CHECK(p[i]==0);
}
int main(void) {
    unsigned char base[400]={0}, out[456], saved[400];
    p0468_c314_evidence e={3,17,17,17,1U|4U|256U|512U|1024U,7,2,4,0,3};
    unsigned i;
    /* Only a synthetic minimum accepted by this codec, not a full route payload. */
    memcpy(base,"PRGI",4);base[4]=4;base[6]=1;base[7]=144;
    base[14]=2;base[19]=4;put(base+28,123);put(base+396,checksum(base,396));
    memcpy(saved,base,sizeof base);
    CHECK(p0468_c314_envelope_encode(out,base,&e)==0);
    CHECK(memcmp(base,saved,sizeof base)==0);
    CHECK(memcmp(out,"PRGX",4)==0 && out[4]==1 && out[6]==1 && out[7]==200);
    CHECK(get(out+8)==123 && get(out+12)==3 && get(out+16)==17);
    CHECK(memcmp(out+52,base,400)==0 && get(out+452)==checksum(out,452));
    for(i=44;i<52;++i)CHECK(out[i]==0);
    e.flags=8;CHECK(p0468_c314_envelope_encode(out,base,&e)<0);clear_check(out);e.flags=3;
    e.current_index=5;CHECK(p0468_c314_envelope_encode(out,base,&e)<0);clear_check(out);e.current_index=4;
    e.wire_junction_count=17;CHECK(p0468_c314_envelope_encode(out,base,&e)<0);clear_check(out);e.wire_junction_count=2;
    e.session_generation=0;CHECK(p0468_c314_envelope_encode(out,base,&e)<0);clear_check(out);e.session_generation=3;
    e.linked_lane=7;CHECK(p0468_c314_envelope_encode(out,base,&e)<0);clear_check(out);
    e.maneuver_present|=2048;CHECK(p0468_c314_envelope_encode(out,base,&e)==0);
    base[399]^=1;CHECK(p0468_c314_envelope_encode(out,base,&e)<0);clear_check(out);
    CHECK(p0468_c314_envelope_encode(out,0,&e)<0);clear_check(out);
    CHECK(p0468_c314_envelope_encode(out,saved,0)<0);clear_check(out);
    CHECK(p0468_c314_envelope_encode(0,saved,&e)<0);
    printf("Original envelope codec: %u assertions PASS; synthetic only\n",checks);
    return 0;
}
