# Native publication contracts and rendering distinctions

These are authored interoperability notes. No OEM serializer, BAP framework,
header, bytecode listing or complete vehicle transport implementation is included.

## Function 23: maneuver descriptor

The observed maneuver item serializes, in order, `mainElement`, `direction`,
`zLevelGuidance` as byte-valued fields, then a raw-content BAP string named
`sidestreets`. The status can contain up to three maneuver items. This is the
observed item contract, **not a complete BAP packet framing specification**.

The P0468 generated type to inspect locally is
`de.vw.mib.bap.mqbab2.generated.navsd.serializer.ManeuverDescriptor_Status$Maneuver_1`.
The DSI-side descriptor name is `org.dsi.ifc.navigation.BapManeuverDescriptor`.

`sidestreets` has a 17-unit capacity in the inspected serializer. Its raw mode
emits the low byte of each supplied unit; it must not pass through UTF-8 text
encoding. In particular, an embedded zero is data, and 0xff is one byte rather
than a multibyte text encoding. The pure Java plan returns the code units.

Observed directional codes include:

```text
0 forward, 32 forward-left, 64 left, 96 back-left,
128 back, 160 back-right, 192 right, 224 forward-right.
```

Intermediate multiples of 16 and special 65/193 occur in target-specific
rendering paths. Their presence is not a generic angle-quantization rule.
The verified policy only emits side templates `[64]`, `[0]`, and `[64,0]` for
the complete right-turn geometry described in NATIVE-GUIDANCE.md.

Useful main-symbol identifiers are 0 no symbol, 1 neutral/no information,
2 direction-to-destination, 3 arrival, 6 off-road, 9 calculating, 10 recalculating,
11 follow-road, 13 turn, 15/16 exit-right/left, 19/20 fork-two/three, 21/22
roundabout families, 34 Michigan turn, 35 double turn, 38 waypoint direction.
These are observed identifiers, not a promise of a visible glyph for every
combination. Direction and cluster view affect the result.

The P0468 stock function maps native source identifiers 32/33 to BAP 15/16.
It also removes side/z-level information for some main-symbol families. A
replacement publisher must deliberately reproduce or bypass the appropriate
validation; copying the source identifier directly is not equivalent.

## Function 24: lane array exchange

On the inspected P0468 stack, lane guidance is an **array** protocol. The
relevant generated type names include `LaneGuidance_ChangedArray`,
`LaneGuidance_GetArray`, `LaneGuidance_StatusArray`, and
`LaneGuidance_LaneGuidance`, under the generated NavSD serializer package.

The top-level flag is `laneGuidanceOnOff`. Item fields used by the experimental
bridge are `laneDirection`, `guidanceInfo`, `laneType`, `laneMarking_left`,
`laneMarking_right`, `laneDescription`, and `laneSidestreets`.

A useful implementation contract, independently of a specific OEM framework:

1. Construct and replace a complete source list from one admitted plan.
2. Publish the source **before** announcing ChangedArray. A synchronous or
   immediate GetArray must see the new list, not the old selection.
3. Answer GetArray using the requested start/range, the actual source size,
   the correct array header and any required consecutive transmission position.
4. Keep array-element positions consistent with the response header. The bridge
   sorts semantic lane positions, then assigns source-array positions 0..N-1.
5. Implement request tracking, acknowledge/error paths and teardown according
   to the actual target framework. A notification alone does not transfer the
   lane items to the cluster.
6. Clearing removes the old list and turns presentation off; do not leave a
   previously recommended pair in the GetArray source.

Our experimental P0468 path uses the stock-named
`FsgArrayMessageBlockingRequestTracker` and `FsgArrayRequestData` helpers. Their
implementations are not distributed. The tested application policy has a
six-lane limit; a request helper's larger window is not permission to invent
additional lanes.

The conservative mapping preserves recommendation status, uses an unspecified
lane type / unsupported description, clears markings, and leaves lane side
branches empty. Those simplifications do not validate rich lane markings or
multidirectional lane glyphs. `C314GuidanceAdapter.Plan` exposes only the
supported semantic values; your publisher remains target-specific.

## Functions 17, 18, 20 and 38

- **17:** route activation/status. Do not equate “no current maneuver” with
  “route stopped”; calculation and joining states need separate handling.
- **18:** distance/progress. The stock path uses a formatter and separate
  distance/unit values. A distance of 150 m displaying as 200 m remains an
  unresolved encoding/formatting/rendering issue, not a general conversion rule.
- **20:** `turnToInfo` and `signPost` are supplementary strings. Inspected
  capacities were 76 and 31. A published string does not guarantee the cluster
  shows it in the current view or prioritizes it over a native symbol label.
- **38:** `InfoStates_Status.states` describes information such as mobile
  navigation. Its numeric namespace is separate from RGI route states.

## MIB widget versus actual cluster

`de.vw.mib.widgets.Guidance` is an MIB-side widget, not the cluster firmware.
It provided useful accepted-combination observations, including fork-two
directions 64/192 and fork-three directions 0/64/192. It uses a four-entry
side-branch rendering limit, while the BAP item allows 17 units. These limits
must not be treated as the same hardware capability.

The widget's waypoint identifier differs from the BAP waypoint identifier
observed at the cluster. Likewise, a widget rejecting or drawing a shape is
not independent proof of the cluster result. The 23-case catalogue below
records the actual submitted descriptors and the narrower physical observations.

No glyph numbers, atlas images or extracted image resources are required to
use these native descriptor interfaces.
