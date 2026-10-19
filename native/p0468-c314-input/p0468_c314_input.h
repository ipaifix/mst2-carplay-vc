#ifndef P0468_C314_INPUT_H
#define P0468_C314_INPUT_H
#include "../p0468-rgi-parser/p0468_rgi_parser.h"

/* OFFLINE candidate. No hooks, publication, file I/O or installed producer. */
#define P0468_C314_DUPLICATE (-40)
#define P0468_C314_NARROWING (-41)
typedef struct {
    p0468_rgi_frame value;
    p0468_rgi_u16 wire_junction_count, wire_current_count;
    p0468_rgi_u8 junction_bounded, current_list_bounded, lane_list_bounded;
} p0468_c314_input;

/* Bounded means the parser did not truncate the represented lists. It does NOT
 * prove physical-road topology, a current generation, or a delta's admission.
 * On failure the ENTIRE output is cleared. Recognized duplicate top-level
 * fields are refused, except repeated maneuver angles and lane information.
 * Nested lane status/highlight duplicates and scalar narrowing are refused;
 * explicit lane positions and signed-16 angle-list units are required.
 */
int p0468_c314_parse(const p0468_rgi_u8 *, p0468_rgi_size, p0468_c314_input *);
#endif
