# Native arrows and lane guidance on P0468

## Separate data and rendering paths

The tested architecture consumes iAP2 RGI messages and publishes native
navigation descriptors through BAP NavSD. The cluster draws its own symbols.
It does not send a bitmap, video stream or copied OEM graphical resource.

The useful message families are route updates `0x5201`, maneuver definitions
`0x5202`, and lane events `0x5204`. A prefetched definition is not automatically
the current maneuver, and a lane event is not current merely because it was
received recently.

Observed NavSD function boundaries on the inspected P0468 target:

- **17 RGStatus:** route-guidance activation/status.
- **18 DistanceToNextManeuver:** distance and approach progress.
- **20 TurnToInfo:** supplementary road/turn text.
- **23 ManeuverDescriptor:** main symbol, direction and branch descriptors.
- **24 LaneGuidance:** lane presentation and recommendations.
- **38 InfoStates:** information state, including mobile navigation.

These identifiers describe interoperability findings, not redistributed
serializers or a complete BAP specification.

## Rich arrow findings

The combination of main symbol, direction and side-branch descriptors matters.
In the tested catalogue:

- `main=13, direction=192` gave a right turn.
- Adding side codes `[64]`, `[0]`, or `[64,0]` gave a left branch, a forward
  branch, or both, while retaining the selected right turn.
- Two-branch forks used `main=19`; three-branch forks used `main=20`, with
  selected directions `64/0/192` as applicable.
- A neutral joining-route chevron with `main=2` did not provide a reliable
  custom joining-route text. `main=6` had previously caused an unwanted
  off-road label; it is not a suitable generic joining-route substitution.

The side-branch field is an opaque byte-oriented descriptor, not UTF-8 road
text. The receiving serializer and actual cluster must be verified separately.
The MIB's own guidance widget is not proof of the cluster's rendering policy.
No glyph atlas, resource table or OEM string resource is supplied.

The included pure policy intentionally recognizes only three complete,
qualified right-turn templates. It does not invent a general conversion from
CarPlay angles to every native maneuver or roundabout form.

## Lane contract and a useful correction

A recommended lane's primary direction is independent of its optional list
of other angles. Requiring that primary angle to be repeated in the list
incorrectly discards valid simple-lane cases.

The reference policy accepts primary-only simple direction information when
its presence and status are valid. It still refuses a lane whose explicit
angles describe conflicting direction sectors; it never silently drops a
branch to make a recommendation look valid.

Admission also requires:

- active/current route and maneuver, with verified transport;
- matching route generation and maneuver identity;
- a complete lane event selected by the current lane-event index;
- optional maneuver-to-lane link checked if actually present;
- fresh lane visibility gate, with its own age;
- unique lane positions, sorted by position for presentation;
- one complete event or a cleared result, never a partial recommendation.

The reference policy limits events to six lanes because that was the bridge's
tested contract. It is not a claim about a universal protocol limit. The input
parser has different bounded storage limits; storage capacity is not admission.

## What was visually observed

A synthetic catalogue on one P0468 vehicle showed four lanes with the two
right lanes recommended (`0/0/2/2`), replacement by the two left lanes
(`2/2/0/0`), then clearing of the lane row while retaining the main maneuver.
This was observed in both the central guidance view and the right dial.

Rich intersections, two/three-branch forks and final clearing were also
observed. This validates those catalogue cases on that cluster, not an
arbitrary incoming CarPlay lane event or every dynamic transition.

## Important limits

Joining/recalculating text remained absent or indistinguishable. Some
roundabout branches were not individually resolved. A submitted distance of
150 m was displayed as 200 m; this remains a chain/encoding issue to investigate,
not a proved cluster-rounding rule.

Live route restart/recalculation still showed a recovery defect. Consequently,
the experimental integration is not suitable for permanent activation.

The [offline policy](OFFLINE-CODE.md) is provided without the proprietary HMI
publisher. Setting `complete`, `coherent` or `transportVerified` to true by
default would bypass its contract, not implement an integration.
