#include "p0468_c314_envelope.h"
typedef char c314_uint_must_be_32[(sizeof(unsigned)==4)?1:-1];
static unsigned get(const unsigned char *p){return ((unsigned)p[0]<<24)|((unsigned)p[1]<<16)|((unsigned)p[2]<<8)|p[3];}
static unsigned short_get(const unsigned char *p){return ((unsigned)p[0]<<8)|p[1];}
static void put(unsigned char *p,unsigned v){p[0]=(unsigned char)(v>>24);p[1]=(unsigned char)(v>>16);p[2]=(unsigned char)(v>>8);p[3]=(unsigned char)v;}
static void short_put(unsigned char *p,unsigned v){p[0]=(unsigned char)(v>>8);p[1]=(unsigned char)v;}
static unsigned hash(const unsigned char *p,unsigned size){unsigned h=2166136261U,i;for(i=0;i<size;++i){h^=p[i];h*=16777619U;}return h;}
int p0468_c314_envelope_encode(unsigned char *out,const unsigned char *base,const p0468_c314_evidence *e){
 unsigned i,required=1U|4U|256U|512U|1024U;
 if(!out)return -1;
 for(i=0;i<P0468_C314_ENVELOPE_SIZE;++i)out[i]=0;
 if(!base||!e||base[0]!='P'||base[1]!='R'||base[2]!='G'||base[3]!='I'||base[4]!=4||base[5]||short_get(base+6)!=400||hash(base,396)!=get(base+396))return -1;
 if(get(base+28)==0||get(base+28)>0x7fffffffU||e->session_generation==0||e->session_generation>32||e->maneuver_present&~8191U||e->gate_present&~7U||e->flags&~7U)return -1;
 if(e->current_index!=short_get(base+18)||e->wire_junction_count<base[14])return -1;
 if((e->flags&2U)&&(e->wire_junction_count>16U||e->wire_junction_count!=base[14]))return -1;
 if((e->flags&1U)&&(!(e->flags&2U)||(e->maneuver_present&required)!=required))return -1;
 if(!(e->maneuver_present&2048U)&&e->linked_lane!=0)return -1;
 out[0]='P';out[1]='R';out[2]='G';out[3]='X';out[4]=1;short_put(out+6,P0468_C314_ENVELOPE_SIZE);
 put(out+8,get(base+28));put(out+12,e->session_generation);put(out+16,e->route_generation);
 put(out+20,e->definition_generation);put(out+24,e->lane_generation);put(out+28,e->maneuver_present);put(out+32,e->gate_present);
 short_put(out+36,e->wire_junction_count);short_put(out+38,e->current_index);short_put(out+40,e->linked_lane);short_put(out+42,e->flags);
 for(i=0;i<400;++i)out[52+i]=base[i];
 put(out+452,hash(out,452));return 0;
}
