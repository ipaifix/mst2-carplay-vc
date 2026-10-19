package org.mst2research.guidance;

/**
 * OFFLINE CANDIDATE ONLY. No publisher, snapshot decoder, hooks or boot entry.
 * Java 1.4 / P0468 core-JCL compatible. Not a simulator of the Kombi.
 * A null arrow override or empty lane result means CLEAR previous enrichment,
 * then use the fresh baseline/status owned by the caller. Never keep old data.
 */
public final class C314RichPolicy {
    public static final int OK = 0;
    public static final int CONTEXT = 1;
    public static final int GEOMETRY = 2;
    public static final int UNSUPPORTED = 3;
    public static final int LANE_DATA = 4;
    public static final int MULTI_DIRECTION = 5;

    private C314RichPolicy() {}

    public static final class Context {
        public boolean transportVerified, active, maneuverValid;
        public int routeState, maneuverType, maneuverIndex;
        public long routeGeneration, routeAgeMs;
    }

    public static final class Geometry {
        public int maneuverType, maneuverIndex, present, junctionType, exitAngle;
        public long routeGeneration;
        public boolean complete, coherent;
        public int[] otherAngles;
    }

    public static final class Descriptor {
        public final int main, direction;
        private final int[] sides;
        private Descriptor(int main, int direction, int[] sides) {
            this.main = main;
            this.direction = direction;
            this.sides = copy(sides);
        }
        public int[] sideCodes() { return copy(sides); }
    }

    public static final class ArrowResult {
        public final int reason;
        public final Descriptor override;
        public final boolean clearPreviousEnrichment = true;
        private ArrowResult(int reason, Descriptor override) {
            this.reason = reason;
            this.override = override;
        }
    }

    public static final class Lane {
        public int present, position, status, highlightedAngle;
        public boolean complete;
        public int[] angles;
    }

    public static final class LaneEvent {
        public long routeGeneration, ageMs;
        public int eventIndex, currentEventIndex, linkedEventIndex, maneuverIndex;
        public boolean showing, complete, coherent, currentIndexPresent, linkedIndexPresent;
        public Lane[] lanes;
    }

    public static final class LaneArrow {
        public final int position, direction, guidanceInfo;
        private LaneArrow(int position, int direction, int guidanceInfo) {
            this.position = position;
            this.direction = direction;
            this.guidanceInfo = guidanceInfo;
        }
    }

    public static final class LanesResult {
        public final int reason;
        public final boolean clearPreviousEnrichment = true;
        private final LaneArrow[] values;
        private LanesResult(int reason, LaneArrow[] values) {
            this.reason = reason;
            this.values = values;
        }
        public LaneArrow[] arrows() {
            LaneArrow[] result = new LaneArrow[values.length];
            System.arraycopy(values, 0, result, 0, values.length);
            return result;
        }
    }

    private static int[] copy(int[] values) {
        int[] result = new int[values.length];
        System.arraycopy(values, 0, result, 0, values.length);
        return result;
    }

    private static boolean current(Context c) {
        return c != null && c.transportVerified && c.active && c.maneuverValid &&
            c.routeState == 1 && c.routeGeneration > 0 && c.routeAgeMs >= 0 && c.routeAgeMs <= 10000 &&
            c.maneuverIndex >= 0 && c.maneuverIndex <= 65535 &&
            c.maneuverType >= 0 && c.maneuverType <= 53 && c.maneuverType != 11;
    }

    /** No invented TTL for unchanged prefetched definitions within a generation. */
    private static boolean geometry(Context c, Geometry g) {
        if (!current(c) || g == null || !g.complete || !g.coherent ||
            g.routeGeneration != c.routeGeneration || g.maneuverIndex != c.maneuverIndex ||
            g.maneuverType != c.maneuverType || (g.present & (1 | 4 | 256 | 512 | 1024)) != (1 | 4 | 256 | 512 | 1024) ||
            g.junctionType < 0 || g.junctionType > 1 ||
            g.otherAngles == null || g.otherAngles.length > 16 || g.exitAngle < -180 || g.exitAngle > 180) return false;
        for (int i = 0; i < g.otherAngles.length; ++i) {
            int a = g.otherAngles[i];
            if (a < -180 || a > 180 || a == g.exitAngle) return false;
            for (int j = 0; j < i; ++j) if (g.otherAngles[j] == a) return false;
        }
        return true;
    }

    /** Strict two-field geometry contract. Does NOT choose a rich fork. */
    public static boolean geometryCompatible(Context c, Geometry g) { return geometry(c, g); }

    /**
     * Three exact right-turn templates captured from the GPS on this P0468.
     * No arbitrary angle quantization, no soft-code synthesis, no fork guessing.
     */
    public static ArrowResult arrow(Context c, Geometry g) {
        if (!current(c)) return new ArrowResult(CONTEXT, null);
        if (!geometry(c, g)) return new ArrowResult(GEOMETRY, null);
        if (g.maneuverType != 2 || g.junctionType != 0 || g.exitAngle != 90)
            return new ArrowResult(UNSUPPORTED, null);
        boolean left = false, forward = false;
        for (int i = 0; i < g.otherAngles.length; ++i) {
            int a = g.otherAngles[i];
            if (a == -90) left = true;
            else if (a == 0) forward = true;
            else return new ArrowResult(UNSUPPORTED, null);
        }
        if (!left && !forward) return new ArrowResult(UNSUPPORTED, null);
        // Preserve exact stock ordering: [64], [0], or [64,0].
        int[] sides = left && forward ? new int[] {64, 0} : new int[] {left ? 64 : 0};
        return new ArrowResult(OK, new Descriptor(13, 192, sides));
    }

    /** Existing C3 simple direction sectors; deliberately no U-turn lane glyph. */
    private static int direction(int angle) {
        if (angle < -157 || angle > 157) return -1;
        int abs = angle < 0 ? -angle : angle;
        if (abs <= 22) return 0;
        if (abs <= 67) return angle < 0 ? 32 : 224;
        if (abs <= 112) return angle < 0 ? 64 : 192;
        return angle < 0 ? 96 : 160;
    }

    /** Primary and angles are separate fields; primary need not occur in angles. */
    public static int simpleLaneDirection(Lane lane) {
        if (lane == null || !lane.complete || (lane.present & 3) != 3 ||
            lane.position < 0 || lane.position > 32767 || lane.status < 0 || lane.status > 2 ||
            lane.angles == null || lane.angles.length > 16 ||
            ((lane.present & 4) == 0 && lane.angles.length != 0)) return -1;
        boolean primary = (lane.present & 8) != 0;
        if (lane.status > 0 && !primary) return -1;
        int selected = primary ? direction(lane.highlightedAngle) : -1;
        if (primary && selected < 0) return -1;
        for (int i = 0; i < lane.angles.length; ++i) {
            int candidate = direction(lane.angles[i]);
            if (candidate < 0 || (selected >= 0 && candidate != selected)) return -1;
            selected = candidate;
        }
        return selected;
    }

    /** All-or-nothing event: never show a partial/reordered recommendation. */
    public static LanesResult lanes(Context c, LaneEvent event) {
        LaneArrow[] empty = new LaneArrow[0];
        if (!current(c)) return new LanesResult(CONTEXT, empty);
        if (event == null || !event.complete || !event.coherent || !event.showing ||
            event.routeGeneration != c.routeGeneration || event.maneuverIndex != c.maneuverIndex ||
            event.ageMs < 0 || event.ageMs > 10000 || event.eventIndex < 0 || event.eventIndex > 65535 ||
            !event.currentIndexPresent || event.currentEventIndex != event.eventIndex ||
            (event.linkedIndexPresent && event.linkedEventIndex != event.eventIndex) || event.lanes == null ||
            event.lanes.length == 0 || event.lanes.length > 6) return new LanesResult(LANE_DATA, empty);
        LaneArrow[] result = new LaneArrow[event.lanes.length];
        for (int i = 0; i < result.length; ++i) {
            Lane lane = event.lanes[i];
            int dir = simpleLaneDirection(lane);
            if (dir < 0) return new LanesResult(MULTI_DIRECTION, empty);
            for (int j = 0; j < i; ++j)
                if (result[j].position == lane.position) return new LanesResult(LANE_DATA, empty);
            result[i] = new LaneArrow(lane.position, dir, lane.status);
        }
        for (int i = 1; i < result.length; ++i) {
            LaneArrow value = result[i];
            int j = i;
            while (j > 0 && result[j - 1].position > value.position) {
                result[j] = result[j - 1];
                --j;
            }
            result[j] = value;
        }
        return new LanesResult(OK, result);
    }
}
