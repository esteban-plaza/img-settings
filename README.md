# img-settings

> Add your camera settings as a watermark to your photos — ready to share on WhatsApp.

**[🇪🇸 Leer en español](README.es.md)** · **[📖 Full user guide](docs/USER_GUIDE.md)**

![img-settings on macOS](docs/assets/screenshot-macos-dropzone.png)

---

img-settings reads the EXIF data embedded in your photo (aperture, shutter speed, ISO, focal length, camera model) and stamps it as a clean pill-shaped watermark at the bottom centre of the image. Output is always JPG at WhatsApp HD quality (max 2560 px).

It is a native desktop app for **macOS** (with Apple's Liquid Glass look) and **Windows** (with the Windows 11 look), built with Qt 6.

---

## Download

Go to the [Releases](https://github.com/esteban-plaza/img-settings/releases/latest) page and download the file for your platform:

| Platform | File |
|---|---|
| macOS — Apple Silicon (M1 or newer), macOS 13+ | `img-settings-macos-arm64.dmg` |
| Windows 10 / 11 (64-bit) | `img-settings-windows-x64.zip` |

**macOS:** open the `.dmg` and drag **img-settings** to **Applications**. The app is not signed with an Apple developer certificate, so the first time macOS will block it: open **System Settings → Privacy & Security** and click **Open Anyway** (or run `xattr -d com.apple.quarantine /Applications/img-settings.app`).

**Windows:** unzip the file and run `img-settings.exe` inside the `img-settings` folder. If SmartScreen shows a warning, click **More info → Run anyway**.

---

## How to use

1. Open the app
2. **Drag and drop** your photos or a folder onto the window — or click to choose a folder or photos
3. Tweak the settings:
   - **Subfolders** — also process photos inside subfolders (off by default)
   - **Opacity** — how visible the watermark is (default 82%)
   - **Output** (macOS) / **Save to** (Windows) — where to save the results (default: an `out/` folder next to your photos)
4. Done — click **Reveal in Finder** (macOS) or **Open in File Explorer** (Windows) to see your photos

Your original photos are never modified.

---

## Supported formats

| Format | Support |
|---|---|
| JPG / JPEG | ✓ |
| PNG | ✓ |
| JPEG XL (`.jxl`) | ✓ — including wide-gamut / HDR images (converted to sRGB) |
| ARW (Sony RAW) | ✓ — uses the embedded preview; falls back to `dcraw` or ImageMagick |

Most ARW files work out of the box. If one does not, install one of these tools:
```bash
brew install dcraw          # recommended
brew install imagemagick    # alternative
```

---

## Build from source

Requires Qt 6.9+ (with Qt Shader Tools) and CMake 3.21+. libjxl is optional and enables JPEG XL input.

```bash
brew install qt cmake jpeg-xl pkgconf   # macOS

git clone https://github.com/esteban-plaza/img-settings.git
cd img-settings

make        # build the app and the tests into build/
make test   # run the unit tests
make run    # open the app (macOS)
```

See the [development guide](ai-specs/specs/development_guide.md) for Windows builds and the release process.

---

## License

[MIT](LICENSE). The watermark uses the [Go Regular](https://go.dev/blog/go-fonts) font, under the license in [assets/Go-Font-LICENSE](assets/Go-Font-LICENSE).
