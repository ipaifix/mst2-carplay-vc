#include "p0468_rgi_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int hex_value(char character)
{
    if (character >= '0' && character <= '9') return character - '0';
    if (character >= 'a' && character <= 'f') return character - 'a' + 10;
    if (character >= 'A' && character <= 'F') return character - 'A' + 10;
    return -1;
}

static const char *maneuver_name(unsigned int type)
{
    static const char *const names[] = {
        "no-turn", "left", "right", "straight", "u-turn", "follow-road",
        "enter-roundabout", "exit-roundabout", "off-ramp", "on-ramp",
        "arrive-end", "start-route", "arrive-destination", "keep-left",
        "keep-right", "enter-ferry", "exit-ferry", "change-ferry",
        "start-route-u-turn", "u-turn-roundabout", "left-at-end",
        "right-at-end", "highway-off-left", "highway-off-right",
        "arrive-left", "arrive-right", "u-turn-when-possible",
        "arrive-end-directions"
    };
    if (type < sizeof(names) / sizeof(names[0])) return names[type];
    if (type >= 28U && type <= 46U) return "roundabout-exit";
    if (type == 47U) return "sharp-left";
    if (type == 48U) return "sharp-right";
    if (type == 49U) return "slight-left";
    if (type == 50U) return "slight-right";
    if (type == 51U) return "change-highway";
    if (type == 52U) return "change-highway-left";
    if (type == 53U) return "change-highway-right";
    return "unknown";
}

static void print_frame(unsigned long index, const p0468_rgi_frame *frame,
                        int show_text)
{
    unsigned int angle;
    if (frame->message_id == P0468_RGI_MESSAGE_UPDATE) {
        printf("frame=%lu id=0x5201 component=0x%04X", index,
               (unsigned int)frame->component_id);
        if (frame->route.present & P0468_RGI_ROUTE_STATE)
            printf(" route_state=%u", (unsigned int)frame->route.state);
        if (frame->route.present & P0468_RGI_ROUTE_DISTANCE_REMAINING)
            printf(" remaining=%u", frame->route.distance_remaining);
        if (frame->route.present & P0468_RGI_ROUTE_DISTANCE_TO_MANEUVER)
            printf(" next=%u", frame->route.distance_to_maneuver);
        if (frame->route.present & P0468_RGI_ROUTE_MANEUVER_COUNT)
            printf(" maneuver_count=%u", (unsigned int)frame->route.maneuver_count);
        if (frame->route.present & P0468_RGI_ROUTE_VISIBLE)
            printf(" visible=%u", (unsigned int)frame->route.visible);
        if (frame->route.present & P0468_RGI_ROUTE_DESTINATION)
            printf(" destination=%s", show_text ? frame->route.destination : "<redacted>");
        if (frame->route.present & P0468_RGI_ROUTE_SOURCE_NAME)
            printf(" source=%s", show_text ? frame->route.source_name : "<redacted>");
        putchar('\n');
        return;
    }
    if (frame->message_id == P0468_RGI_MESSAGE_MANEUVER) {
        printf("frame=%lu id=0x5202 component=0x%04X", index,
               (unsigned int)frame->component_id);
        if (frame->maneuver.present & P0468_RGI_MANEUVER_INDEX)
            printf(" index=%u", (unsigned int)frame->maneuver.index);
        if (frame->maneuver.present & P0468_RGI_MANEUVER_TYPE)
            printf(" type=%u(%s)", (unsigned int)frame->maneuver.type,
                   maneuver_name(frame->maneuver.type));
        if (frame->maneuver.present & P0468_RGI_MANEUVER_DISTANCE)
            printf(" distance=%u", frame->maneuver.distance);
        if (frame->maneuver.present & P0468_RGI_MANEUVER_DESCRIPTION)
            printf(" description=%s", show_text ? frame->maneuver.description : "<redacted>");
        if (frame->maneuver.present & P0468_RGI_MANEUVER_AFTER_ROAD)
            printf(" road=%s", show_text ? frame->maneuver.after_road : "<redacted>");
        if (frame->maneuver.present & P0468_RGI_MANEUVER_JUNCTION_ANGLES) {
            printf(" angles=");
            for (angle = 0U; angle < frame->maneuver.junction_angle_count; ++angle) {
                if (angle != 0U) putchar(',');
                printf("%d", (int)frame->maneuver.junction_angles[angle]);
            }
        }
        if (frame->maneuver.present & P0468_RGI_MANEUVER_EXIT_ANGLE)
            printf(" exit_angle=%d", (int)frame->maneuver.exit_angle);
        putchar('\n');
        return;
    }
    if (frame->message_id == P0468_RGI_MESSAGE_LANE) {
        unsigned int lane_index;
        printf("frame=%lu id=0x5204 component=0x%04X", index,
               (unsigned int)frame->component_id);
        if (frame->lane.present & P0468_RGI_LANE_INDEX)
            printf(" index=%u", (unsigned int)frame->lane.index);
        printf(" lanes=%u complete=%u", (unsigned int)frame->lane.lane_count,
               (unsigned int)frame->lane.complete);
        for (lane_index = 0U; lane_index < frame->lane.lane_count;
             ++lane_index) {
            const p0468_rgi_lane_item *lane = &frame->lane.lanes[lane_index];
            printf(" lane%u={position=%u,status=%u,highlight=%d,angles=",
                   lane_index, (unsigned int)lane->position,
                   (unsigned int)lane->status, (int)lane->angle_highlight);
            for (angle = 0U; angle < lane->angle_count; ++angle) {
                if (angle != 0U) putchar(',');
                printf("%d", (int)lane->angles[angle]);
            }
            putchar('}');
        }
        if (frame->lane.present & P0468_RGI_LANE_DESCRIPTION)
            printf(" description=%s", show_text ? frame->lane.description : "<redacted>");
        putchar('\n');
        return;
    }
    printf("frame=%lu id=0x%04X component=0x%04X\n", index,
           (unsigned int)frame->message_id,
           (unsigned int)frame->component_id);
}

int main(int argc, char **argv)
{
    FILE *input;
    char line[4096];
    unsigned long index = 0UL;
    int show_text = 0;

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: %s capture.log [--show-text]\n", argv[0]);
        return 2;
    }
    if (argc == 3) {
        if (strcmp(argv[2], "--show-text") != 0) return 2;
        show_text = 1;
    }
    input = fopen(argv[1], "rb");
    if (input == NULL) {
        perror(argv[1]);
        return 2;
    }
    while (fgets(line, sizeof(line), input) != NULL) {
        char *hex = strstr(line, " data=");
        p0468_rgi_u8 bytes[1024];
        p0468_rgi_size count = 0U;
        p0468_rgi_frame parsed;
        int result;
        if (hex == NULL) continue;
        hex += 6;
        while (hex[0] != '\0' && hex[0] != '\r' && hex[0] != '\n') {
            int high = hex_value(hex[0]);
            int low = hex_value(hex[1]);
            if (high < 0 || low < 0 || count >= sizeof(bytes)) {
                fprintf(stderr, "frame %lu: invalid hex capture\n", index);
                fclose(input);
                return 1;
            }
            bytes[count++] = (p0468_rgi_u8)((high << 4) | low);
            hex += 2;
        }
        result = p0468_rgi_parse_frame(bytes, count, &parsed);
        if (result != P0468_RGI_PARSE_OK) {
            fprintf(stderr, "frame %lu: parse error %d\n", index, result);
            fclose(input);
            return 1;
        }
        print_frame(index, &parsed, show_text);
        ++index;
    }
    fclose(input);
    printf("decoded_frames=%lu\n", index);
    return index == 0UL ? 1 : 0;
}
