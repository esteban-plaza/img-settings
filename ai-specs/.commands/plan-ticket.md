# Command: plan-ticket

## Usage

```
/plan-ticket <ticket-id>
```

Example: `/plan-ticket IMG-42`

## What This Command Does

Given a ticket ID and its description, this command generates a detailed, step-by-step implementation plan for a change to img-settings. The plan is saved to `ai-specs/changes/<ticket-id>.md`.

## Instructions for the AI

1. **Read the ticket**: Ask the user for the ticket description if not already provided.
2. **Read the relevant source files**: Use the architecture in `qt-standards.mdc` to identify which files are affected.
3. **Identify the scope**:
   - Which engine functions (`src/exif.*`, `src/watermark.*`, `src/jxl.*`) need to change?
   - Does `Processor` (`src/processor.*`) need a new property, signal or invokable?
   - Which QML files change, and does the change apply to both the macOS and the Windows shell?
   - Are new tests required?
4. **Generate the plan** with the following sections:

---

### Plan Template

```markdown
# Implementation Plan: <ticket-id> — <short title>

## Overview
<2-3 sentence description of the change and why it is needed>

**Key principles applied:**
- TDD: failing test before implementation
- Baby steps: one commit per step
- Engine isolation: core logic stays in the engine, no QML / Qt Quick dependencies

## Architecture Context

### Files to Modify
- `src/watermark.cpp` — <reason>
- `src/processor.cpp` — <reason>
- `qml/MacShell.qml` / `qml/WindowsShell.qml` — <reason>

### Tests to Add
- `tests/tst_engine.cpp` — test cases for new or changed engine functions

## Implementation Steps

### Step 0: Create Feature Branch
git checkout main && git pull origin main
git checkout -b feature/<ticket-id>-<short-description>

---

### Step 1: Write Failing Tests
File: `tests/tst_engine.cpp`
Action: Add data-driven Qt tests (`_data()` + `QFETCH`) for the new/changed function.
Tests must fail before the implementation exists.

---

### Step N: Implement <function name>
File: `src/watermark.cpp` (+ `src/watermark.h`)
Action: <detailed description>
Expected function signature: ...
Implementation notes: ...

---

### Step N+1: Update Processor / QML (if needed)
...

---

### Step N+2: Verify
make test
open build/img-settings.app                                   # macOS look
IMG_SETTINGS_STYLE=windows build/img-settings.app/Contents/MacOS/img-settings  # Windows look

---

### Step N+3: Commit
git add <files>
git commit -m "feat(engine): <subject>"

## Definition of Done
- [ ] Tests written and passing
- [ ] No new compiler or QML warnings
- [ ] Public engine functions have a contract comment
- [ ] UI checked in both styles (if QML changed)
- [ ] USER_GUIDE updated if user-facing behaviour changed
- [ ] Commit follows Conventional Commits format
```

---

5. **Save the plan** to `ai-specs/changes/<ticket-id>.md`.
6. **Do not implement** — the plan is reviewed before execution.
