# Lifecycle, epochs, ages and the offline adapter contract

This is the original bridge's research contract, not an OEM ABI. The pure
reference code can be reused without its old HMI publisher.

## Independent identities

Keep at least the producer identity, connection/session generation, route
generation, current maneuver index, maneuver-definition generation, selected
lane-event index and lane-event generation distinct.

A restarted process can reuse a PID. A new phone connection is not by itself
a new route definition. A prefetched definition belongs to a known route
epoch and becomes current only after explicit selection. A sequence number
advancing does not refresh every field carried in the snapshot.

The supervised original design grants an expected producer/session identity
outside the data frame. Reading an ID from the frame and immediately declaring
that same ID authenticated would be circular.

## Suggested original producer rules

Use one serialized worker for the accumulator. Do not modify caches from
concurrent transport callbacks. The original producer design:

- ignores orphan definitions before an explicit route epoch is established;
- preserves admissible prefetch through calculation/recalculation to activation;
- retires affected caches on STOP, source change, invalid/truncated selection,
  incompatible geometry redefinition and justified route-epoch boundaries;
- closes on clock regression or disconnect instead of silently reusing state;
- checks strictly advancing sequences and fails on generation exhaustion;
- keeps another component's frame from advancing this component's clock/state.

That conservative orphan-prefetch rule can contribute to a route-restart
availability problem. It is a policy to investigate, not a complete proven
explanation of the observed defect.

## Geometry and lane proof

For the first rich geometry admission, require index, type, junction type,
exit angle and other-angle list together, untruncated and coherent. A legacy
delta accumulator can still maintain a simple symbol without those deltas
becoming atomic rich geometry. A description-only update does not repair
missing geometry.

If current geometry changes, invalidate the linked lane selection. An unrelated
future maneuver's amendment should not unnecessarily erase the current lane
event. Check optional links only when present; a real index zero is valid.

## Freshness is layered

Historical reference thresholds are bridge policies, not protocol constants:

- route/instruction context: 10,000 ms;
- joining-route / start-route waiting context in the adapter: up to 90,000 ms;
- maneuver distance/progress: 5,000 ms in the earlier publisher;
- lane display permission: 10,000 ms;
- publication-window/reader admission: 2,500 ms in the envelope consumer design.

The lane age is the age of the **received display permission**, not automatically
the age of the cached 0x5204 definition. The definition can be prefetched.
Neither a heartbeat nor a road-name update renews that permission. Add reader
elapsed time to source ages with overflow-safe arithmetic.

On the actual route-reconnect trial, active-route updates were separated by
approximately 16 and 19 seconds while no current maneuver definition was
available. The 10-second instruction threshold coincided with repeated clear
and republish behavior. A correct fix must distinguish route existence,
waiting for a maneuver, and a stale instruction. Extending all timeouts or
retaining an old turn would conceal the problem rather than establish validity.

## C314GuidanceAdapter.Sample

The included adapter accepts contract version 1, expected-owner/frame checks,
producer/session identity, a policy context, geometry, optional lane event and
a verified fresh baseline descriptor. These are API fields, not a wire decoder.

Modes are **0 REFUSED**, **1 STATUS**, **2 BASELINE**, **3 RICH**. Plans always
request replacement of old side and lane layers. An absent rich override does
not mean “keep the previous enriched display”.

The status mapping in this original adapter is:

```text
stopped/inactive -> main 0, guidance inactive
joining route   -> main 2, guidance active, instruction layers off
calculating     -> main 9, guidance active, instruction layers off
recalculating   -> main 10, guidance active, instruction layers off
arrival         -> main 3, guidance active, instruction layers off
localization    -> main 1, guidance active, instruction layers off
active but missing current maneuver -> main 1, instruction layers off
```

These are proposed publication decisions. Joining/recalculating custom text
and stable route restart were not validated by the physical catalogue.

## Original PRGX/v1 evidence envelope

The exported `native/p0468-c314-envelope` codec wraps an original private-bridge
400-byte PRGI/v4 payload in a 456-byte envelope. This format is our research
transport, **not a Volkswagen or Apple format**. All multi-byte values below
are big-endian:

```text
0..3    magic PRGX
4       version 1
5       reserved zero
6..7    total length 456
8..11   expected producer ID
12..15  session generation
16..19  route generation
20..23  selected definition generation
24..27  lane generation
28..31  maneuver field-presence mask
32..35  selection/display gate mask
36..37  junction count before truncation
38..39  current maneuver index
40..41  optional linked lane-event index
42..43  evidence flags
44..51  reserved zero
52..451 embedded original PRGI/v4 bytes
452..455 FNV-1a-32 over bytes 0..451
```

Gate bits are 1 current-maneuver selection, 2 current-lane selection, 4 received
lane-display permission. Evidence bits are 1 atomic geometry, 2 untruncated
angles, 4 complete lane event. The masks have to come from producer evidence,
not be filled with ones by the caller. Integrity is not authentication.

The codec checks the embedded PRGI magic/version/length/checksum, producer ID,
current index and applicable flag/count constraints. It does not decode every
embedded payload field or establish the producer's truth. The old full PRGI/v4
publisher and HMI reader are intentionally not bundled here. Use the exported
Sample API or your own explicit format unless you implement that full contract.

The codec has no allocation, files, hooks or QNX dependencies. Its synthetic
tests exercise only envelope constraints; they do not reconstruct a real route.
