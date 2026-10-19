# P0468 symbols and the connected phone-guidance path

These are interface identifiers and authored behavioral observations, not
redistributed OEM classes. Findings were checked against the HMI fingerprints
in [EVIDENCE-AND-LIMITS.md](EVIDENCE-AND-LIMITS.md). No offset is portable to P0480.

## 1. CarPlay mode input

Class: `de.vw.mib.asl.internal.carplay.common.CarPlayModeHandling`.

Relevant methods:

- `getNavigationType(org.dsi.ifc.carplay.AppState[])`;
- `handleModeChanges(org.dsi.ifc.carplay.Resource[], org.dsi.ifc.carplay.AppState[])`.

The inspected `getNavigationType` recognizes the mode parameter named
`appstate_nav_device` first and returns **2**. Otherwise it recognizes
`appstate_nav_hmi` and returns **3**; absent either match it returns **1**.

Those parameter objects are defined in
`de.vw.mib.asl.internal.carplay.common.CarPlayModeParameter`.
The observed three-int constructor tuples are `(2,2,0)` for device navigation
and `(2,1,0)` for HMI navigation. These are **DSI-side object arguments**;
do not reinterpret them as BAP InfoStates or iAP2 RGI route-state numbers.
Verify the target AppState definition and equality behavior before use.

## 2. Shared active-navigation state and service update

Class: `de.vw.mib.asl.internal.carplay.common.CarPlayGlobalProperies`.
The spelling **Properies** is the actual identifier in this firmware.

Relevant methods are `getActiveNavigation`, `setActiveNavigation`, and
`isHmiRouteGuidanceActive`. On a changed active-navigation value, the setter
calls the CarPlay service's `sendRouteGuidanceUpdate` path.

Class: `de.vw.mib.asl.internal.carplay.api.impl.ASLCarplayServicesImpl`.

Its `isCarPlayRouteGuidanceActive()` result is **true exactly when the shared
active-navigation value equals 2** in the inspected implementation.
Do not infer “video is playing”, “phone connected”, or “secondary stream 111
is active” from this boolean; it is the route-guidance service predicate.

## 3. BAP-facing CarPlay service adapter

Class: `de.vw.mib.bap.mqbab2.common.api.carplay.CarPlayASLDataAdapter`.

`isCarPlayRouteGuidanceActive()` delegates to the ASL CarPlay service. The
adapter does not establish a separate video or maneuver-data validity proof.

## 4. Information-state selection

Class: `de.vw.mib.bap.mqbab2.navsd.functions.InfoStates`.

- `getFunctionId()` identifies NavSD function **38**.
- `getCurrentBAPInfoState()` selects **6** when CarPlay, Android Auto **or**
  CarLife route guidance is active.
- Otherwise the selection follows the native-navigation status path.
- The generated NavSD serializer named
  `de.vw.mib.bap.mqbab2.generated.navsd.serializer.InfoStates_Status`
  carries the field named `states`.
- The observed CarPlay listener identifier in this class is **-2147475646**.
  It is a firmware-specific listener identifier, not a BAP function number.

These observations connect the mode handler's device-navigation classification
to an explicit mobile-navigation state sent toward the cluster. They do not
prove the receiving cluster's rendering or whether another publisher also
controls the overlay on P0480.

## 5. Parallel native-navigation and Exbox effects

`CarPlayModeHandling.handleModeChanges` also has paths involving
`de.vw.mib.asl.api.navigation.ASLNavigationServices.stopGuidance` and the
class `de.vw.mib.asl.internal.carplay.common.ExboxGuidanceListenerImpl`.

The Exbox listener's `setSmartphoneGuidanceActive` and
`setSmartphoneGuidanceInactive` forward ACTIVE/INACTIVE to
`de.vw.mib.asl.api.exboxm.guidance.ExboxGuidanceManager.changeSmartphoneGuidanceStateTo`.
Its guidance-state callback can notify an `ASLNavigationServicesListener`.

This establishes **another side-effect path to inspect**. It does not establish
that the Exbox manager draws the overlay or that bypassing it is safe.
Suppressing all of `handleModeChanges` would also suppress unrelated mode,
audio, screen or native-navigation handling. It is not a proposed patch.

## 6. Native guidance boundaries

Classes in `de.vw.mib.bap.mqbab2.navsd.functions` of interest:
`RGStatus`, `DistanceToNextManeuver`, `TurnToInfo`, `ManeuverDescriptor`,
`LaneGuidance` and `InfoStates`.

The shared navigation source is
`de.vw.mib.bap.mqbab2.common.api.navigation.NavigationASLDataAdapter`.
Observed source identifiers include **732** for route/guidance state,
**741** for next-maneuver distance/progress, **744** for turn-to text, and
**751** for maneuver descriptors. `NavigationService.getTurnToInfo()` is
the named text accessor in the inspected path.

Keep the shared source intact while investigating a narrower publisher.
Changing its data globally can affect stock navigation and other consumers.
