#ifndef P0468_RGI_PARSER_H
#define P0468_RGI_PARSER_H

typedef unsigned char p0468_rgi_u8;
typedef unsigned short p0468_rgi_u16;
typedef unsigned int p0468_rgi_u32;
typedef unsigned long long p0468_rgi_u64;
typedef signed short p0468_rgi_s16;
typedef unsigned long p0468_rgi_size;

#define P0468_RGI_TEXT_MAX 96U
#define P0468_RGI_MANEUVER_LIST_MAX 16U
#define P0468_RGI_JUNCTION_ANGLES_MAX 16U
#define P0468_RGI_LANES_MAX 8U
#define P0468_RGI_LANE_ANGLES_MAX 16U

#define P0468_RGI_MESSAGE_UPDATE   0x5201U
#define P0468_RGI_MESSAGE_MANEUVER 0x5202U
#define P0468_RGI_MESSAGE_LANE     0x5204U

enum p0468_rgi_parse_result {
    P0468_RGI_PARSE_OK = 0,
    P0468_RGI_PARSE_NULL = -1,
    P0468_RGI_PARSE_SHORT = -2,
    P0468_RGI_PARSE_HEADER = -3,
    P0468_RGI_PARSE_LENGTH = -4,
    P0468_RGI_PARSE_MESSAGE = -5,
    P0468_RGI_PARSE_TLV = -6,
    P0468_RGI_PARSE_VALUE = -7
};

enum p0468_rgi_route_field {
    P0468_RGI_ROUTE_STATE = 1U << 0,
    P0468_RGI_ROUTE_MANEUVER_STATE = 1U << 1,
    P0468_RGI_ROUTE_CURRENT_ROAD = 1U << 2,
    P0468_RGI_ROUTE_DESTINATION = 1U << 3,
    P0468_RGI_ROUTE_ETA = 1U << 4,
    P0468_RGI_ROUTE_TIME_REMAINING = 1U << 5,
    P0468_RGI_ROUTE_DISTANCE_REMAINING = 1U << 6,
    P0468_RGI_ROUTE_DISTANCE_STRING = 1U << 7,
    P0468_RGI_ROUTE_DISTANCE_UNITS = 1U << 8,
    P0468_RGI_ROUTE_DISTANCE_TO_MANEUVER = 1U << 9,
    P0468_RGI_ROUTE_MANEUVER_DISTANCE_STRING = 1U << 10,
    P0468_RGI_ROUTE_MANEUVER_DISTANCE_UNITS = 1U << 11,
    P0468_RGI_ROUTE_MANEUVER_LIST = 1U << 12,
    P0468_RGI_ROUTE_MANEUVER_COUNT = 1U << 13,
    P0468_RGI_ROUTE_VISIBLE = 1U << 14,
    P0468_RGI_ROUTE_LANE_INDEX = 1U << 15,
    P0468_RGI_ROUTE_LANE_TOTAL = 1U << 16,
    P0468_RGI_ROUTE_LANE_SHOWING = 1U << 17,
    P0468_RGI_ROUTE_SOURCE_NAME = 1U << 18,
    P0468_RGI_ROUTE_SOURCE_SUPPORTS = 1U << 19
};

enum p0468_rgi_maneuver_field {
    P0468_RGI_MANEUVER_INDEX = 1U << 0,
    P0468_RGI_MANEUVER_DESCRIPTION = 1U << 1,
    P0468_RGI_MANEUVER_TYPE = 1U << 2,
    P0468_RGI_MANEUVER_AFTER_ROAD = 1U << 3,
    P0468_RGI_MANEUVER_DISTANCE = 1U << 4,
    P0468_RGI_MANEUVER_DISTANCE_STRING = 1U << 5,
    P0468_RGI_MANEUVER_DISTANCE_UNITS = 1U << 6,
    P0468_RGI_MANEUVER_DRIVING_SIDE = 1U << 7,
    P0468_RGI_MANEUVER_JUNCTION_TYPE = 1U << 8,
    P0468_RGI_MANEUVER_JUNCTION_ANGLES = 1U << 9,
    P0468_RGI_MANEUVER_EXIT_ANGLE = 1U << 10,
    P0468_RGI_MANEUVER_LINKED_LANE = 1U << 11,
    P0468_RGI_MANEUVER_EXIT_INFO = 1U << 12
};

enum p0468_rgi_lane_field {
    P0468_RGI_LANE_INDEX = 1U << 0,
    P0468_RGI_LANE_INFORMATIONS = 1U << 1,
    P0468_RGI_LANE_DESCRIPTION = 1U << 2
};

enum p0468_rgi_lane_item_field {
    P0468_RGI_LANE_ITEM_POSITION = 1U << 0,
    P0468_RGI_LANE_ITEM_STATUS = 1U << 1,
    P0468_RGI_LANE_ITEM_ANGLES = 1U << 2,
    P0468_RGI_LANE_ITEM_HIGHLIGHT = 1U << 3
};

typedef struct {
    p0468_rgi_u32 present;
    p0468_rgi_u8 state;
    p0468_rgi_u8 maneuver_state;
    char current_road[P0468_RGI_TEXT_MAX];
    char destination[P0468_RGI_TEXT_MAX];
    p0468_rgi_u64 eta;
    p0468_rgi_u64 time_remaining;
    p0468_rgi_u32 distance_remaining;
    char distance_string[P0468_RGI_TEXT_MAX];
    p0468_rgi_u8 distance_units;
    p0468_rgi_u32 distance_to_maneuver;
    char maneuver_distance_string[P0468_RGI_TEXT_MAX];
    p0468_rgi_u8 maneuver_distance_units;
    p0468_rgi_u16 maneuver_list[P0468_RGI_MANEUVER_LIST_MAX];
    p0468_rgi_u8 maneuver_list_count;
    p0468_rgi_u16 maneuver_count;
    p0468_rgi_u8 visible;
    p0468_rgi_u16 lane_index;
    p0468_rgi_u16 lane_total;
    p0468_rgi_u8 lane_showing;
    char source_name[P0468_RGI_TEXT_MAX];
    p0468_rgi_u8 source_supports;
} p0468_rgi_route_update;

typedef struct {
    p0468_rgi_u32 present;
    p0468_rgi_u16 index;
    char description[P0468_RGI_TEXT_MAX];
    p0468_rgi_u8 type;
    char after_road[P0468_RGI_TEXT_MAX];
    p0468_rgi_u32 distance;
    char distance_string[P0468_RGI_TEXT_MAX];
    p0468_rgi_u8 distance_units;
    p0468_rgi_u8 driving_side;
    p0468_rgi_u8 junction_type;
    p0468_rgi_s16 junction_angles[P0468_RGI_JUNCTION_ANGLES_MAX];
    p0468_rgi_u8 junction_angle_count;
    p0468_rgi_s16 exit_angle;
    p0468_rgi_u16 linked_lane;
    char exit_info[P0468_RGI_TEXT_MAX];
} p0468_rgi_maneuver_update;

typedef struct {
    p0468_rgi_u8 present;
    p0468_rgi_u16 position;
    p0468_rgi_u8 status;
    p0468_rgi_s16 angles[P0468_RGI_LANE_ANGLES_MAX];
    p0468_rgi_u8 angle_count;
    p0468_rgi_s16 angle_highlight;
} p0468_rgi_lane_item;

typedef struct {
    p0468_rgi_u32 present;
    p0468_rgi_u16 index;
    p0468_rgi_lane_item lanes[P0468_RGI_LANES_MAX];
    p0468_rgi_u8 lane_count;
    p0468_rgi_u8 complete;
    char description[P0468_RGI_TEXT_MAX];
} p0468_rgi_lane_update;

typedef struct {
    p0468_rgi_u16 message_id;
    p0468_rgi_u16 component_id;
    p0468_rgi_u8 component_present;
    p0468_rgi_route_update route;
    p0468_rgi_maneuver_update maneuver;
    p0468_rgi_lane_update lane;
} p0468_rgi_frame;

int p0468_rgi_parse_frame(const p0468_rgi_u8 *frame,
                           p0468_rgi_size length,
                           p0468_rgi_frame *parsed);

#endif
