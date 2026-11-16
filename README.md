---
generated_by: "OpenAI Codex (GPT-6; exact variant not exposed)"
timestamp: "2026-11-06T11:00:00-06:00"
---
# Lume
Native Pebble Time 2 (Emery) analog watchface. Broad pencil hands, oversized cardinal numerals, eight batons, restrained recessed edges. No simulated glow or third-party branding.

Six palettes: Warm black, Paper, Olive, Ivory, Navy, Oxblood. Custom background, foreground, accent colors. Optional seconds hand (Off by default). Optional date window or ISO/US/EU footer. Optional digital footer supports watch default, 12-hour, 24-hour. Battery uses the watch battery service.

Build: `python3 execution/build.py`; verify: `python3 execution/verify.py`; host: `python3 -m pytest tests/test_policy.py`; settings: `node watchface/settings.test.js`.

The mask generator uses OFL Archivo Black; attribution is in licenses/. Generated masks include only numerals. System Gothic supplies every label and footer.

Parent-controlled native QA: clean/build with `LUME_FIXTURE=1`, install serially, then `python3 execution/capture_native.py --monitor PORT`. Fixture AppMessage keys 11000 hour,11001 minute,11002 second,11004 battery,11005 year,11006 month,11007 day. Production build strips LUME_* variables before clean/build. Do not publish fixture bundles.


## Round-compatible release — 0.1.1

Lume includes native layouts for Pebble Time2 (Emery200×228), original Time Round (Chalk180×180), and Round2 (Gabbro260×260). The rectangular layout and existing settings identities are preserved.

Build: `python3 execution/build.py`, then `python3 execution/verify.py`. Local bundle: `dist/Lume-0.1.1.pbw`. Native round previews: `media/round/`. Repeatable fixture/capture instructions: [round compatibility QA](knowledge/procedural/round_compatibility.md).

This update is [publicly available](https://apps.repebble.com/7730100d622c463c92fd601c) with round screenshots. Public PBW bytes and screenshot pixels verify; physical-watch validation remains separate.
