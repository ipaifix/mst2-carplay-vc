package org.mst2research.guidance;

public final class C314RichPolicyTest {
    private static int checks;
    private static void check(boolean value, String label) {
        ++checks;
        if (!value) throw new RuntimeException(label);
    }
    private static C314RichPolicy.Context context() {
        C314RichPolicy.Context c = new C314RichPolicy.Context();
        c.transportVerified = c.active = c.maneuverValid = true;
        c.routeState = 1; c.routeGeneration = 1001; c.routeAgeMs = 0;
        c.maneuverType = 2; c.maneuverIndex = 2;
        return c;
    }
    private static C314RichPolicy.Geometry geometry(int[] angles) {
        C314RichPolicy.Geometry g = new C314RichPolicy.Geometry();
        g.maneuverType = 2; g.maneuverIndex = 2; g.routeGeneration = 1001;
        g.present = 1 | 4 | 256 | 512 | 1024; g.junctionType = 0; g.exitAngle = 90;
        g.complete = g.coherent = true; g.otherAngles = angles;
        return g;
    }
    private static C314RichPolicy.Lane lane(int position, int status, boolean primary, int angle, boolean list, int[] others) {
        C314RichPolicy.Lane lane = new C314RichPolicy.Lane();
        lane.position = position; lane.status = status; lane.complete = true;
        lane.present = 3 | (primary ? 8 : 0) | (list ? 4 : 0);
        lane.highlightedAngle = angle; lane.angles = others;
        return lane;
    }
    private static C314RichPolicy.LaneEvent event(C314RichPolicy.Lane[] lanes) {
        C314RichPolicy.LaneEvent event = new C314RichPolicy.LaneEvent();
        event.routeGeneration = 1001; event.maneuverIndex = 2;
        event.eventIndex = event.currentEventIndex = event.linkedEventIndex = 7;
        event.currentIndexPresent = event.linkedIndexPresent = true;
        event.showing = event.complete = event.coherent = true;
        event.lanes = lanes;
        return event;
    }
    public static void main(String[] args) {
        int[][] shapes = {{-90}, {0}, {-90, 0}, {0, -90}};
        for (int i = 0; i < shapes.length; ++i) {
            C314RichPolicy.ArrowResult r = C314RichPolicy.arrow(context(), geometry(shapes[i]));
            check(r.reason == 0 && r.override.main == 13 && r.override.direction == 192, "stock pair");
            int[] sides = r.override.sideCodes();
            check(sides.length == (i < 2 ? 1 : 2), "stock count");
            check(sides[0] == (i == 1 ? 0 : 64), "stock first");
            if (sides.length == 2) check(sides[1] == 0, "stock wire order");
            sides[0] = 255;
            check(r.override.sideCodes()[0] != 255, "immutable sides");
            check(r.clearPreviousEnrichment, "explicit replacement");
        }
        int[][] rejected = {{90}, {-90, -90}, {-80}, {1000}, {-181}, {}, {-90, 0, 45}};
        for (int i = 0; i < rejected.length; ++i)
            check(C314RichPolicy.arrow(context(), geometry(rejected[i])).override == null, "no guessed geometry");
        for (int i = 0; i < 9; ++i) {
            C314RichPolicy.Context c = context(); C314RichPolicy.Geometry g = geometry(new int[]{-90});
            if (i == 0) c.active = false;
            if (i == 1) c.maneuverValid = false;
            if (i == 2) c.transportVerified = false;
            if (i == 3) c.routeAgeMs = 10001;
            if (i == 4) c.routeAgeMs = -1;
            if (i == 5) c.routeGeneration = 0;
            if (i == 6) g.routeGeneration = 1002;
            if (i == 7) g.maneuverIndex = 3;
            if (i == 8) g.maneuverType = 1;
            C314RichPolicy.ArrowResult r = C314RichPolicy.arrow(c, g);
            check(r.override == null && r.clearPreviousEnrichment, "context clear");
        }
        for (int state = 0; state <= 6; ++state) {
            C314RichPolicy.Context c = context(); c.routeState = state;
            check((C314RichPolicy.arrow(c, geometry(new int[]{-90})).override != null) == (state == 1), "route state");
        }
        for (int bit = 0; bit < 5; ++bit) {
            int[] required = {1, 4, 256, 512, 1024};
            C314RichPolicy.Geometry g = geometry(new int[]{-90}); g.present &= ~required[bit];
            check(C314RichPolicy.arrow(context(), g).override == null, "presence cannot default");
        }
        for (int mode = 0; mode < 6; ++mode) {
            C314RichPolicy.Geometry g = geometry(new int[]{-90});
            if (mode == 0) g.complete = false;
            if (mode == 1) g.coherent = false;
            if (mode == 2) g.otherAngles = null;
            if (mode == 3) g.otherAngles = new int[17];
            if (mode == 4) g.junctionType = 1;
            if (mode == 5) g.exitAngle = 0;
            check(C314RichPolicy.arrow(context(), g).override == null, "geometry refusal");
        }
        C314RichPolicy.Context fresh = context(); fresh.routeAgeMs = 10000;
        check(C314RichPolicy.arrow(fresh, geometry(new int[]{-90})).override != null, "freshness boundary");
        // Correct Apple contract, but no rich KEEP production without captured shape qualification.
        for (int type = 13; type <= 14; ++type) {
            C314RichPolicy.Context c = context(); c.maneuverType = type;
            C314RichPolicy.Geometry g = geometry(new int[]{type == 13 ? 45 : -45});
            g.maneuverType = type; g.exitAngle = type == 13 ? -45 : 45;
            check(C314RichPolicy.geometryCompatible(c, g), "exit separate from alternatives");
            check(C314RichPolicy.arrow(c, g).override == null, "fork study only");
        }
        check(C314RichPolicy.arrow(null, null).override == null, "null clear");
        check(C314RichPolicy.arrow(context(), null).override == null, "no definition");
        check(C314RichPolicy.simpleLaneDirection(lane(0, 2, true, 45, false, new int[0])) == 224, "primary-only recommendation");
        check(C314RichPolicy.simpleLaneDirection(lane(0, 2, true, 0, false, new int[0])) == 0, "real zero primary");
        check(C314RichPolicy.simpleLaneDirection(lane(0, 2, false, 0, false, new int[0])) == -1, "absent primary not zero");
        check(C314RichPolicy.simpleLaneDirection(lane(0, 2, true, 45, true, new int[]{0})) == -1, "multi-branch not flattened");
        check(C314RichPolicy.simpleLaneDirection(lane(0, 2, true, 45, true, new int[]{46})) == 224, "single existing direction sector");
        check(C314RichPolicy.simpleLaneDirection(lane(0, 0, false, 0, true, new int[]{0})) == 0, "unrecommended explicit angle");
        check(C314RichPolicy.simpleLaneDirection(lane(0, 1, false, 0, true, new int[]{0})) == -1, "recommendation requires primary");
        for (int angle = -180; angle <= 180; ++angle) {
            int dir = C314RichPolicy.simpleLaneDirection(lane(0, 2, true, angle, false, new int[0]));
            check((dir >= 0) == (angle >= -157 && angle <= 157), "lane angle domain");
        }
        int[] invalid = {-32768, -1000, 1000, 32767, Integer.MIN_VALUE, Integer.MAX_VALUE};
        for (int i = 0; i < invalid.length; ++i)
            check(C314RichPolicy.simpleLaneDirection(lane(0, 2, true, invalid[i], false, new int[0])) == -1, "lane invalid angle");
        C314RichPolicy.Lane[] lanes = {lane(4,2,true,45,false,new int[0]), lane(1,0,false,0,true,new int[]{0})};
        C314RichPolicy.LanesResult result = C314RichPolicy.lanes(context(), event(lanes));
        check(result.reason == 0 && result.arrows().length == 2, "whole lane event");
        check(result.arrows()[0].position == 1 && result.arrows()[1].position == 4, "sort by position");
        check(result.arrows()[1].guidanceInfo == 2, "preserve status");
        C314RichPolicy.LaneArrow[] values = result.arrows(); values[0] = null;
        check(result.arrows()[0] != null, "immutable event");
        for (int mode = 0; mode < 16; ++mode) {
            C314RichPolicy.LaneEvent e = event(lanes);
            if (mode == 0) e.showing = false;
            if (mode == 1) e.complete = false;
            if (mode == 2) e.coherent = false;
            if (mode == 3) e.routeGeneration = 1002;
            if (mode == 4) e.maneuverIndex = 3;
            if (mode == 5) e.ageMs = 10001;
            if (mode == 6) e.ageMs = -1;
            if (mode == 7) e.linkedEventIndex = 8;
            if (mode == 8) e.eventIndex = e.linkedEventIndex = 65536;
            if (mode == 9) e.lanes = null;
            if (mode == 10) e.lanes = new C314RichPolicy.Lane[0];
            if (mode == 11) e.lanes = new C314RichPolicy.Lane[7];
            if (mode == 12) e.lanes = new C314RichPolicy.Lane[]{lanes[0], lanes[0]};
            if (mode == 13) e.lanes = new C314RichPolicy.Lane[]{lanes[0], lane(3,2,true,45,true,new int[]{0})};
            if (mode == 14) e.currentIndexPresent = false;
            if (mode == 15) e.currentEventIndex = 8;
            C314RichPolicy.LanesResult bad = C314RichPolicy.lanes(context(), e);
            check(bad.arrows().length == 0 && bad.clearPreviousEnrichment, "clear invalid complete event");
        }
        C314RichPolicy.LaneEvent noOptionalLink = event(lanes);
        noOptionalLink.linkedIndexPresent = false;
        noOptionalLink.linkedEventIndex = 0;
        check(C314RichPolicy.lanes(context(), noOptionalLink).arrows().length == 2, "optional maneuver link not invented");
        for (int mode = 0; mode < 5; ++mode) {
            C314RichPolicy.Lane l = lane(0,2,true,45,false,new int[0]);
            if (mode == 0) l.complete = false;
            if (mode == 1) l.position = -1;
            if (mode == 2) l.status = 3;
            if (mode == 3) l.angles = new int[]{45};
            if (mode == 4) l.present &= ~2;
            check(C314RichPolicy.simpleLaneDirection(l) == -1, "malformed lane");
        }
        System.out.println("C314 offline policy PASS: " + checks + " assertions; no vehicle/HMI classes loaded");
    }
}
