package org.mst2research.guidance;

/**
 * OFFLINE publication plan, NOT a native snapshot decoder or BAP publisher.
 * Input assertions must eventually come from a verified, versioned producer.
 * No caches, firmware classes, I/O, threads, loader or installation entry.
 */
public final class C314GuidanceAdapter {
    public static final int CONTRACT = 1;
    public static final int REFUSED = 0, STATUS = 1, BASELINE = 2, RICH = 3;
    private C314GuidanceAdapter() {}

    public static final class Sample {
        public int contractVersion, producerId;
        public long sessionGeneration;
        public boolean frameValidated, ownerVerified;
        public C314RichPolicy.Context context;
        public C314RichPolicy.Geometry geometry;
        public C314RichPolicy.LaneEvent laneEvent;
        public Baseline baseline;
    }

    /** Fresh simple-mapping output for this exact current maneuver, not a cache. */
    public static final class Baseline {
        public long routeGeneration;
        public int maneuverIndex, maneuverType, main, direction;
        public boolean verified;
    }

    public static final class Plan {
        public final int mode, main, direction, arrowReason, laneReason;
        public final boolean guidanceActive, instructionLayersAllowed;
        public final boolean replacePreviousSides = true, replacePreviousLanes = true;
        private final String sides;
        private final C314RichPolicy.LaneArrow[] lanes;
        private Plan(int mode, int main, int direction, boolean active, boolean instruction,
                     int arrowReason, int laneReason, String sides, C314RichPolicy.LaneArrow[] lanes) {
            this.mode = mode; this.main = main; this.direction = direction;
            this.guidanceActive = active; this.instructionLayersAllowed = instruction;
            this.arrowReason = arrowReason; this.laneReason = laneReason;
            this.sides = sides; this.lanes = lanes;
        }
        /** Raw UTF-16 code units, including U+0000. Never encode as UTF-8 text. */
        public String sideUnits() { return sides; }
        public C314RichPolicy.LaneArrow[] laneArrows() {
            C314RichPolicy.LaneArrow[] result = new C314RichPolicy.LaneArrow[lanes.length];
            System.arraycopy(lanes, 0, result, 0, lanes.length);
            return result;
        }
    }

    private static Plan status(int mode, int main, boolean active) {
        return new Plan(mode, main, 0, active, false, C314RichPolicy.CONTEXT,
                        C314RichPolicy.LANE_DATA, "", new C314RichPolicy.LaneArrow[0]);
    }

    private static boolean context(Sample s) {
        if (s == null || s.contractVersion != CONTRACT || !s.frameValidated || !s.ownerVerified ||
            s.producerId <= 0 || s.sessionGeneration <= 0 || s.context == null) return false;
        C314RichPolicy.Context c = s.context;
        return c.transportVerified && c.routeGeneration > 0 &&
               c.routeAgeMs >= 0 && c.routeAgeMs <= (c.routeState == 6 ||
                   (c.routeState == 1 && c.maneuverValid && c.maneuverType == 11) ? 90000 : 10000) &&
               c.routeState >= 0 && c.routeState <= 6;
    }

    private static boolean baseline(C314RichPolicy.Context c, Baseline b) {
        return b != null && b.verified && b.routeGeneration == c.routeGeneration &&
               b.maneuverIndex == c.maneuverIndex && b.maneuverType == c.maneuverType &&
               b.main >= 0 && b.main <= 38 && b.direction >= 0 && b.direction <= 255;
    }

    public static Plan evaluate(Sample s) {
        if (!context(s)) return status(REFUSED, 0, false);
        C314RichPolicy.Context c = s.context;
        if (!c.active || c.routeState == 0) return status(STATUS, 0, false);
        // A fixed destination marker is not a heading or an instruction ahead.
        if (c.routeState == 6 || (c.routeState == 1 && c.maneuverValid && c.maneuverType == 11))
            return status(STATUS, 2, true);
        // Preserve route activity while clearing the old turn, bar and lanes.
        // No invented grace period and no promise of Kombi text rendering.
        if (c.routeState == 3) return status(STATUS, 9, true);
        if (c.routeState == 5) return status(STATUS, 10, true);
        if (c.routeState == 2) return status(STATUS, 3, true);
        if (c.routeState == 4) return status(STATUS, 1, true);
        if (!c.maneuverValid || c.maneuverIndex < 0 || c.maneuverIndex > 65535 ||
            c.maneuverType < 0 || c.maneuverType > 53)
            return status(STATUS, 1, true);
        C314RichPolicy.ArrowResult arrow = C314RichPolicy.arrow(c, s.geometry);
        C314RichPolicy.LanesResult lane = C314RichPolicy.lanes(c, s.laneEvent);
        int main, direction, mode;
        String sides = "";
        if (arrow.override != null) {
            main = arrow.override.main; direction = arrow.override.direction; mode = RICH;
            int[] codes = arrow.override.sideCodes();
            if (codes.length > 17) return status(REFUSED, 0, false);
            char[] units = new char[codes.length];
            for (int i = 0; i < codes.length; ++i) {
                if (codes[i] < 0 || codes[i] > 65535) return status(REFUSED, 0, false);
                units[i] = (char)codes[i];
            }
            sides = new String(units);
        } else if (baseline(c, s.baseline)) {
            main = s.baseline.main; direction = s.baseline.direction; mode = BASELINE;
        } else return status(STATUS, 1, true);
        return new Plan(mode, main, direction, true, true, arrow.reason, lane.reason,
                        sides, lane.arrows());
    }
}
