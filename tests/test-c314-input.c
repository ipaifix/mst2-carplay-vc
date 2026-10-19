#include "../native/p0468-c314-input/p0468_c314_input.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned checks, size;
static unsigned char raw[1024];
static p0468_c314_input out;
#define CHECK(x) do { ++checks; assert(x); } while (0)
static void word(unsigned off, unsigned value) { raw[off] = (unsigned char)(value >> 8); raw[off+1] = (unsigned char)value; }
static void begin(unsigned message) { memset(raw,0,sizeof(raw)); size=6; raw[0]=raw[1]=0x40;word(4,message); }
static void scalar(unsigned id,unsigned bytes,unsigned long long value) {
    unsigned i; word(size,4+bytes); word(size+2,id);
    for(i=0;i<bytes;++i)raw[size+4+i]=(unsigned char)(value >> ((bytes-i-1)*8));
    size+=4+bytes;
}
static int run(void) { word(2,size); return p0468_c314_parse(raw,size,&out); }
static void cleared(void) {
    unsigned i;
    for(i=0;i<sizeof(out);++i)CHECK(((unsigned char *)&out)[i]==0);
}
static void lane_block(unsigned position,unsigned angles,unsigned duplicate) {
    unsigned start=size,i;
    size+=4;scalar(0,2,position);scalar(1,1,2);scalar(3,2,(unsigned short)-90);
    if(duplicate)scalar(1,1,2);
    if(angles){word(size,4+angles*2);word(size+2,2);for(i=0;i<angles;++i)word(size+4+i*2,i);size+=4+angles*2;}
    word(start,size-start);word(start+2,2);
}
int main(void) {
    unsigned i,n;
    for(n=0;n<=17;++n) {
        begin(0x5202); scalar(0,2,0x10); scalar(1,2,4); scalar(3,1,2);
        scalar(9,1,0); scalar(11,2,90);
        for(i=0;i<n;++i)scalar(10,2,(unsigned short)(-90+(int)i));
        CHECK(run()==0);
        CHECK(out.wire_junction_count==n && out.junction_bounded==(n<=16));
        CHECK(out.value.maneuver.junction_angle_count==(n>16?16:n));
        CHECK((out.value.maneuver.present & 256U)!=0);
    }
    begin(0x5202); scalar(11,2,0);CHECK(run()==0);
    CHECK((out.value.maneuver.present&1024U)!=0 && out.value.maneuver.exit_angle==0);
    begin(0x5202);scalar(3,1,2);scalar(3,1,2);CHECK(run()==P0468_C314_DUPLICATE);cleared();
    begin(0x5202);scalar(1,4,65536);CHECK(run()==P0468_C314_NARROWING);cleared();
    begin(0x5202);scalar(11,8,0x10000);CHECK(run()==P0468_C314_NARROWING);cleared();
    begin(0x5202);scalar(11,2,0xffa6);CHECK(run()==0);CHECK(out.value.maneuver.exit_angle==-90);
    for(n=16;n<=17;++n){
        begin(0x5201); word(size,4+n*2);word(size+2,13);
        for(i=0;i<n;++i) {
            word(size+4+i*2,i);
        }
        size+=4+n*2;
        CHECK(run()==0);CHECK(out.wire_current_count==n&&out.current_list_bounded==(n==16));
    }
    begin(0x5201);scalar(13,2,0);scalar(13,2,0);CHECK(run()==P0468_C314_DUPLICATE);cleared();
    begin(0x5201);scalar(1,8,257);CHECK(run()==P0468_C314_NARROWING);cleared();
    begin(0x5204);scalar(1,8,65536);CHECK(run()==P0468_C314_NARROWING);cleared();
    begin(0x5202);scalar(0xffff,1,2);CHECK(run()==0); // Unknown TLV skipped, not invented.
    for(i=0;i<size;++i) { CHECK(p0468_c314_parse(raw,i,&out)<0);cleared(); }
    CHECK(p0468_c314_parse(NULL,0,&out)<0);cleared();
    CHECK(p0468_c314_parse(raw,size,NULL)<0);
    // Malformed nested lane information: legacy failure cannot leak partial output.
    begin(0x5204);scalar(2,1,1);CHECK(run()<0);cleared();
    begin(0x5204);scalar(1,2,6);lane_block(0,0,0);CHECK(run()==0);
    CHECK(out.lane_list_bounded && out.value.lane.lane_count==1);
    CHECK(out.value.lane.lanes[0].angle_highlight==-90 && out.value.lane.lanes[0].angle_count==0);
    begin(0x5204);lane_block(0,0,1);CHECK(run()==P0468_C314_DUPLICATE);cleared();
    for(n=16;n<=17;++n){begin(0x5204);lane_block(0,n,0);CHECK(run()==0);CHECK(out.lane_list_bounded==(n==16));}
    for(n=8;n<=9;++n){begin(0x5204);for(i=0;i<n;++i)lane_block(i,0,0);CHECK(run()==0);CHECK(out.lane_list_bounded==(n==8));}
    // Nested narrowing, missing position and odd angle units never pass silently.
    begin(0x5204);size+=4;scalar(0,2,0);scalar(1,8,258);word(6,size-6);word(8,2);
    CHECK(run()==P0468_C314_NARROWING);cleared();
    begin(0x5204);size+=4;scalar(1,1,2);word(6,size-6);word(8,2);CHECK(run()<0);cleared();
    begin(0x5204);size+=4;scalar(0,2,0);scalar(1,1,2);scalar(2,1,0);word(6,size-6);word(8,2);CHECK(run()<0);cleared();
    for(n=0;n<10000;++n) {
        begin(0x5202);scalar(1,2,n%65536);scalar(3,1,n%54);
        scalar(9,1,n%2);scalar(11,2,(unsigned short)((int)(n%361)-180));
        scalar(10,2,(unsigned short)((int)((n+1)%361)-180));
        CHECK(run()==0 && out.junction_bounded && out.wire_junction_count==1);
    }
    printf("C314 input audit: %u assertions PASS (OFFLINE ONLY)\n",checks);
    return 0;
}
