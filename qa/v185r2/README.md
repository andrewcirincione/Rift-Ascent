# 1.8.5 PSP startup regression

The supplied physical PSP boot log stopped after `main start result: 0`.
The old `worker entered` checkpoint was after `ascent_open`, concealing the
failing save-loader path. The prior user-mode imports, module-info locator,
streamed-art layout and next-vsync framebuffer fixes remain intact.

With the pinned PSPDEV v20261001 toolchain, the unmodified source rebuilt to
the exact public 1.8.5 SHA-256:
`dd0fb61a40f496e7328bf489399c100c76cedad056ea66b2d775480c73198ce6`.
Compiler stack reports and the debugger both show 201,432 bytes consumed
while scanning nonexistent slots, exceeding the allocated 196,608 bytes.
Old stable migration adds another 28,520 bytes. PPSSPP permits this access
outside the individual thread's allocation; its lack of a memory exception
was insufficient validation for hardware.

The fix separates `save_and_quit` from the permanent `run_game` frame and
prevents beta-import buffers from being inlined into `ascent_open`.
No heap or larger thread allocation is needed. Gameplay code is unchanged.
Startup logging and HOME registration now precede save initialization.

Build: `PSPDEV=/path/to/pspdev bash qa/v185/build.sh`.
The build emits `.su` frame reports and runs `check_stack.py`, auditing the
startup/save call paths with an 8 KiB allowance for small callees and another
32 KiB margin. This is a targeted stack audit, not a whole-program proof.
`python3 qa/v185r2/check_stack.py --baseline` must reject the original frames.
Run `bash qa/v185/test.sh` for the native sanitizer-backed regressions.

The exact packaged EBOOT passed `qa/v185/qa_ppsspp.py` with
`RIFT_INSTANCE=185r2 RIFT_PREFIX=r2- RIFT_DIRECT=1 RIFT_FAST_PRESENTATION=1`
and `RIFT_TOOLCHAIN` pointing to PSPDEV, extracted official PPSSPP 1.20.4,
and Xvfb/xdotool. Repeated transitions are accelerated only by the documented
test driver option. Combat and save state use the actual release executable.

Cold-boot regression:

```sh
mkdir -p qa/v185r2/evidence/boot-fixtures
gcc -O2 qa/v185r2/boot_fixtures.c -o /tmp/rift185r2-fixtures
/tmp/rift185r2-fixtures qa/v185r2/evidence/boot-fixtures
# Compile the original v1.8.5 source separately and set RIFT_BASELINE_ELF.
python3 qa/v185r2/boot_probe.py --baseline
for scenario in fresh stable185 stable184 beta184 legacy184 corrupt; do
  python3 qa/v185r2/boot_probe.py --scenario "$scenario"
done
```

The probe resets the exact EBOOT at entry and observes registers/breakpoints;
it does not patch game state. It verifies the 192 KiB thread allocation,
stack use at save reads/copies, title frame 60, imported slot presence, audio,
art errors, and preservation of original beta/legacy files. Fixture output
directories must be empty before regeneration. The first probe-driver attempt
used unprefixed decimal debugger expressions, which PPSSPP interprets as hex;
that diagnostic timeout is retained separately. The corrected driver uses
explicit hexadecimal addresses throughout.
The probe also waits for the persisted frame-60 checkpoint, rather than a
presentation counter that can reach its threshold before that log write.
The first successful matrix is retained. Additional measured migration boots
restore fixtures after the initial emulator boot and use a converter-prologue
breakpoint, avoiding PPSSPP's optimized memcpy shortcut; these capture the
actual deepest old-save migration frames as well.

Source remains `qa/v185/source` with synchronized build copies. Release tag
`v1.8.5-r2` preserves the original release and in-game version. Hardware
confirmation on the owner's PSP-3000 / 6.60 / ARK-5 remains outstanding.
