> **HARDWARE SAFETY / RELEASE STATUS — 2026-10-22**
>
> **NOT VERIFIED ON REAL PSP; DO NOT INSTALL THE EXPERIMENTAL EBOOT BUILDS.** A PSP-3000 running system software 6.60 and ARK-5 has repeatedly frozen on launch (black screen; HOME unresponsive; hard reset required), including with the tiny test. The 30-second auto-exit in that test only runs if its event loop executes, so it is *not* a hardware watchdog. Cave Story and other homebrew work normally on the same PSP. Compiling with PSPDEV and displaying a title screen in PPSSPP did not establish real-hardware compatibility. Repackaging as single-segment legacy ELF and compiling a firmware-6.60 PRX did not resolve the problem. Do not request further on-device testing without an evidence-based revision and explicit risk acknowledgement.

# Rift Ascent — PSP build repository

This repository will build **Rift Ascent** into `EBOOT.PBP` with the official PSPDEV toolchain.

## Source archive upload

Upload the previously prepared `Rift_Ascent_GitHub_Ready.zip` file to the **repository root** using GitHub's **Add file → Upload files → Commit changes**. Do not unpack the archive; the GitHub Actions workflow unpacks it in its isolated build environment.

On a successful build, download the **Rift-Ascent-EBOOT** artifact from the latest successful workflow run under **Actions**.

**Current status:** Awaiting the project source ZIP. Build and PSP/PPSSPP boot verification are not complete.
