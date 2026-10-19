#include "p0468_rgi_parser.h"

static void p0468_zero(void *pointer, p0468_rgi_size length)
{
    p0468_rgi_u8 *bytes = (p0468_rgi_u8 *)pointer;
    while (length != 0U) {
        *bytes++ = 0U;
        --length;
    }
}

static p0468_rgi_u16 p0468_u16_be(const p0468_rgi_u8 *bytes)
{
    return (p0468_rgi_u16)(((p0468_rgi_u16)bytes[0] << 8) |
                            (p0468_rgi_u16)bytes[1]);
}

static p0468_rgi_u64 p0468_unsigned_be(const p0468_rgi_u8 *bytes,
                                        p0468_rgi_size length)
{
    p0468_rgi_u64 value = 0U;
    while (length != 0U) {
        value = (value << 8) | (p0468_rgi_u64)*bytes++;
        --length;
    }
    return value;
}

static int p0468_scalar(const p0468_rgi_u8 *bytes,
                         p0468_rgi_size length,
                         p0468_rgi_u64 *value)
{
    if (length != 1U && length != 2U && length != 4U && length != 8U) {
        return P0468_RGI_PARSE_VALUE;
    }
    *value = p0468_unsigned_be(bytes, length);
    return P0468_RGI_PARSE_OK;
}

static void p0468_text(char *destination,
                        const p0468_rgi_u8 *source,
                        p0468_rgi_size length)
{
    p0468_rgi_size copy = length;
    p0468_rgi_size index;

    while (copy != 0U && source[copy - 1U] == 0U) --copy;
    if (copy >= P0468_RGI_TEXT_MAX) copy = P0468_RGI_TEXT_MAX - 1U;
    for (index = 0U; index < copy; ++index) {
        destination[index] = (char)source[index];
    }
    destination[copy] = '\0';
}

static int p0468_parse_route_tlv(p0468_rgi_frame *parsed,
                                  p0468_rgi_u16 identifier,
                                  const p0468_rgi_u8 *value,
                                  p0468_rgi_size length)
{
    p0468_rgi_u64 scalar;
    p0468_rgi_size index;
    int result;

    if (identifier == 0U) {
        if (length != 2U) return P0468_RGI_PARSE_VALUE;
        parsed->component_id = p0468_u16_be(value);
        parsed->component_present = 1U;
        return P0468_RGI_PARSE_OK;
    }
    if (identifier == 3U || identifier == 4U || identifier == 8U ||
        identifier == 11U || identifier == 19U) {
        char *target = parsed->route.current_road;
        p0468_rgi_u32 bit = P0468_RGI_ROUTE_CURRENT_ROAD;
        if (identifier == 4U) {
            target = parsed->route.destination;
            bit = P0468_RGI_ROUTE_DESTINATION;
        } else if (identifier == 8U) {
            target = parsed->route.distance_string;
            bit = P0468_RGI_ROUTE_DISTANCE_STRING;
        } else if (identifier == 11U) {
            target = parsed->route.maneuver_distance_string;
            bit = P0468_RGI_ROUTE_MANEUVER_DISTANCE_STRING;
        } else if (identifier == 19U) {
            target = parsed->route.source_name;
            bit = P0468_RGI_ROUTE_SOURCE_NAME;
        }
        p0468_text(target, value, length);
        parsed->route.present |= bit;
        return P0468_RGI_PARSE_OK;
    }
    if (identifier == 13U) {
        if ((length & 1U) != 0U) return P0468_RGI_PARSE_VALUE;
        parsed->route.maneuver_list_count = 0U;
        for (index = 0U; index + 1U < length &&
             parsed->route.maneuver_list_count < P0468_RGI_MANEUVER_LIST_MAX;
             index += 2U) {
            parsed->route.maneuver_list[parsed->route.maneuver_list_count++] =
                p0468_u16_be(value + index);
        }
        parsed->route.present |= P0468_RGI_ROUTE_MANEUVER_LIST;
        return P0468_RGI_PARSE_OK;
    }
    if (identifier != 1U && identifier != 2U && identifier != 5U &&
        identifier != 6U && identifier != 7U && identifier != 9U &&
        identifier != 10U && identifier != 12U && identifier != 14U &&
        identifier != 15U && identifier != 16U && identifier != 17U &&
        identifier != 18U && identifier != 20U) {
        return P0468_RGI_PARSE_OK;
    }
    result = p0468_scalar(value, length, &scalar);
    if (result != P0468_RGI_PARSE_OK) return result;
    switch (identifier) {
        case 1U:
            parsed->route.state = (p0468_rgi_u8)scalar;
            parsed->route.present |= P0468_RGI_ROUTE_STATE;
            break;
        case 2U:
            parsed->route.maneuver_state = (p0468_rgi_u8)scalar;
            parsed->route.present |= P0468_RGI_ROUTE_MANEUVER_STATE;
            break;
        case 5U:
            parsed->route.eta = scalar;
            parsed->route.present |= P0468_RGI_ROUTE_ETA;
            break;
        case 6U:
            parsed->route.time_remaining = scalar;
            parsed->route.present |= P0468_RGI_ROUTE_TIME_REMAINING;
            break;
        case 7U:
            parsed->route.distance_remaining = (p0468_rgi_u32)scalar;
            parsed->route.present |= P0468_RGI_ROUTE_DISTANCE_REMAINING;
            break;
        case 9U:
            parsed->route.distance_units = (p0468_rgi_u8)scalar;
            parsed->route.present |= P0468_RGI_ROUTE_DISTANCE_UNITS;
            break;
        case 10U:
            parsed->route.distance_to_maneuver = (p0468_rgi_u32)scalar;
            parsed->route.present |= P0468_RGI_ROUTE_DISTANCE_TO_MANEUVER;
            break;
        case 12U:
            parsed->route.maneuver_distance_units = (p0468_rgi_u8)scalar;
            parsed->route.present |= P0468_RGI_ROUTE_MANEUVER_DISTANCE_UNITS;
            break;
        case 14U:
            parsed->route.maneuver_count = (p0468_rgi_u16)scalar;
            parsed->route.present |= P0468_RGI_ROUTE_MANEUVER_COUNT;
            break;
        case 15U:
            parsed->route.visible = (p0468_rgi_u8)scalar;
            parsed->route.present |= P0468_RGI_ROUTE_VISIBLE;
            break;
        case 16U:
            parsed->route.lane_index = (p0468_rgi_u16)scalar;
            parsed->route.present |= P0468_RGI_ROUTE_LANE_INDEX;
            break;
        case 17U:
            parsed->route.lane_total = (p0468_rgi_u16)scalar;
            parsed->route.present |= P0468_RGI_ROUTE_LANE_TOTAL;
            break;
        case 18U:
            parsed->route.lane_showing = (p0468_rgi_u8)scalar;
            parsed->route.present |= P0468_RGI_ROUTE_LANE_SHOWING;
            break;
        case 20U:
            parsed->route.source_supports = (p0468_rgi_u8)scalar;
            parsed->route.present |= P0468_RGI_ROUTE_SOURCE_SUPPORTS;
            break;
        default:
            break;
    }
    return P0468_RGI_PARSE_OK;
}

static int p0468_parse_maneuver_tlv(p0468_rgi_frame *parsed,
                                     p0468_rgi_u16 identifier,
                                     const p0468_rgi_u8 *value,
                                     p0468_rgi_size length)
{
    p0468_rgi_u64 scalar;
    int result;

    if (identifier == 0U) {
        if (length != 2U) return P0468_RGI_PARSE_VALUE;
        parsed->component_id = p0468_u16_be(value);
        parsed->component_present = 1U;
        return P0468_RGI_PARSE_OK;
    }
    if (identifier == 2U || identifier == 4U || identifier == 6U ||
        identifier == 13U) {
        char *target = parsed->maneuver.description;
        p0468_rgi_u32 bit = P0468_RGI_MANEUVER_DESCRIPTION;
        if (identifier == 4U) {
            target = parsed->maneuver.after_road;
            bit = P0468_RGI_MANEUVER_AFTER_ROAD;
        } else if (identifier == 6U) {
            target = parsed->maneuver.distance_string;
            bit = P0468_RGI_MANEUVER_DISTANCE_STRING;
        } else if (identifier == 13U) {
            target = parsed->maneuver.exit_info;
            bit = P0468_RGI_MANEUVER_EXIT_INFO;
        }
        p0468_text(target, value, length);
        parsed->maneuver.present |= bit;
        return P0468_RGI_PARSE_OK;
    }
    if (identifier == 10U) {
        if (length != 2U) return P0468_RGI_PARSE_VALUE;
        if (parsed->maneuver.junction_angle_count <
            P0468_RGI_JUNCTION_ANGLES_MAX) {
            parsed->maneuver.junction_angles[
                parsed->maneuver.junction_angle_count++] =
                    (p0468_rgi_s16)p0468_u16_be(value);
        }
        parsed->maneuver.present |= P0468_RGI_MANEUVER_JUNCTION_ANGLES;
        return P0468_RGI_PARSE_OK;
    }
    if (identifier != 1U && identifier != 3U && identifier != 5U &&
        identifier != 7U && identifier != 8U && identifier != 9U &&
        identifier != 11U && identifier != 12U) {
        return P0468_RGI_PARSE_OK;
    }
    result = p0468_scalar(value, length, &scalar);
    if (result != P0468_RGI_PARSE_OK) return result;
    switch (identifier) {
        case 1U:
            parsed->maneuver.index = (p0468_rgi_u16)scalar;
            parsed->maneuver.present |= P0468_RGI_MANEUVER_INDEX;
            break;
        case 3U:
            parsed->maneuver.type = (p0468_rgi_u8)scalar;
            parsed->maneuver.present |= P0468_RGI_MANEUVER_TYPE;
            break;
        case 5U:
            parsed->maneuver.distance = (p0468_rgi_u32)scalar;
            parsed->maneuver.present |= P0468_RGI_MANEUVER_DISTANCE;
            break;
        case 7U:
            parsed->maneuver.distance_units = (p0468_rgi_u8)scalar;
            parsed->maneuver.present |= P0468_RGI_MANEUVER_DISTANCE_UNITS;
            break;
        case 8U:
            parsed->maneuver.driving_side = (p0468_rgi_u8)scalar;
            parsed->maneuver.present |= P0468_RGI_MANEUVER_DRIVING_SIDE;
            break;
        case 9U:
            parsed->maneuver.junction_type = (p0468_rgi_u8)scalar;
            parsed->maneuver.present |= P0468_RGI_MANEUVER_JUNCTION_TYPE;
            break;
        case 11U:
            parsed->maneuver.exit_angle = (p0468_rgi_s16)scalar;
            parsed->maneuver.present |= P0468_RGI_MANEUVER_EXIT_ANGLE;
            break;
        case 12U:
            parsed->maneuver.linked_lane = (p0468_rgi_u16)scalar;
            parsed->maneuver.present |= P0468_RGI_MANEUVER_LINKED_LANE;
            break;
        default:
            break;
    }
    return P0468_RGI_PARSE_OK;
}

static int p0468_parse_lane_information(p0468_rgi_frame *parsed,
                                         const p0468_rgi_u8 *value,
                                         p0468_rgi_size length)
{
    p0468_rgi_size offset = 0U;
    p0468_rgi_u8 first_lane = parsed->lane.lane_count;
    p0468_rgi_lane_item *lane = (p0468_rgi_lane_item *)0;

    while (offset < length) {
        p0468_rgi_u16 tlv_length;
        p0468_rgi_u16 identifier;
        const p0468_rgi_u8 *nested_value;
        p0468_rgi_size nested_length;
        p0468_rgi_size angle_offset;
        p0468_rgi_u64 scalar;
        int result;

        if (length - offset < 4U) return P0468_RGI_PARSE_TLV;
        tlv_length = p0468_u16_be(value + offset);
        identifier = p0468_u16_be(value + offset + 2U);
        if (tlv_length < 4U || (p0468_rgi_size)tlv_length > length - offset) {
            return P0468_RGI_PARSE_TLV;
        }
        nested_value = value + offset + 4U;
        nested_length = (p0468_rgi_size)tlv_length - 4U;

        if (identifier == 0U) {
            if (nested_length != 1U && nested_length != 2U) {
                return P0468_RGI_PARSE_VALUE;
            }
            if (parsed->lane.lane_count < P0468_RGI_LANES_MAX) {
                lane = &parsed->lane.lanes[parsed->lane.lane_count++];
                lane->angle_highlight = 1000;
                lane->position = nested_length == 2U ?
                    p0468_u16_be(nested_value) : nested_value[0];
                lane->present |= P0468_RGI_LANE_ITEM_POSITION;
            } else {
                parsed->lane.complete = 0U;
                lane = (p0468_rgi_lane_item *)0;
            }
            offset += (p0468_rgi_size)tlv_length;
            continue;
        }

        if (identifier == 1U || identifier == 2U || identifier == 3U) {
            if (lane == (p0468_rgi_lane_item *)0 &&
                parsed->lane.lane_count < P0468_RGI_LANES_MAX) {
                lane = &parsed->lane.lanes[parsed->lane.lane_count];
                lane->position = parsed->lane.lane_count;
                lane->angle_highlight = 1000;
                ++parsed->lane.lane_count;
            }
        }

        if (identifier == 1U) {
            result = p0468_scalar(nested_value, nested_length, &scalar);
            if (result != P0468_RGI_PARSE_OK) return result;
            if (lane != (p0468_rgi_lane_item *)0) {
                lane->status = (p0468_rgi_u8)scalar;
                lane->present |= P0468_RGI_LANE_ITEM_STATUS;
            }
        } else if (identifier == 2U) {
            if (nested_length == 0U) return P0468_RGI_PARSE_VALUE;
            angle_offset = 0U;
            while (angle_offset < nested_length) {
                p0468_rgi_s16 angle;
                if (nested_length - angle_offset >= 2U) {
                    angle = (p0468_rgi_s16)p0468_u16_be(
                        nested_value + angle_offset);
                    angle_offset += 2U;
                } else {
                    angle = (p0468_rgi_s16)nested_value[angle_offset++];
                }
                if (lane != (p0468_rgi_lane_item *)0 &&
                    lane->angle_count < P0468_RGI_LANE_ANGLES_MAX) {
                    lane->angles[lane->angle_count++] = angle;
                    if (!(lane->present & P0468_RGI_LANE_ITEM_HIGHLIGHT) &&
                        lane->angle_highlight == 1000) {
                        lane->angle_highlight = angle;
                    }
                } else {
                    parsed->lane.complete = 0U;
                }
            }
            if (lane != (p0468_rgi_lane_item *)0) {
                lane->present |= P0468_RGI_LANE_ITEM_ANGLES;
            }
        } else if (identifier == 3U) {
            result = p0468_scalar(nested_value, nested_length, &scalar);
            if (result != P0468_RGI_PARSE_OK) return result;
            if (lane != (p0468_rgi_lane_item *)0) {
                lane->angle_highlight = (p0468_rgi_s16)scalar;
                lane->present |= P0468_RGI_LANE_ITEM_HIGHLIGHT;
            }
        }
        offset += (p0468_rgi_size)tlv_length;
    }
    while (first_lane < parsed->lane.lane_count) {
        if ((parsed->lane.lanes[first_lane].present &
             (P0468_RGI_LANE_ITEM_POSITION | P0468_RGI_LANE_ITEM_STATUS)) !=
            (P0468_RGI_LANE_ITEM_POSITION | P0468_RGI_LANE_ITEM_STATUS)) {
            parsed->lane.complete = 0U;
        }
        ++first_lane;
    }
    return P0468_RGI_PARSE_OK;
}

static int p0468_parse_lane_tlv(p0468_rgi_frame *parsed,
                                 p0468_rgi_u16 identifier,
                                 const p0468_rgi_u8 *value,
                                 p0468_rgi_size length)
{
    p0468_rgi_u64 scalar;
    int result;

    if (identifier == 0U) {
        if (length != 2U) return P0468_RGI_PARSE_VALUE;
        parsed->component_id = p0468_u16_be(value);
        parsed->component_present = 1U;
        return P0468_RGI_PARSE_OK;
    }
    if (identifier == 1U) {
        result = p0468_scalar(value, length, &scalar);
        if (result != P0468_RGI_PARSE_OK) return result;
        parsed->lane.index = (p0468_rgi_u16)scalar;
        parsed->lane.present |= P0468_RGI_LANE_INDEX;
        return P0468_RGI_PARSE_OK;
    }
    if (identifier == 2U) {
        parsed->lane.present |= P0468_RGI_LANE_INFORMATIONS;
        return p0468_parse_lane_information(parsed, value, length);
    }
    if (identifier == 3U) {
        p0468_text(parsed->lane.description, value, length);
        parsed->lane.present |= P0468_RGI_LANE_DESCRIPTION;
    }
    return P0468_RGI_PARSE_OK;
}

int p0468_rgi_parse_frame(const p0468_rgi_u8 *frame,
                           p0468_rgi_size length,
                           p0468_rgi_frame *parsed)
{
    p0468_rgi_size offset;
    p0468_rgi_u16 declared;
    int result;

    if (frame == (const p0468_rgi_u8 *)0 ||
        parsed == (p0468_rgi_frame *)0) return P0468_RGI_PARSE_NULL;
    p0468_zero(parsed, (p0468_rgi_size)sizeof(*parsed));
    if (length < 6U) return P0468_RGI_PARSE_SHORT;
    if (frame[0] != 0x40U || frame[1] != 0x40U) {
        return P0468_RGI_PARSE_HEADER;
    }
    declared = p0468_u16_be(frame + 2U);
    if ((p0468_rgi_size)declared != length) return P0468_RGI_PARSE_LENGTH;
    parsed->message_id = p0468_u16_be(frame + 4U);
    if (parsed->message_id != P0468_RGI_MESSAGE_UPDATE &&
        parsed->message_id != P0468_RGI_MESSAGE_MANEUVER &&
        parsed->message_id != P0468_RGI_MESSAGE_LANE) {
        return P0468_RGI_PARSE_MESSAGE;
    }
    if (parsed->message_id == P0468_RGI_MESSAGE_LANE) {
        parsed->lane.complete = 1U;
    }

    offset = 6U;
    while (offset < length) {
        p0468_rgi_u16 tlv_length;
        p0468_rgi_u16 identifier;
        const p0468_rgi_u8 *value;
        p0468_rgi_size value_length;
        if (length - offset < 4U) return P0468_RGI_PARSE_TLV;
        tlv_length = p0468_u16_be(frame + offset);
        identifier = p0468_u16_be(frame + offset + 2U);
        if (tlv_length < 4U || (p0468_rgi_size)tlv_length > length - offset) {
            return P0468_RGI_PARSE_TLV;
        }
        value = frame + offset + 4U;
        value_length = (p0468_rgi_size)tlv_length - 4U;
        if (parsed->message_id == P0468_RGI_MESSAGE_UPDATE) {
            result = p0468_parse_route_tlv(parsed, identifier, value,
                                            value_length);
        } else if (parsed->message_id == P0468_RGI_MESSAGE_MANEUVER) {
            result = p0468_parse_maneuver_tlv(parsed, identifier, value,
                                               value_length);
        } else {
            result = p0468_parse_lane_tlv(parsed, identifier, value,
                                           value_length);
        }
        if (result != P0468_RGI_PARSE_OK) return result;
        offset += (p0468_rgi_size)tlv_length;
    }
    return offset == length ? P0468_RGI_PARSE_OK : P0468_RGI_PARSE_TLV;
}
