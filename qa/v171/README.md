# Rift Ascent 1.71

Canonical source: `qa/v171/source`. The Furnace Maw receives a generated material/light repaint: broad weathered ash-silver steel surfaces, charcoal cloth and dark joints, with orange concentrated in its cage helm and furnace chest. This separates the same chained prison-engine silhouette from the fiery Act 1 arena at native PSP resolution. The left-facing pose, anchored feet, gameplay, AI and animation timing are preserved.

`art/furnace-maw.png` is the final transparent image, produced with built-in image generation using the original `qa/v170/art/furnace-maw.png` as the edit target. The exact prompt is in `art/prompt.txt`. `prepare_art.py` downsamples the generated pixels and preserves alpha. The other 448 streamed textures retain their exact content, including the boss dialogue illustrations. Their furnace/cage/chains identity remains consistent with this brighter combat lighting.

Build from the repository root with PSPDEV set:

```sh
bash qa/v171/build_release.sh
```

The build explicitly rewrites PARAM.SFO with 1.71 to prevent a stale title surviving an incremental build. It retains the complete five-track soundtrack, home-screen media, hardware-confirmed startup fix, 32 MiB PSP target and fixed 4 MiB texture cache.

Validation includes 826 native animation frames with ASan/UBSan and framebuffer boundary checks, 2,287 identical resident/streamed rendered cases, all 449 streamed textures through cache eviction and stale-handle recovery, unchanged gameplay/source checks, and media validation. LeakSanitizer is disabled because this runtime cannot inspect sibling process threads; address and undefined-behavior checks remain enabled.

The exact packaged EBOOT is playtested in PPSSPP 1.20.4 with native 480x272 screenshots: all three characters against the Maw, boss dialogue handoff, card preview, idle, incoming hit, enemy strike, guard, killing-blow fade/music, save/quit and cold resume. Fixtures establish reproducible starting states; actions use emulated controller input. `evidence/final-tests.txt` records the final binary and completed checks. Physical PSP testing of this patch remains pending.
