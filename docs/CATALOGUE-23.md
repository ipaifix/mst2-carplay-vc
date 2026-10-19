# Exact synthetic catalogue: 23 states

`examples/catalogue-23.json` contains the original synthetic inputs transcribed
from the authored catalogue. It is sufficient to recreate the descriptor
sequence using your own publisher; it is not an installable packet capture.

The baseline non-clear case uses active guidance, a valid maneuver, a submitted
distance of 150 m, main symbol 13 and direction 192, unless overridden.
The catalogue used an approximately five-second dwell per state. Timings are
not a protocol requirement; do not replay this automatically on a vehicle.

The important groups are:

- slots 0 and 22: clear the synthetic presentation;
- 1: simple right turn;
- 2..4: selected right with left, forward, or both secondary branches;
- 5..6: four lane positions, recommendations moved right to left;
- 7: remove lane presentation while retaining the main maneuver;
- 8..9: joining/recalculating status experiments, without instruction distance;
- 10..16: study roundabout descriptors; not a generic CarPlay angle mapping;
- 17..21: two/three-branch fork selections, again synthetic study descriptors.

Physical observation summaries from the P0468 trial:

- the three enriched right turns were visible in the central view and right dial;
- both four-lane selections, their replacement and lane-row clearing were visible;
- joining and recalculating produced similar chevrons without the desired texts;
- roundabout glyphs appeared, but not all individual branches were resolved;
  in RA05, four submitted branch codes did not yield four separately qualified
  branches in the available review;
- five fork selections were visible, and final clearing returned the stock view;
- 150 m submitted was displayed as 200 m, an unresolved distance issue.

The original catalogue JSON describes submitted values and observations as
separate fields. Unknown values stay unknown. The recordings are private, so
the observations cannot be independently verified from media in this branch.

Do not feed the roundabout/fork study rows into live RGI mappings merely
because a synthetic shape was visible. The pure policy intentionally admits
fewer cases than this catalogue can request.
