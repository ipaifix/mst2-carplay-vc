# Establishing RGI reception: observations from the P0468 work

This note describes the negotiated capabilities and sequence. It does not
provide a driver patch, copied instruction bytes, memory offsets or credentials.

## Capability advertisement must be complete

In the successful P0468 path, IdentificationInformation included:

- outgoing message IDs **0x5200** (StartRouteGuidanceUpdates) and **0x5203**
  (StopRouteGuidanceUpdates);
- incoming IDs **0x5201**, **0x5202**, **0x5204**;
- a RouteGuidanceDisplay component using the experimental component instance
  identifier **0x0010**.

The identified stock-list callbacks were parameter **6** for outgoing IDs and
parameter **7** for incoming IDs. This is a target-specific implementation
observation, not a universal callback layout or permission to patch another
driver by offset. Retain each target's existing capabilities when adding RGI.

Component instance 0x0010 must match the component information carried in the
RGI frames. It is not a BAP function, firmware identifier, or requirement that
every implementation choose that same unused instance value.

## Sequence and narrow evidence

The stable isolated experiment advertised the capabilities in memory, waited
for identification acceptance, then requested **one** 0x5200 update stream
through the already identified stock send path for that component. The process
survived its bounded supervision window. No BAP output or receive-time file
logger was added to that isolation test.

Earlier reception established that the phone sent real 0x5201 route and 0x5202
maneuver messages when capability/component negotiation was present. That
small initial burst contained no 0x5204. It is evidence for route/maneuver
reception, not a lane-event qualification corpus.

Do not start repeated requests on every received frame, rebuild the entire
capability list from guessed defaults, or combine negotiation changes and
cluster publishing in a first target experiment.

## Callback work and lifecycle

An early combined receive experiment exited before its stability check. Later
isolation retained negotiation/request and removed per-frame file logging.
That narrowed the remaining uncertainty; it did not independently prove the
cause of every process exit.

The design implication is to copy bounded input into an owned queue or buffer,
then parse/persist on a separate serialized worker. Avoid unbounded formatting,
file writes, locks or complex publication in a hot stock receive callback.
Overflow must fail visibly and retire affected evidence rather than silently
merge an incomplete sequence into a valid-looking route.

On teardown, track ownership of the request and the session. Distinguish STOP,
phone disconnect, source change, absent maneuver data and stale display gates.
The current native-guidance restart defect remains open even though transport
admission succeeded in later work.

## Provider limits

Video being available from a navigation app does not establish that the app
provides usable RGI maneuver/lane metadata. The P0468 native-guidance work had
usable route/maneuver data with tested Apple/Google Maps sessions; other app
behavior must be checked per app/version and event type. Do not infer Waze lane
metadata support from the external project's Waze video result.
