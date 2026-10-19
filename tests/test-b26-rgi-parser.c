#include "../native/p0468-rgi-parser/p0468_rgi_parser.h"

#include <assert.h>
#include <string.h>

static void test_route_update(void)
{
    static const p0468_rgi_u8 frame[] = {
        0x40,0x40,0x00,0x39,0x52,0x01,
        0x00,0x06,0x00,0x00,0x00,0x10,
        0x00,0x05,0x00,0x01,0x01,
        0x00,0x08,0x00,0x07,0x00,0x00,0x02,0x84,
        0x00,0x08,0x00,0x0A,0x00,0x00,0x00,0x0C,
        0x00,0x08,0x00,0x0D,0x00,0x00,0x00,0x01,
        0x00,0x06,0x00,0x0E,0x00,0x0B,
        0x00,0x05,0x00,0x0F,0x01,
        0x00,0x05,0x00,0x14,0x01
    };
    p0468_rgi_frame parsed;
    assert(p0468_rgi_parse_frame(frame, sizeof(frame), &parsed) == 0);
    assert(parsed.message_id == 0x5201U);
    assert(parsed.component_present && parsed.component_id == 0x0010U);
    assert(parsed.route.state == 1U);
    assert(parsed.route.distance_remaining == 644U);
    assert(parsed.route.distance_to_maneuver == 12U);
    assert(parsed.route.maneuver_list_count == 2U);
    assert(parsed.route.maneuver_list[0] == 0U);
    assert(parsed.route.maneuver_list[1] == 1U);
    assert(parsed.route.maneuver_count == 11U);
    assert(parsed.route.visible == 1U);
    assert(parsed.route.source_supports == 1U);
}

static void test_maneuver_update(void)
{
    static const p0468_rgi_u8 frame[] = {
        0x40,0x40,0x00,0x46,0x52,0x02,
        0x00,0x06,0x00,0x00,0x00,0x10,
        0x00,0x06,0x00,0x01,0x00,0x02,
        0x00,0x0F,0x00,0x02,'T','u','r','n',' ','r','i','g','h','t',0x00,
        0x00,0x05,0x00,0x03,0x02,
        0x00,0x0C,0x00,0x04,'T','e','s','t',' ','R','d',0x00,
        0x00,0x08,0x00,0x05,0x00,0x00,0x00,0x28,
        0x00,0x06,0x00,0x0A,0xFF,0xA6,
        0x00,0x06,0x00,0x0B,0x00,0x5A
    };
    p0468_rgi_frame parsed;
    assert(p0468_rgi_parse_frame(frame, sizeof(frame), &parsed) == 0);
    assert(parsed.message_id == 0x5202U);
    assert(parsed.maneuver.index == 2U);
    assert(parsed.maneuver.type == 2U);
    assert(parsed.maneuver.distance == 40U);
    assert(strcmp(parsed.maneuver.description, "Turn right") == 0);
    assert(strcmp(parsed.maneuver.after_road, "Test Rd") == 0);
    assert(parsed.maneuver.junction_angle_count == 1U);
    assert(parsed.maneuver.junction_angles[0] == -90);
    assert(parsed.maneuver.exit_angle == 90);
}

static void test_lane_guidance_update(void)
{
    static const p0468_rgi_u8 frame[] = {
        0x40,0x40,0x00,0x55,0x52,0x04,
        0x00,0x06,0x00,0x00,0x00,0x10,
        0x00,0x06,0x00,0x01,0x00,0x07,
        0x00,0x1D,0x00,0x02,
          0x00,0x06,0x00,0x00,0x00,0x00,
          0x00,0x05,0x00,0x01,0x00,
          0x00,0x08,0x00,0x02,0xFF,0xA6,0x00,0x00,
          0x00,0x06,0x00,0x03,0xFF,0xA6,
        0x00,0x1B,0x00,0x02,
          0x00,0x06,0x00,0x00,0x00,0x01,
          0x00,0x05,0x00,0x01,0x02,
          0x00,0x06,0x00,0x02,0x00,0x00,
          0x00,0x06,0x00,0x03,0x00,0x00,
        0x00,0x0B,0x00,0x03,'A','5',' ','e','x','i','t'
    };
    p0468_rgi_frame parsed;
    assert(p0468_rgi_parse_frame(frame, sizeof(frame), &parsed) == 0);
    assert(parsed.message_id == 0x5204U);
    assert(parsed.component_present && parsed.component_id == 0x0010U);
    assert(parsed.lane.index == 7U);
    assert(parsed.lane.lane_count == 2U);
    assert(parsed.lane.complete == 1U);
    assert(parsed.lane.lanes[0].position == 0U);
    assert(parsed.lane.lanes[0].status == 0U);
    assert(parsed.lane.lanes[0].angle_count == 2U);
    assert(parsed.lane.lanes[0].angles[0] == -90);
    assert(parsed.lane.lanes[0].angles[1] == 0);
    assert(parsed.lane.lanes[0].angle_highlight == -90);
    assert(parsed.lane.lanes[1].position == 1U);
    assert(parsed.lane.lanes[1].status == 2U);
    assert(parsed.lane.lanes[1].angle_count == 1U);
    assert(parsed.lane.lanes[1].angle_highlight == 0);
    assert(strcmp(parsed.lane.description, "A5 exit") == 0);
}

static void test_malformed_nested_lane_tlv_is_rejected(void)
{
    static const p0468_rgi_u8 frame[] = {
        0x40,0x40,0x00,0x0E,0x52,0x04,
        0x00,0x08,0x00,0x02,0x00,0x03,0x00,0x00
    };
    p0468_rgi_frame parsed;
    assert(p0468_rgi_parse_frame(frame, sizeof(frame), &parsed) ==
           P0468_RGI_PARSE_TLV);
}

static void test_rejections(void)
{
    static const p0468_rgi_u8 short_frame[] = {0x40,0x40,0x00};
    static const p0468_rgi_u8 bad_length[] = {
        0x40,0x40,0x00,0x07,0x52,0x01
    };
    static const p0468_rgi_u8 bad_tlv[] = {
        0x40,0x40,0x00,0x0A,0x52,0x01,0x00,0x03,0x00,0x01
    };
    p0468_rgi_frame parsed;
    assert(p0468_rgi_parse_frame(short_frame, sizeof(short_frame), &parsed) ==
           P0468_RGI_PARSE_SHORT);
    assert(p0468_rgi_parse_frame(bad_length, sizeof(bad_length), &parsed) ==
           P0468_RGI_PARSE_LENGTH);
    assert(p0468_rgi_parse_frame(bad_tlv, sizeof(bad_tlv), &parsed) ==
           P0468_RGI_PARSE_TLV);
}

static void test_unknown_tlv_is_forward_compatible(void)
{
    static const p0468_rgi_u8 frame[] = {
        0x40,0x40,0x00,0x0F,0x52,0x01,
        0x00,0x09,0x12,0x34,0xAA,0xBB,0xCC,0xDD,0xEE
    };
    p0468_rgi_frame parsed;
    assert(p0468_rgi_parse_frame(frame, sizeof(frame), &parsed) == 0);
    assert(parsed.message_id == 0x5201U);
    assert(parsed.route.present == 0U);
}

int main(void)
{
    test_route_update();
    test_maneuver_update();
    test_lane_guidance_update();
    test_malformed_nested_lane_tlv_is_rejected();
    test_rejections();
    test_unknown_tlv_is_forward_compatible();
    return 0;
}
