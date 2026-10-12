# img-settings — User Guide

**[🇪🇸 Leer en español](USER_GUIDE.es.md)**

---

## Table of contents

1. [What is img-settings?](#1-what-is-img-settings)
2. [Installation](#2-installation)
3. [Using the app](#3-using-the-app)
4. [Understanding the watermark](#4-understanding-the-watermark)
5. [Supported formats](#5-supported-formats)
6. [Troubleshooting](#6-troubleshooting)

---

## 1. What is img-settings?

img-settings is a small app that takes your photos and adds a subtle watermark showing the camera settings used to take the shot — aperture, shutter speed, ISO, focal length, and camera model. The result is exported as a JPG file optimised for WhatsApp HD sharing (max 2560 px on the longest side, quality 92).

It works with JPG, PNG, JPEG XL and Sony ARW (RAW) files, on macOS and Windows.

---

## 2. Installation

### Download

Go to the [Releases page](https://github.com/esteban-plaza/img-settings/releases/latest) and download the file for your system:

| System | File to download |
|---|---|
| Mac with Apple Silicon (M1 or newer), macOS 13 Ventura or later | `img-settings-macos-arm64.dmg` |
| Windows 10 or 11, 64-bit | `img-settings-windows-x64.zip` |

Not sure which Mac you have? Click the  menu → **About This Mac**. If it says "Apple M…" you have Apple Silicon. Macs with Intel processors are not supported.

### macOS

1. Open `img-settings-macos-arm64.dmg`
2. Drag **img-settings** onto the **Applications** folder

macOS will block the app the first time because it was downloaded from the internet and is not signed with an Apple developer certificate. To allow it:

**Option A — System Settings:**
1. Open img-settings — macOS blocks it and shows an alert
2. Open **System Settings → Privacy & Security**
3. Scroll down to the security section and click **Open Anyway**

**Option B — Terminal:**
```bash
xattr -d com.apple.quarantine /Applications/img-settings.app
```

### Windows

1. Right-click `img-settings-windows-x64.zip` → **Extract All…**
2. Open the extracted `img-settings` folder and double-click `img-settings.exe`

Keep `img-settings.exe` inside its folder — it needs the files next to it. If Windows Defender SmartScreen shows a warning, click **More info → Run anyway**.

---

## 3. Using the app

The app looks native on each platform: Apple's Liquid Glass style on macOS and the Windows 11 style on Windows. Both work the same way.

### Add your photos

![Drop zone on macOS](assets/screenshot-macos-dropzone.png)

![Drop zone on Windows](assets/screenshot-windows-dropzone.png)

You can:

- **Drag and drop** one or more photos onto the window
- **Drag and drop a folder** to process all photos inside it
- **Click the drop zone** (macOS) or **Choose folder…** (Windows) to pick a folder
- Click **Choose Photos…** to pick individual photos
- Press **⌘O** (macOS) or **Ctrl+O** (Windows) to open the folder picker

The badges under the drop zone (macOS) or the text below it (Windows) list the formats this version supports. Processing starts immediately.

### Settings

The settings bar is at the bottom of the window.

#### Subfolders
When you add a folder, this option controls whether the app also looks inside its subfolders.

- **Off (default):** only photos directly inside the folder
- **On:** every photo in every subfolder

#### Opacity
How visible the watermark background is, from 0% to 100%. The default is **82%**, which gives a clean look without being too intrusive.

#### Output (macOS) / Save to (Windows)
The folder where the processed photos are saved.

- **Default — "Auto (./out)":** an `out/` folder is created next to your photos (inside the folder you added, or next to the first photo)
- **Custom:** click the button to choose a different folder
- **Back to automatic:** click the **×** next to the folder (macOS) or **Reset** (Windows)

> The original photos are never modified. Photos that are already inside the output folder are skipped, so running the app twice on the same folder does not stamp them twice.

### Progress and results

![Processing on macOS](assets/screenshot-macos-processing.png)

While working, the window shows a progress bar and a grid with a preview of each finished photo and its size. Several photos are processed in parallel. Click **Stop** (or press **Esc**) to cancel the remaining photos.

If a photo cannot be processed, its tile shows an error badge — hover over it to read the reason.

![Done on macOS](assets/screenshot-macos-done.png)

![Done on Windows](assets/screenshot-windows-done.png)

When everything is done:

- **Reveal in Finder / Open in File Explorer** — opens the output folder
- **Process More** — goes back to the start to process another batch
- Click a preview to open that photo

---

## 4. Understanding the watermark

The watermark is a dark pill-shaped label placed at the **bottom centre** of the image. It shows the following information when it is available in the photo's EXIF data:

| Icon | Field | Example |
|---|---|---|
| Camera | Camera model | ILCE-7RM5 |
| Aperture blades | Aperture | f/2.8 |
| Clock | Shutter speed | 1/250 |
| Chip | ISO | ISO 400 |
| Lens circles | Focal length | 85mm |

**Fields with no data are simply omitted.** If a photo has no EXIF data at all (for example, a screenshot), the photo is exported without a watermark.

The watermark size adapts to the image: it scales with the image height and shrinks the text if needed so everything fits within the image width.

Photos are rotated according to their EXIF orientation, so portrait shots come out upright.

---

## 5. Supported formats

| Format | Notes |
|---|---|
| JPG / JPEG | |
| PNG | EXIF is read from the `eXIf` chunk when present |
| JPEG XL (`.jxl`) | EXIF is read even when compressed; wide-gamut and HDR images are converted to sRGB |
| ARW (Sony RAW) | See below |

Output is always JPG.

### Sony RAW files (ARW)

img-settings tries three methods in order:

1. **Embedded JPEG preview** — most Sony cameras embed a full-resolution JPEG inside the ARW file. img-settings extracts it directly; this is fast and needs no extra tools.
2. **dcraw** — if no usable preview is found, img-settings uses dcraw to decode the RAW data.
3. **ImageMagick** — if dcraw is not installed, it falls back to ImageMagick.

To install the fallback tools on macOS:

```bash
brew install dcraw
# or
brew install imagemagick
```

---

## 6. Troubleshooting

**The app does not open on macOS**
Follow the steps in [macOS](#macos). macOS blocks apps that are not signed with an Apple developer certificate.

**The app does not start on Windows**
Make sure you extracted the whole `.zip` and run `img-settings.exe` from inside the extracted folder — running it from inside the zip or moving the `.exe` alone does not work.

**The watermark is not showing**
The photo likely has no EXIF data. This can happen with screenshots, photos exported from some apps, or images that had their metadata stripped. Try a photo taken directly with a camera or phone.

**"No supported photos found"**
The files or folder you added contain no supported photos. If the photos are in subfolders, turn on **Subfolders**.

**ARW files show an error**
Install dcraw or ImageMagick (see [Sony RAW files](#sony-raw-files-arw)).

**The output photo is the same size as the original**
If your photo is already 2560 px or smaller on its longest side, img-settings does not resize it.
