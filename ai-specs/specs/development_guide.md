# Development Guide — img-settings

## Prerequisites

| Requirement | Version | Notes |
|-------------|---------|-------|
| Qt | 6.9+ (CI uses 6.11.2) | Needs Qt Quick, Quick Controls 2 and **Qt Shader Tools** |
| CMake | 3.21+ | |
| C++ compiler | C++17 | Xcode Command Line Tools on macOS, MSVC 2022 on Windows |
| libjxl + pkg-config | 0.12 | Optional; without it `.jxl` input is disabled |

On macOS with Homebrew:

```bash
brew install qt cmake jpeg-xl pkgconf
```

On Windows, install Qt with the Qt Online Installer (MSVC 2022 64-bit + Qt Shader Tools) and, for JPEG XL, `vcpkg install libjxl pkgconf --triplet x64-windows-static-md` (see `.github/workflows/build.yml` for the exact CMake flags).

---

## Getting Started

```bash
# 1. Clone the repository
git clone https://github.com/esteban-plaza/img-settings.git
cd img-settings

# 2. Build the app and the unit tests
make

# 3. Launch it (macOS)
make run
```

If CMake cannot find Qt, pass its location: `make CMAKE_ARGS="-DCMAKE_PREFIX_PATH=/path/to/Qt/6.11.2/macos"`.

---

## Build Targets

```bash
make          # Configure (Release) + build app and tests into build/
make test     # Build and run the unit tests (CTest)
make run      # Build and open build/img-settings.app (macOS)
make clean    # Remove build/ and dist/
```

The macOS build produces `build/img-settings.app`; on Windows, `build/Release/img-settings.exe`.

### Previewing the Windows look on macOS

```bash
IMG_SETTINGS_STYLE=windows build/img-settings.app/Contents/MacOS/img-settings
```

### Packaging

Release packaging (`macdeployqt` / `windeployqt`, `.dmg` / `.zip`) is done by the CI. To reproduce it locally, follow the steps in `.github/workflows/build.yml`.

---

## Running Tests

```bash
make test

# Or run the test binary directly for Qt Test output
QT_QPA_PLATFORM=offscreen build/tests/tst_engine

# A single test function, or one data row
QT_QPA_PLATFORM=offscreen build/tests/tst_engine readExif_jpegWithExif
QT_QPA_PLATFORM=offscreen build/tests/tst_engine "applyOrientation:6 rotate 90 cw"
```

Tests live in `tests/tst_engine.cpp` and cover the engine: supported formats, output paths, file collection, resize, orientation, EXIF parsing, watermark drawing, decoding (including ARW previews), the full `processFile` pipeline and JPEG XL (when built with libjxl).

---

## Project Structure (quick reference)

```
src/           # C++: engine (exif, watermark, jxl), Processor, main
qml/           # QML UI: Main.qml + MacShell (Liquid Glass) / WindowsShell (Fluent)
shaders/       # Glass refraction shader
assets/        # Icons, Go Regular font
tests/         # Qt Test unit tests + fixtures
.github/
  workflows/
    build.yml    # Build, test, bundle and package (macOS arm64, Windows x64)
    ci.yml       # Runs build.yml on PRs
    release.yml  # semantic-release + build.yml + release assets
docs/          # User guides (EN + ES) and screenshots
build/         # Build output (git-ignored)
```

---

## Git Workflow

### Creating a Feature Branch

```bash
# 1. Start from latest main
git checkout main
git pull origin main

# 2. Create feature branch
git checkout -b feature/<short-description>
# Examples:
#   feature/heic-input
#   fix/arw-orientation
#   chore/update-qt

# 3. Work in baby steps — one commit per logical change
# 4. Push and open a PR against main
git push -u origin feature/<short-description>
```

### Commit Message Format (Conventional Commits)

```
<type>(<scope>): <subject>
```

| Type | Scope examples | When to use |
|------|---------------|-------------|
| `feat` | `engine`, `ui`, `mac`, `windows` | New user-visible feature |
| `fix` | `engine`, `ui`, `mac`, `windows` | Bug fix |
| `docs` | `docs`, `readme` | Documentation only |
| `chore` | `ci`, `deps`, `build` | Tooling / CI change |
| `refactor` | `engine`, `ui` | Code restructure, no behaviour change |
| `test` | `engine` | Adding or fixing tests |
| `perf` | `engine` | Performance improvement |

Examples:
```
feat(engine): read EXIF from HEIC files
fix(engine): correct EXIF orientation for tag value 8
chore(ci): cache the vcpkg build between runs
test(engine): add data-driven tests for outputPath
docs: add development guide to ai-specs
```

### Merging

- All PRs require CI to pass (macOS + Windows builds, unit tests, smoke test) before merge.
- Merge to `main` triggers semantic-release if the commit history includes a releasable change (`feat` or `fix`).

---

## Release Process

Releases are fully automated via semantic-release and GitHub Actions.

1. Merge a PR with a `feat` or `fix` commit to `main`.
2. `release.yml` runs semantic-release, which:
   - Determines the next semver version from commit messages.
   - Creates a GitHub Release with auto-generated release notes.
3. `build.yml` builds the app with that version: macOS Apple Silicon (`img-settings-macos-arm64.dmg`) and Windows x64 (`img-settings-windows-x64.zip`).
4. Both packages are uploaded as release assets.

No manual version bumping — the commit message type drives the version.

| Commit type | Version bump |
|-------------|-------------|
| `fix` | Patch (1.0.0 → 1.0.1) |
| `feat` | Minor (1.0.0 → 1.1.0) |
| `BREAKING CHANGE` footer | Major (1.0.0 → 2.0.0) |

---

## Definition of Done Checklist

Before merging any pull request, verify:

**Development**
- [ ] Implemented following `qt-standards.mdc`
- [ ] Engine code has no QML / Qt Quick dependencies
- [ ] Errors are returned as readable messages, never thrown or swallowed
- [ ] macOS and Windows UIs both updated when the change is user-visible

**Testing**
- [ ] New engine behaviour is covered by tests in `tests/tst_engine.cpp`
- [ ] Data-driven tests used where several inputs are tested
- [ ] `make test` passes
- [ ] UI checked in both styles (macOS and `IMG_SETTINGS_STYLE=windows`)

**Code Quality**
- [ ] No new compiler or QML warnings
- [ ] Public engine functions have a contract comment
- [ ] No dead code or commented-out blocks committed

**CI**
- [ ] macOS and Windows jobs pass (build, tests, smoke test, packaging)

**Documentation**
- [ ] `docs/USER_GUIDE.md` updated if user-facing behaviour changed
- [ ] `docs/USER_GUIDE.es.md` updated to match the English guide
- [ ] `README.md` / `README.es.md` updated if setup or usage changed
- [ ] Screenshots in `docs/assets/` refreshed if the UI changed
- [ ] Commit messages follow Conventional Commits format

---

## Troubleshooting

### CMake cannot find Qt

Pass the Qt prefix: `cmake -S . -B build -DCMAKE_PREFIX_PATH=<Qt>/6.11.2/<arch>`.

### `Qt6ShaderTools` not found

Install the **Qt Shader Tools** module (Homebrew's `qt` includes it; with the Qt installer, select it under Additional Libraries).

### "libjxl not found: JPEG XL (.jxl) input is disabled"

Install libjxl and pkg-config (`brew install jpeg-xl pkgconf`), then reconfigure (`make clean && make`).

### ARW files fail to decode

img-settings tries three strategies for Sony RAW files:
1. Embedded JPEG preview (no tools needed)
2. `dcraw` — `brew install dcraw`
3. ImageMagick — `brew install imagemagick`

### Unit tests fail with "could not connect to display"

Run them with `QT_QPA_PLATFORM=offscreen` (CTest sets it automatically).
