#include "p0468_c314_input.h"

static void clear(void *p, p0468_rgi_size size) {
    p0468_rgi_u8 *out = p;
    while (size--) *out++ = 0;
}
static unsigned be16(const p0468_rgi_u8 *p) {
    return ((unsigned)p[0] << 8) | p[1];
}
static unsigned scalar_bits(unsigned message, unsigned id) {
    if (message == 0x5202U) {
        if (id == 1U || id == 12U || id == 11U) return 16U;
        if (id == 3U || id == 7U || id == 8U || id == 9U) return 8U;
        if (id == 5U) return 32U;
    } else if (message == 0x5201U) {
        if (id == 5U || id == 6U) return 64U;
        if (id == 7U || id == 10U) return 32U;
        if (id == 14U || id == 16U || id == 17U) return 16U;
        if (id == 1U || id == 2U || id == 9U || id == 12U || id == 15U || id == 18U || id == 20U) return 8U;
    } else if (message == 0x5204U && id == 1U) return 16U;
    return 0U;
}

/* Conservative nested subset: explicit position starts each lane; angle data
 * consists of signed-16 units, not a guessed odd trailing byte. */
static int lane_audit(const p0468_rgi_u8 *raw, p0468_rgi_size length) {
    p0468_rgi_size offset = 0;
    unsigned seen = 0, have_position = 0;
    while (offset < length) {
        unsigned size, id;
        p0468_rgi_size payload, i;
        p0468_rgi_u64 value = 0;
        if (length - offset < 4U) return P0468_RGI_PARSE_TLV;
        size = be16(raw + offset); id = be16(raw + offset + 2U);
        if (size < 4U || size > length - offset) return P0468_RGI_PARSE_TLV;
        payload = size - 4U;
        if (id == 0U) {
            if (payload != 1U && payload != 2U) return P0468_RGI_PARSE_VALUE;
            have_position = 1U; seen = 0U;
        } else if (id == 1U || id == 3U) {
            unsigned bit = 1U << id;
            if (!have_position) return P0468_RGI_PARSE_VALUE;
            if (seen & bit) return P0468_C314_DUPLICATE;
            seen |= bit;
            if (payload != 1U && payload != 2U && payload != 4U && payload != 8U) return P0468_RGI_PARSE_VALUE;
            for (i = 0; i < payload; ++i) value = (value << 8) | raw[offset + 4U + i];
            if (value > (id == 1U ? 255U : 65535U)) return P0468_C314_NARROWING;
        } else if (id == 2U) {
            if (!have_position || payload == 0U || (payload & 1U)) return P0468_RGI_PARSE_VALUE;
        }
        offset += size;
    }
    return P0468_RGI_PARSE_OK;
}

int p0468_c314_parse(const p0468_rgi_u8 *raw, p0468_rgi_size length, p0468_c314_input *out) {
    p0468_rgi_u32 seen = 0;
    p0468_rgi_size offset;
    unsigned message, junctions = 0, currents = 0;
    int result;
    if (!out) return P0468_RGI_PARSE_NULL;
    clear(out, sizeof(*out));
    if (!raw) return P0468_RGI_PARSE_NULL;
    /* Do not invoke the legacy parser until the full top-level audit passes. */
    if (length < 6U) return P0468_RGI_PARSE_SHORT;
    if (raw[0] != 0x40U || raw[1] != 0x40U) return P0468_RGI_PARSE_HEADER;
    if (length != be16(raw + 2)) return P0468_RGI_PARSE_LENGTH;
    message = be16(raw + 4);
    if (message != 0x5201U && message != 0x5202U && message != 0x5204U) return P0468_RGI_PARSE_MESSAGE;
    for (offset = 6U; offset < length;) {
        unsigned size, id, bits, known, repeat;
        p0468_rgi_u64 value = 0;
        p0468_rgi_size i, payload;
        if (length - offset < 4U) return P0468_RGI_PARSE_TLV;
        size = be16(raw + offset); id = be16(raw + offset + 2);
        if (size < 4U || size > length - offset) return P0468_RGI_PARSE_TLV;
        payload = size - 4U;
        known = id <= (message == 0x5201U ? 20U : message == 0x5202U ? 13U : 3U);
        repeat = (message == 0x5202U && id == 10U) || (message == 0x5204U && id == 2U);
        if (known && !repeat) {
            p0468_rgi_u32 bit = 1U << id;
            if (seen & bit) return P0468_C314_DUPLICATE;
            seen |= bit;
        }
        bits = scalar_bits(message, id);
        if (bits) {
            if (payload != 1U && payload != 2U && payload != 4U && payload != 8U) return P0468_RGI_PARSE_VALUE;
            for (i = 0; i < payload; ++i) value = (value << 8) | raw[offset + 4U + i];
            if (bits != 64U && value > (((p0468_rgi_u64)1U << bits) - 1U)) return P0468_C314_NARROWING;
        }
        if (message == 0x5202U && id == 10U) {
            if (payload != 2U) return P0468_RGI_PARSE_VALUE;
            ++junctions;
        }
        if (message == 0x5201U && id == 13U) {
            if (payload & 1U) return P0468_RGI_PARSE_VALUE;
            currents = (unsigned)(payload / 2U);
        }
        if (message == 0x5204U && id == 2U) {
            result = lane_audit(raw + offset + 4U, payload);
            if (result != P0468_RGI_PARSE_OK) return result;
        }
        offset += size;
    }
    result = p0468_rgi_parse_frame(raw, length, &out->value);
    if (result != P0468_RGI_PARSE_OK) {
        clear(out, sizeof(*out));
        return result;
    }
    out->wire_junction_count = (p0468_rgi_u16)junctions;
    out->wire_current_count = (p0468_rgi_u16)currents;
    out->junction_bounded = (p0468_rgi_u8)(junctions <= P0468_RGI_JUNCTION_ANGLES_MAX);
    out->current_list_bounded = (p0468_rgi_u8)(currents <= P0468_RGI_MANEUVER_LIST_MAX);
    out->lane_list_bounded = (p0468_rgi_u8)(message == 0x5204U && out->value.lane.complete);
    return P0468_RGI_PARSE_OK;
}
