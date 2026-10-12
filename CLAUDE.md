# Claude Configuration — img-settings

## Core Development Rules

All rules for this project are defined in the spec files below. Read them at the start of every session before making any change.

See @ai-specs/specs/base-standards.mdc for core principles (TDD, baby steps, English only, incremental commits).

See @ai-specs/specs/qt-standards.mdc for C++/Qt standards: project layout, coding conventions, error handling, testing, branch naming, commit format, CI/CD pipeline, and build system.

See @ai-specs/specs/development_guide.md for build commands, test commands, release workflow, and the Definition of Done checklist.

## Available Commands

| Command | Description |
|---------|-------------|
| `/plan-ticket <id>` | Generate a step-by-step implementation plan saved to `ai-specs/changes/<id>.md` |
| `/develop @<plan.md>` | Execute an implementation plan with TDD (failing test first) |
| `/commit` | Create a well-formed Conventional Commits message |

See @ai-specs/.commands/plan-ticket.md for `/plan-ticket` instructions.
See @ai-specs/.commands/develop.md for `/develop` instructions.
See @ai-specs/.commands/commit.md for `/commit` instructions.

## Agent

See @ai-specs/.agents/qt-developer.md for the Qt developer agent role and workflow.

## Project Quick Reference

```
src/      # C++: engine (exif, watermark, jxl), Processor (QML bridge), main
qml/      # QML UI: Main.qml + MacShell (Liquid Glass) / WindowsShell (Fluent)
shaders/  # Glass refraction shader
assets/   # Icons, Go Regular font
tests/    # Qt Test unit tests (make test)
docs/     # User guides (EN + ES)
build/    # Build output (git-ignored)
```

**Key rule**: the engine (`src/exif.*`, `src/watermark.*`, `src/jxl.*`) must never depend on QML / Qt Quick or `processor.h`.
