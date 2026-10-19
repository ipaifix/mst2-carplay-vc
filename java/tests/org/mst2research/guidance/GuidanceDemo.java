package org.mst2research.guidance;

/** Synthetic integration example. These proof flags are NOT transport evidence. */
public final class GuidanceDemo {
    private GuidanceDemo() {}
    private static C314GuidanceAdapter.Sample sample() {
        C314GuidanceAdapter.Sample s = new C314GuidanceAdapter.Sample();
        s.contractVersion=1; s.producerId=123; s.sessionGeneration=1;
        s.frameValidated=s.ownerVerified=true;
        C314RichPolicy.Context c=s.context=new C314RichPolicy.Context();
        c.transportVerified=c.active=c.maneuverValid=true;
        c.routeState=1; c.routeGeneration=1; c.routeAgeMs=0;
        c.maneuverType=2; c.maneuverIndex=4;
        C314RichPolicy.Geometry g=s.geometry=new C314RichPolicy.Geometry();
        g.routeGeneration=1;g.maneuverIndex=4;g.maneuverType=2;
        g.present=1|4|256|512|1024;g.junctionType=0;g.exitAngle=90;
        g.otherAngles=new int[]{-90,0};g.complete=g.coherent=true;
        C314GuidanceAdapter.Baseline b=s.baseline=new C314GuidanceAdapter.Baseline();
        b.routeGeneration=1;b.maneuverIndex=4;b.maneuverType=2;
        b.main=13;b.direction=192;b.verified=true;
        C314RichPolicy.LaneEvent e=s.laneEvent=new C314RichPolicy.LaneEvent();
        e.routeGeneration=1;e.maneuverIndex=4;e.eventIndex=e.currentEventIndex=7;
        e.currentIndexPresent=e.showing=e.complete=e.coherent=true;e.ageMs=0;
        e.lanes=new C314RichPolicy.Lane[4];
        for(int i=0;i<4;++i){
            C314RichPolicy.Lane lane=e.lanes[i]=new C314RichPolicy.Lane();
            lane.present=1|2|8;lane.position=i;lane.status=i>=2?2:0;
            lane.highlightedAngle=90;lane.angles=new int[0];lane.complete=true;
        }
        return s;
    }
    private static String json(String name,C314GuidanceAdapter.Plan p) {
        StringBuffer b=new StringBuffer("{\"scenario\":\"");b.append(name);
        b.append("\",\"synthetic_only\":true,\"mode\":").append(p.mode);
        b.append(",\"guidance_active\":").append(p.guidanceActive);
        b.append(",\"instruction_layers\":").append(p.instructionLayersAllowed);
        b.append(",\"replace_sides\":").append(p.replacePreviousSides);
        b.append(",\"replace_lanes\":").append(p.replacePreviousLanes);
        b.append(",\"main\":").append(p.main).append(",\"direction\":").append(p.direction);
        b.append(",\"side_codes\":[");
        for(int i=0;i<p.sideUnits().length();++i){if(i>0)b.append(',');b.append((int)p.sideUnits().charAt(i));}
        b.append("],\"lanes\":[");C314RichPolicy.LaneArrow[] lanes=p.laneArrows();
        for(int i=0;i<lanes.length;++i){
            if(i>0)b.append(',');b.append("{\"position\":").append(lanes[i].position);
            b.append(",\"direction\":").append(lanes[i].direction);
            b.append(",\"recommendation\":").append(lanes[i].guidanceInfo).append('}');
        }
        return b.append("]}").toString();
    }
    public static void main(String[] args) {
        C314GuidanceAdapter.Sample s=sample();
        System.out.println(json("right_turn_two_recommended_lanes",C314GuidanceAdapter.evaluate(s)));
        s.context.routeState=5;
        System.out.println(json("recalculation_clears_instruction",C314GuidanceAdapter.evaluate(s)));
        s.context.routeState=0;
        System.out.println(json("stop_clears_route",C314GuidanceAdapter.evaluate(s)));
        s=sample();s.frameValidated=false;
        System.out.println(json("unverified_frame_refused",C314GuidanceAdapter.evaluate(s)));
    }
}
