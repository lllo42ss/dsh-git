# Contributing

## Branch model

- `main` is always testable and tagged for releases.
- `feature/<topic>` for new behavior.
- `fix/<topic>` for corrections.
- `docs/<topic>` for documentation-only changes.
- Merge with `--no-ff` so the decision history stays visible.

## Commit messages

Use Conventional Commits:

```text
feat(control): add ball-position feedforward
fix(vision): reject stale frames
docs(test): record P-02 results
chore(ci): cache Python packages
```

## Before committing

```bash
python -B -m unittest discover -s tests -v
python -B scripts/check_repo.py
```

If firmware changed, also build locally:

```bash
cd firmware
pio run
```

## Hooks

Enable the checked-in hook once per clone:

```bash
git config core.hooksPath .githooks
```

The hook runs the same Python checks. Firmware builds stay manual because the
ESP32 toolchain is large and board-specific.

## Pull requests

- Keep one logical change per PR.
- Link the affected scoring item or test ID.
- Include bench data for control changes; include a short video for vision changes.
- Never commit credentials, API keys, or personal absolute paths.
