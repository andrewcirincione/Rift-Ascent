# Rift Ascent GitHub builds

The maintained source now lives directly in `GAME/src`, `portable_build` and the
versioned `qa` directories. See [README.md](README.md) for the latest downloads.

For 1.4.0, use `qa/v140/build_release.sh` with a complete released 1.3.11 EBOOT
as the soundtrack baseline. The source ZIP contains generated artwork and the
converted home-screen audio. Original full-length WAV recordings are not needed
when preserving the music from a complete baseline EBOOT.

Publishing is handled by `.github/workflows/publish-release.yml`, using the exact
tested binary and checksum in `release/current.json`. It creates permanent
GitHub Release downloads rather than relying only on expiring Actions artifacts.
