#ifndef P0468_C314_ENVELOPE_H
#define P0468_C314_ENVELOPE_H
/* Explicit new wrapper, NOT a silent modification of the v4/400-byte ABI.
 * Codec only: the future producer must establish epochs and atomic geometry.
 */
#define P0468_C314_ENVELOPE_SIZE 456U
#define P0468_C314_BASE_SIZE 400U
typedef struct {
 unsigned session_generation,route_generation,definition_generation,lane_generation;
 unsigned maneuver_present,gate_present;
 unsigned short wire_junction_count,current_index,linked_lane,flags;
} p0468_c314_evidence;
/* flags: 1=atomic geometry fields; 2=untruncated angles; 4=complete lane event.
 * gate_present: 1=current maneuver selection, 2=current lane selection,
 * 4=received lane display gate. Integrity is not authentication.
 * On failure, output is zeroed. No allocation, I/O, native hook or publication.
 */
int p0468_c314_envelope_encode(unsigned char *,const unsigned char *,const p0468_c314_evidence *);
#endif
