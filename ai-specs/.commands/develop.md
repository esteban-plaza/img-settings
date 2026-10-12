# Command: develop

## Usage

```
/develop @ai-specs/changes/<ticket-id>.md
```

Example: `/develop @ai-specs/changes/IMG-42.md`

## What This Command Does

Executes the implementation plan referenced by the given file, following each step exactly as described. Uses TDD: writes the failing test first, then implements the code to make it pass.

## Instructions for the AI

1. **Read the plan file** completely before writing any code.
2. **Execute steps in order** — never skip or reorder steps.
3. **TDD discipline**:
   - Write the test first.
   - Run `make test` — confirm it fails.
   - Implement the minimum code to make it pass.
   - Run `make test` — confirm it passes.
4. **After each step**:
   - The build must have no new compiler or QML warnings.
   - Commit with a Conventional Commits message referencing the ticket.
5. **Check the UI** if the plan touches `qml/`: launch the app on macOS and with `IMG_SETTINGS_STYLE=windows`.
6. **Update documentation** if the plan requires it:
   - `docs/USER_GUIDE.md`
   - `docs/USER_GUIDE.es.md`
   - `README.md`
7. **Do not push** without explicit user confirmation.
