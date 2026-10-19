# Phone-navigation overlay: a concrete P0468 lead

## The two guidance problems are different

The reviewed P0480 alt-screen project's immediate blocker is a cluster message
covering its video while navigation runs on the phone. Native maneuver arrows
and lane recommendations are another interface and another milestone.

These P0468 findings may help locate the first problem. They do not establish
which component ultimately draws or prioritizes the P0480 message.

## Static finding on the exact P0468 HMI

The [connected symbol map](P0468-SYMBOL-MAP.md) now includes the mode handler,
shared active-navigation state, CarPlay service adapter and Exbox side path.
On this target, `CarPlayModeHandling.getNavigationType` classifies device
navigation as 2; the ASL CarPlay service reports guidance active exactly when
the shared active-navigation value is 2. The BAP-facing service adapter delegates
to that predicate, and InfoStates selects the mobile-navigation information state.

The inspected MQB/AB2 navigation implementation exposes:

- function **38**, implemented by the class named `InfoStates`;
- a mobile-navigation information state with numeric value **6**;
- `getCurrentBAPInfoState`, which selects that value if the CarPlay,
  Android Auto **or** CarLife service reports route guidance active;
- the CarPlay predicate named `isCarPlayRouteGuidanceActive`.

When all three mobile predicates are false, the implementation falls through
to native-navigation status handling. Therefore, simply forcing the global
CarPlay predicate false is not a demonstrated, isolated fix: that service can
have other consumers, and other mobile services can still select state 6.

This behavior was rechecked during preparation of this publication against
the privately held P0468 HMI analysis input. No class, bytecode listing,
decompiled method or patch bytes are redistributed.

**Do not confuse numeric namespaces.** CarPlay RGI route state 6 means joining
the route in our tested input model. BAP InfoStates value 6 describes mobile
navigation activity. Copying the number between these interfaces is wrong.

## Why this is a lead, not a fix

- Function 38 provides an explicit mobile-navigation state to the cluster.
- The receiving cluster's rendering and priority policy have not been reversed.
- The P0480 project's `CarPlayModeHandling` investigation has a concrete P0468
  counterpart upstream of the CarPlay service predicate. The P0480 counterpart
  and its actual rendering effects still need to be checked on that target.
- No experiment in this research has shown that suppressing or substituting
  InfoStates removes the overlay while preserving every stock behavior.
- P0468 observations do not establish the same code or ABI on P0480.

## Suggested investigation sequence

1. Locate function 38 and its mobile-service predicates in a legitimately held
   copy of the target firmware. Compare behavior, not binary addresses.
2. Correlate the outgoing information state with the appearance/disappearance
   of the message on the actual cluster. Record transitions as well as a steady
   active route; keep video presentation and native BAP guidance distinguishable.
3. Map the callers/consumers of `CarPlayModeHandling` and the mobile predicates.
   Establish whether there is one causal path or multiple independent publishers.
4. Only then design a bounded, explicitly armed intervention at the narrowest
   proven publication boundary. Preserve native navigation and stock behavior
   whenever the experiment is absent, expired, disconnected or uncertain.
5. Qualify route stop, phone disconnect/reconnect, native-map return and reboot
   recovery. A removed message alone is not sufficient acceptance.

No write script, installer or patch is supplied here. In particular, there is
no recommendation to disable route guidance globally, kill a stock process,
or install a P0468 class on P0480.

## Related public source

The comparison target is the
[P0480 roadmap at the reviewed revision](https://github.com/1mikew/mst2-altscreen-carplay/blob/be141a330966adda05515ae03b2423a4b067f179/ROADMAP.md).
Its reported video result belongs to that author and target; it is not a
vehicle result of this repository.
