# V2 SD Asset Baseline

This file freezes the legacy SD card layout as a migration baseline for the V2
rewrite. It does not define the final manifest-driven category layout yet.

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

## Migration Notes

- The legacy folders remain the reference source for asset inventory only.
- V2 PR1 does not preserve the legacy folder naming as the runtime contract.
- Gameplay-relevant sounds will later be regrouped by category and by
  `ActionType` outcome before manifest-based weighting and no-repeat logic are
  added.
- `06` and `07` are not a stable V2 target layout. They exist only as legacy
  references for placeholder modes and service feedback.
- The computer voice pools in `08` to `11` are candidates for the future
  `BaseTts` category, but that mapping is deferred until the manifest phase.
