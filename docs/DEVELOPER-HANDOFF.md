# Developer handoff: continue the MST2 work from here

This dossier is meant to support an implementation, not just announce results.
The original code is included; target-specific OEM implementations are not.

## If your current blocker is the phone-navigation overlay

Start with [the exact P0468 symbol map](P0468-SYMBOL-MAP.md), then
[the phone-overlay investigation](PHONE-NAVIGATION-OVERLAY.md).

The strongest new static lead is a connected chain, not a guessed class name:

```text
CarPlayModeHandling.handleModeChanges
  -> getNavigationType: device navigation / HMI navigation / absent
  -> CarPlayGlobalProperies.setActiveNavigation
  -> ASLCarplayServicesImpl route-guidance service
  -> CarPlayASLDataAdapter.isCarPlayRouteGuidanceActive
  -> NavSD InfoStates.getCurrentBAPInfoState
  -> function 38, states = 6 when mobile navigation is active
```

The mode handler also interacts with native route guidance and the Exbox
smartphone-guidance manager. Treat those as additional consumers/side effects
to verify before suppressing a notification or changing a shared state.

For your own target, reproduce the static chain and correlate function 38 with
the overlay before changing code. P0468 has no vehicle-tested overlay fix here.

## If you are adding native arrows and lanes

1. Review [RGI negotiation](RGI-SESSION-SETUP.md) and
   [framing and field contracts](RGI-WIRE-CONTRACT.md). Preserve presence,
   counts before truncation, current indices and the difference between a
   definition and a selected instruction.
2. Read [native NavSD publication contracts](BAP-NAVSD-CONTRACTS.md). Function 24
   is an array exchange, including a later GetArray response, not merely a
   one-shot notification containing a private list.
3. Use the exported `C314RichPolicy` and `C314GuidanceAdapter` as offline
   reference code. Their output is a complete replacement plan; provide your
   own verified producer and publisher.
4. Read [lifecycle and evidence](LIFECYCLE-AND-EVIDENCE.md) before connecting
   producer booleans to the policy. The sample contract is not a stock ABI.
5. Reproduce [the exact synthetic catalogue](CATALOGUE-23.md) on your own target
   in a separately admitted experiment. Study shapes are not live mappings.

## What you can run now

`python3 tools/run_tests.py` compiles only original C and Java with host tools.
It exercises malformed TLVs, complete/partial fields, lane recommendation
selection, generation/freshness checks, replacement/clear plans, and the
original research envelope. The demo prints synthetic JSON plans.

All input frames in the exported C tests are built in source. The catalogue
JSON was transcribed from authored synthetic test parameters, not exported
from a real route or a device log. No OEM class is needed to run the tests.

Use `python3 tools/run_tests.py --examples` to display the three generated RGI
frames, their decoded fields and the four Java JSON plans. The C framing and
Java planning demos are separate runnable reference layers, not a completed
transport-to-cluster integration.

## What is deliberately still your target-specific work

- Verify the same classes, values and behavior on your firmware and cluster.
- Establish a legal local firmware input and the real BAP transport/framework.
- Implement any stock adapter, serializer binding or vehicle publisher locally.
- Provide ownership, sequencing, activation/recovery and independent cleanup.
- Qualify actual rendering, disconnect/reconnect, route restart and stock return.

These missing pieces are explicit boundaries. The supplied policy is useful
without pretending that the target-specific integration is already available.
