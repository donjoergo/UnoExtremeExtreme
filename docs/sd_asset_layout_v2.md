# V2 SD Asset Layout

This file keeps the legacy SD card inventory as a migration baseline and records
the logical PR4 sound layout now used by the V2 firmware. The current runtime
still uses a transitional mapping to legacy folders until the SD card is
reprovisioned to the final V2 layout.

## Legacy Folder Inventory

| Legacy folder | Current purpose | File count | Keep for migration |
|---------------|-----------------|------------|--------------------|
| `01` | Intro / startup | 14 | Yes |
| `02` | Wait | 36 | Yes |
| `03` | Win | 128 | Yes |
| `04` | Lose | 85 | Yes |
| `05` | Extreme lose | 25 | Yes |
| `06` | Ascending mode | 5 | Partial |
| `07` | Settings | 3 | Partial |
| `08` | Win computer | 24 | Review |
| `09` | Lose computer | 13 | Review |
| `10` | Extreme lose computer | 5 | Review |
| `11` | Wait computer | 18 | Review |

## V2 Logical Folder Mapping

These are the target folder IDs defined for the V2 rewrite and reflected in the
PR4 sound catalog design.

| Folder ID | Pool / Folder Name | Intended content |
|-----------|--------------------|------------------|
| `01` | `Startup` | Startup-ready clips played once after boot when the case is closed |
| `02` | `CaseEvents` | Case-open, case-close, and open-too-long feedback clips |
| `03` | `Wait` | Delay and fake-wait clips |
| `04` | `Safe_BaseNormal` | Classic gameplay clips for safe outcomes |
| `05` | `Lose_BaseNormal` | Classic gameplay clips for lose outcomes |
| `06` | `Safe_BaseFunny` | Funny gameplay clips for safe outcomes |
| `07` | `Lose_BaseFunny` | Funny gameplay clips for lose outcomes |
| `08` | `Safe_BaseTts` | Spoken / robot gameplay clips for safe outcomes |
| `09` | `Lose_BaseTts` | Spoken / robot gameplay clips for lose outcomes |
| `10` | `Safe_Drinking` | Drinking prompts for safe outcomes |
| `11` | `Lose_Drinking` | Drinking prompts for lose outcomes |
| `12` | `Safe_Bdsm` | Dominant / rude prompts for safe outcomes |
| `13` | `Lose_Bdsm` | Dominant / rude prompts for lose outcomes |
| `14` | `Safe_Fetish` | Fetish prompts for safe outcomes |
| `15` | `Lose_Fetish` | Fetish prompts for lose outcomes |
| `16` | `SpamReactions` | Annoyed responses to repeated presses |
| `17` | `CantinaShort` | Short Cantina / blessing style clips |
| `18` | `CantinaLong` | Long protected Cantina mix |

## PR4 Transitional Runtime Mapping

The PR4 firmware uses a compiled-in manifest with the current legacy assets.
This keeps category selection and weighting in place before the SD card is
physically rearranged into the final V2 folder layout.

| Logical pool / category | Current runtime folder | Legacy source | Notes |
|-------------------------|------------------------|---------------|-------|
| `Startup` | `01` | Intro / startup | Direct carry-over |
| `CaseEvents` | `02` | Wait | Temporary placeholder until dedicated case-event clips exist |
| `Safe_BaseFunny` | `03` | Win | Transitional funny-safe pool |
| `Lose_BaseFunny` | `04` | Lose | Transitional funny-lose pool |
| `Safe_BaseTts` | `08` | Win computer | Transitional TTS-safe pool |
| `Lose_BaseTts` | `09` | Lose computer | Transitional TTS-lose pool |
| `SpamReactions` | `06` | Ascending mode | Temporary placeholder pool |

## Migration Notes

- The legacy folders remain the reference source for asset inventory only.
- The PR4 firmware now resolves sounds through a manifest-driven category model
  instead of the old hardcoded folder picks in the app layer.
- Gameplay sounds are selected by explicit `ActionType::Safe` / `ActionType::Lose`
  eligibility before category weighting is applied.
- `BaseNormal` remains part of the V2 type system and config model, but the
  transitional PR4 catalog falls back to `BaseFunny` and `BaseTts` because no
  dedicated normal pool has been provisioned yet.
- `06` and `07` are not a stable V2 target layout. They exist only as legacy
  references for placeholder modes and service feedback.
- `08` and `09` are now used as the first `BaseTts` gameplay pools. `10` and
  `11` remain review candidates for later wait / follow-up use.
