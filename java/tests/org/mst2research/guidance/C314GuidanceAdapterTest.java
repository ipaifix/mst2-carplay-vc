package org.mst2research.guidance;

public final class C314GuidanceAdapterTest {
    private static int checks;
    private static void check(boolean value, String message) {
        ++checks;
        if (!value) throw new RuntimeException(message);
    }
    private static C314GuidanceAdapter.Sample sample() {
        C314GuidanceAdapter.Sample s = new C314GuidanceAdapter.Sample();
        s.contractVersion = 1; s.producerId = 123; s.sessionGeneration = 3;
        s.frameValidated = s.ownerVerified = true;
        C314RichPolicy.Context c = s.context = new C314RichPolicy.Context();
        c.transportVerified = c.active = c.maneuverValid = true;
        c.routeState = 1; c.routeGeneration = 17; c.maneuverIndex = 4; c.maneuverType = 2;
        C314RichPolicy.Geometry g = s.geometry = new C314RichPolicy.Geometry();
        g.maneuverIndex = 4; g.maneuverType = 2; g.routeGeneration = 17;
        g.present = 1 | 4 | 256 | 512 | 1024; g.complete = g.coherent = true;
        g.exitAngle = 90; g.otherAngles = new int[] {-90, 0};
        C314GuidanceAdapter.Baseline b = s.baseline = new C314GuidanceAdapter.Baseline();
        b.verified = true; b.routeGeneration = 17; b.maneuverIndex = 4; b.maneuverType = 2;
        b.main = 13; b.direction = 192;
        C314RichPolicy.LaneEvent e = s.laneEvent = new C314RichPolicy.LaneEvent();
        e.routeGeneration = 17; e.maneuverIndex = 4; e.eventIndex = e.currentEventIndex = 6;
        e.complete = e.coherent = e.showing = e.currentIndexPresent = true;
        C314RichPolicy.Lane lane = new C314RichPolicy.Lane();
        lane.position = 0; lane.status = 2; lane.present = 3 | 8;
        lane.highlightedAngle = 90; lane.complete = true; lane.angles = new int[0];
        e.lanes = new C314RichPolicy.Lane[] {lane};
        return s;
    }
    private static void empty(C314GuidanceAdapter.Plan p) {
        check(p.sideUnits().length() == 0 && p.laneArrows().length == 0, "no residual enrichment");
        check(p.replacePreviousSides && p.replacePreviousLanes, "explicit replace on every plan");
    }
    public static void main(String[] args) {
        C314GuidanceAdapter.Plan rich = C314GuidanceAdapter.evaluate(sample());
        check(rich.mode == 3 && rich.main == 13 && rich.direction == 192, "exact rich descriptor");
        check(rich.sideUnits().length() == 2 && rich.sideUnits().charAt(0) == 64 && rich.sideUnits().charAt(1) == 0,
              "raw BAP unit zero must survive");
        check(rich.laneArrows().length == 1 && rich.laneArrows()[0].direction == 192, "separate lane primary");
        C314RichPolicy.LaneArrow[] copy = rich.laneArrows(); copy[0] = null;
        check(rich.laneArrows()[0] != null, "plan lanes immutable");
        for (int i = 0; i < 9; ++i) {
            C314GuidanceAdapter.Sample s = sample();
            if (i == 0) s.contractVersion = 4; // Existing v4 is not this evidence contract.
            if (i == 1) s.frameValidated = false;
            if (i == 2) s.ownerVerified = false;
            if (i == 3) s.producerId = 0;
            if (i == 4) s.sessionGeneration = 0;
            if (i == 5) s.context.transportVerified = false;
            if (i == 6) s.context.routeAgeMs = 10001;
            if (i == 7) s.context.routeGeneration = 0;
            if (i == 8) s.context.routeAgeMs = -1;
            C314GuidanceAdapter.Plan p = C314GuidanceAdapter.evaluate(s);
            check(p.mode == 0 && !p.guidanceActive && !p.instructionLayersAllowed, "invalid contract clear"); empty(p);
        }
        for (int state = 0; state <= 6; ++state) {
            if (state == 1) continue;
            C314GuidanceAdapter.Sample s = sample(); s.context.routeState = state;
            int[] mains = {0, 0, 3, 9, 1, 10, 2};
            C314GuidanceAdapter.Plan p = C314GuidanceAdapter.evaluate(s);
            check(p.main == mains[state] && p.guidanceActive == (state != 0), "route state plan");
            check(!p.instructionLayersAllowed, "status has no maneuver layers"); empty(p);
        }
        C314GuidanceAdapter.Sample s = sample(); s.context.maneuverType = 11; s.context.routeAgeMs = 90000;
        C314GuidanceAdapter.Plan p = C314GuidanceAdapter.evaluate(s);
        check(p.main == 2 && p.guidanceActive, "joining no off-road or false arrival"); empty(p);
        s.context.routeAgeMs = 90001;
        check(!C314GuidanceAdapter.evaluate(s).guidanceActive, "joining expires");
        for (int i = 0; i < 6; ++i) {
            s = sample(); s.geometry = null; s.laneEvent = null;
            if (i == 0) s.baseline.verified = false;
            if (i == 1) s.baseline.routeGeneration++;
            if (i == 2) s.baseline.maneuverIndex++;
            if (i == 3) s.baseline.maneuverType++;
            if (i == 4) s.baseline.main = 255;
            if (i == 5) s.baseline.direction = -1;
            p = C314GuidanceAdapter.evaluate(s);
            check(p.mode == 1 && p.main == 1, "never reuse unrelated baseline"); empty(p);
        }
        s = sample(); s.geometry = null; s.laneEvent = null;
        p = C314GuidanceAdapter.evaluate(s);
        check(p.mode == 2 && p.main == 13 && p.direction == 192, "fresh simple fallback"); empty(p);
        for (int i = 0; i < 3; ++i) {
            s = sample();
            if (i == 0) s.geometry.routeGeneration++;
            if (i == 1) s.geometry.maneuverIndex++;
            if (i == 2) s.geometry.complete = false;
            s.laneEvent.showing = false;
            p = C314GuidanceAdapter.evaluate(s);
            check(p.mode == 2, "bad rich definition preserves simple fallback"); empty(p);
        }
        s = sample(); s.laneEvent.linkedIndexPresent = true; s.laneEvent.linkedEventIndex = 5;
        p = C314GuidanceAdapter.evaluate(s);
        check(p.mode == 3 && p.laneArrows().length == 0, "lane link mismatch does not poison arrow");
        s = sample(); s.laneEvent.lanes[0].present |= 4; s.laneEvent.lanes[0].angles = new int[] {0};
        check(C314GuidanceAdapter.evaluate(s).laneArrows().length == 0, "multi-direction lanes not flattened");
        // Stateful UI emulated only by applying EVERY replacement, never a cache in the adapter.
        int[] sequence = {1, 5, 1, 6, 1, 0, 1};
        for (int i = 0; i < sequence.length; ++i) {
            s = sample(); s.context.routeState = sequence[i];
            p = C314GuidanceAdapter.evaluate(s);
            if (sequence[i] != 1) empty(p);
            else check(p.mode == 3 && p.sideUnits().length() == 2, "new coherent instruction resumes");
        }
        empty(C314GuidanceAdapter.evaluate(null));
        System.out.println("C314GuidanceAdapter: " + checks + " assertions PASS (OFFLINE ONLY)");
    }
}
