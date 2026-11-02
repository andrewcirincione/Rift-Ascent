# Rift Ascent distribution edition: IP risk review

Reviewed 2 November 2026. Target: `v1.8.6-dist2`, branch `distribution`.
This is a research and engineering assessment, not an attorney's clearance
opinion, a finding of infringement, or assurance against a demand or takedown.
It is primarily U.S.-focused; no worldwide clearance is claimed.

**Recommendation: do not describe this project as legally cleared or promote a
broad public launch yet.** The distribution edition has materially differentiated
rules and text, but the public development builds, the title collision below,
and the unverified provenance/overall-expression questions remain unresolved.

## Findings and actions

| Priority | Finding | Action / remaining decision |
| --- | --- | --- |
| Before wider launch | `main` and older public releases still contain the old catalogs. A new branch does not replace them. | Recommend a standalone distribution repository containing only the reviewed snapshot, and an owner/counsel decision on ending public availability of development builds. Preserve private records and provenance; do not destroy history or evidence. No old branch, tag or asset was deleted by this review. |
| Before wider launch | Another developer publicly uses **Rift Ascent** for a game. | Obtain title clearance or select and clear a different title. A name change has not been made implicitly. Priority, territory, actual trademark rights and confusion have not been adjudicated. |
| Implemented | Proxy's surviving dialogue used Regent/Sovereign Blade language and a closely associated royal/celestial identity. | Rewrote the character's entries, boss exchanges, selected events and ending/return dialogue around a constructed witness, erased testimony and the rift's falsified record. The other characters' cross-references were reconciled. |
| Implemented | Inactive choice UI retained Sly rules and a Sly/Inky badge. | Removed those instructions and the badge. Private save-layout identifiers remain where needed; the retired character catalogs and engines remain absent. |
| Implemented | A reward heading and several enemy intent labels matched reference card names. | Reworded those labels. Also replaced the avoidable enemy name Abyss Watcher with Hollow Surveyor. Ordinary functional vocabulary is not treated as proprietary merely because it occurs in another game. |
| Screened, not cleared | Current collectible names and written descriptions. | No exact normalized matches for 514 collectible names, no full-description matches, and no shared eight-word sequence in the screened C string literals against the specified reference snapshot. This is a finite text comparison, not a substantial-similarity test. |
| Counsel review | Overall presentation, narrative similarities, code authorship and permissions. | Review the actual executable, source provenance and proposed storefront/repository presentation. Changing isolated words, numbers or assets does not determine the result. |

## What was actually checked

The canonical source is `qa/v186/source`. The review covered the current card,
relic and potion catalogs; character engines; story/event/ending strings; UI
strings; packaging rules; the GitHub branch/release surface; Mega Crit's current
published policy; selected primary U.S. authorities; and a preliminary public
title search. The owner expressly excluded artwork/music provenance inspection.
Those assets were preserved; no media authorship or licensing clearance is implied.

The comparison source was the English data in the community project
[spire-archive](https://github.com/nkhoit/spire-archive/tree/4e6b0e040ccd00eb04fb1ab7fb0b298c222d2f29/data),
commit `4e6b0e040ccd00eb04fb1ab7fb0b298c222d2f29` (4 October 2026).
It is not a Mega Crit-certified exhaustive catalog, and later changes or missing
localizations/content may not be represented.

| Reference category | Slay the Spire | Slay the Spire 2 |
| --- | ---: | ---: |
| Cards | 360 | 577 |
| Relics | 181 | 296 |
| Potions | 42 | 63 |
| Characters | 4 | 5 |
| Events | 52 | 66 |
| Monsters | 68 | 111 |
| Powers | 146 | 241 |

That is 2,212 reference rows and 1,642 unique normalized names, checked against
394 Rift Ascent card slots, 94 relics and 26 potions. Case and punctuation were
normalized. Names were compared across categories, not just card against card.
The scan covered 3,373 string literals in 36 source files containing strings.
Nested reference descriptions/templates were also used for phrase screening. Complete
competitor catalogs, images and descriptions are not included in the repository
or release archives; evidence contains source identifiers, hashes and findings.

The surviving exact C-literal name candidates are `DRAW` in the pile UI,
`(null)` in the C formatting helper, and `Strike` in an inactive historical tag
helper. These are not collectible names. The latter is tied to retired state
that the distribution engine does not activate. Private identifiers such as
`regent.stars`, legacy enum names and compatibility hooks are not a representation
that those former systems remain playable; renaming structure fields would also
not establish independent authorship. No proprietary Mega Crit source was obtained
or used for this review, and the review cannot certify the origin of every
pre-existing line of code.

The eight-word threshold is an engineering screening choice, **not a legal rule**.
It misses paraphrases, shorter expressive similarities, scene/character structure,
art, animation, arrangement, and functionally equivalent rules expressed differently.
Zero hits cannot establish zero infringement. There was no exhaustive card-by-card
semantic equivalence proof against every version of either commercial game.

Reproduce the offline regression check with
`python3 distribution/tools/review_text.py`. To repeat the wider comparison,
pass `--reference-dir` pointing to lawfully obtained copies of the fourteen
specified JSON files. File hashes and results are recorded in
[`legal-catalog-audit.json`](evidence/legal-catalog-audit.json).

## Why the mechanical and narrative changes matter

The earlier distribution rewrite replaced the inherited catalogs and character
effect engines. Echo is a shared resource capped at nine, generated differently
by each character, with optional release effects. Powers use bounded triggers;
Wither decays by two. Relics and potions use new compound contracts. These are
design changes, not a blanket numerical multiplier applied to copied cards.
Generic damage, defense, drawing, deck building and randomized route decisions
remain genre mechanics; their presence alone does not settle copyright liability.

Mega Crit's own [Regent introduction](https://www.megacrit.com/news/2025-12-11-neowsletter-issue-17/)
describes a celestial ruler using an uncapped persistent Stars payment resource
and forging a signature Sovereign Blade. Rift Ascent's distribution engine has
no such sword-creation/scaling system, and Echo is not that character's extra
card-payment currency. However, the old Proxy text still explicitly invoked
that identity. This pass removes those specific associations and gives Proxy
a distinct narrative purpose. This comparison is our risk-reduction inference,
not a ruling that a particular word or fantasy archetype is exclusively owned.

Warden's burden is complicity in the furnace; Vesper's is the erasure of names;
Proxy's is coerced certification of erased lives. Their shared loop is sustained
by an oath, a name and testimony. The fifth-win revelation, ten-win unlocks and
once-only three-character ending retain their existing triggers. Crowned bosses,
stars in the setting and common words such as silent are not globally removed.

The remaining broad structure includes turn-based deck combat, displayed enemy
intent, a hand along the bottom, player and opponents on opposite sides, branching
room choices, healing/upgrading stops, relics, bosses and repeat ascents. Some of
these are functional or common genre choices, but the combined expressive
presentation still deserves a side-by-side legal assessment. This review does
not declare the UI/trade dress cleared or rely on separate artwork alone.

## Relevant legal framework and policy

Under [17 U.S.C. §102(b)](https://uscode.house.gov/us/usc/119-118/t17/s102/),
copyright does not extend to ideas, systems or methods of operation. The
[Copyright Office's games guidance](https://www.copyright.gov/register/tx-games.html)
distinguishes game methods and titles from protectable literary/pictorial
expression. This supports independent use of a genre or mechanic, not copying
specific dialogue, artwork or sufficiently expressive rule text.

In [Tetris Holding v. Xio, 863 F. Supp. 2d 394 (D.N.J. 2012)](https://www.govinfo.gov/content/pkg/USCOURTS-njd-3_09-cv-06115/pdf/USCOURTS-njd-3_09-cv-06115-0.pdf),
the court granted summary judgment on copyright and trade dress where the
defendant had copied the game's visual expression extensively, despite not
copying its source code or exact image files. The opinion's discussion of the
overall look and arbitrary expressive choices, particularly pp. 27–38, cautions
against treating every visual choice as an unprotected rule. Its specific facts
do not make every similar game unlawful; this is a district-court decision.

In [Spry Fox v. Lolapps, No. C12-147RAJ, order of 18 September 2012](https://upload.wikimedia.org/wikipedia/commons/5/51/Spry_Fox,_LLC_v._Lolapps,_Inc._Order_on_Motion_to_Dismiss_dated_18_September_2012_(Docket_%E2%84%9624).pdf),
the court allowed the copyright claim to survive dismissal based on plausibly
similar expression, while recognizing unprotected ideas/functionality and thin
protection for some rules language. Differences in individual objects did not
automatically dispose of the claim. This was a pleading-stage decision, **not**
a final infringement judgment. The trademark claim was dismissed with leave to
amend. See pp. 7–15. These cases inform a cautious review; neither decides Rift
Ascent's status.

The [Copyright Office's fair-use guidance](https://www.copyright.gov/fair-use/)
explains the four-factor, fact-specific analysis. Free/noncommercial distribution
can matter, but does not automatically qualify. There is no safe percentage of
names, numbers or text that may be retained. This edition is intended to stand
on independent expression, not a claim that charging nothing authorizes copying.

[Mega Crit's content policy](https://www.megacrit.com/content-policy/) expressly
addresses video content, limited merchandise and mods. Its mod permission has
conditions, including no implied official affiliation and restrictions on
monetization. I found no express blanket permission there for a separate
standalone game using protected Slay the Spire content. A PSP homebrew game is
not automatically a mod of Mega Crit's game merely because it is free.

GitHub's published [6 October 2026 DMCA notice](https://github.com/github/dmca/blob/master/2026/10/2026-10-06-slay-the-spire.md)
attributes a request to Mega Crit concerning alleged redistribution of Slay the
Spire 2 proprietary code and assets, including written content, in another
repository and forks. This is evidence of a public enforcement request, not a
court finding, a notice against this project, or proof that Mega Crit will object
to an independently expressed deckbuilder. It makes assumed tolerance an
unsound basis for release planning.

## Title, affiliation and public distribution

The creator's [Rift Ascent announcement](https://dev.to/tizwildin/neo-vectrs-rift-ascent-1ca1)
links a live browser game and [GareBear99/RiftAscent](https://github.com/GareBear99/RiftAscent).
GitHub metadata records that repository's creation on 28 February 2026. This is
an actual title-use collision in games, separate from Mega Crit. A repository
creation date does not establish first commercial use, ownership, geographic
rights, or priority over this project.

The [USPTO recommends comprehensive clearance](https://www.uspto.gov/trademarks/search/comprehensive-clearance-search-similar-trademarks),
including federal/state records and common-law use. This review performed
preliminary public web/repository searching, **not a completed federal/state or
international trademark database clearance**. No conclusion that the title is
unregistered, available or infringing should be drawn. Counsel should examine
the title and any replacement for similar sound, meaning and commercial
impression in related goods/services, not only exact spelling.

The distribution notice disclaims Mega Crit affiliation. That is helpful
context, not a cure for copying or confusing presentation. Public descriptions
should accurately describe this game's own systems and should not call it an
official PSP port, authorized adaptation or legally cleared Slay the Spire clone.
Do not use Mega Crit branding as the product identity. Accurate source citations
in this review are not advertising permission.

`main` was reviewed at `cb1b2e3635f312f50e8269d2186e0a814182d98e`.
Two selected historical catalogs alone contain 89 and 146 exact reference-name
matches respectively (not necessarily distinct across files). See
[`legacy-public-surface.json`](evidence/legacy-public-surface.json).
Public release metadata also still lists development EBOOT/source downloads,
including `v1.8.6-r4`. Merely linking the new branch does not withdraw that
material. The current source ZIP excludes those historical catalogs and binaries;
Git history and older releases are separate public surfaces and remain intact
under the owner's preservation instruction.

## Concrete handoff before wider distribution

1. Give an IP lawyer the exact dist2 binary/source, this report, screenshots of
   the actual flow, the planned distribution copy and the historic catalogs.
   Ask for an expression/trade-dress review against both Slay the Spire games,
   not just a names/stat spreadsheet. Identify intended countries/platforms.
2. Resolve title clearance or choose a different cleared title before investing
   in public branding. Preserve the project's own first-use and creation records.
3. Decide how to stop offering old development builds publicly while preserving
   a private provenance archive. A clean standalone distribution repository
   would simplify the release boundary but would not erase past conduct or
   establish that the new content is independently lawful.
4. Confirm contributors' authorship/permissions for pre-existing code and retain
   component notices. PSPSDK, font and runtime notices are packaged; that does
   not by itself prove all chain-of-title obligations are satisfied. Artwork
   and music remain an owner-supplied assumption, as expressly requested.
5. If permission is desired for identified protected content, obtain a written
   license covering the actual project and distribution. No contact has been
   made with Mega Crit or counsel and no permission has been inferred from
   silence. Do not promise downstream users rights the owner has not established.

A demand letter can be sent even where a defense may ultimately succeed.
These changes remove specific avoidable similarities and make the evidence
reviewable; they do not support a numerical C&D probability or a risk-free label.
Technical validation and balance results are recorded separately in
[`VALIDATION.md`](VALIDATION.md) and [`BALANCE.md`](BALANCE.md).
