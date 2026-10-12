# img-settings

> Agregá los datos de tu cámara como marca de agua en tus fotos — listas para compartir por WhatsApp.

**[🇬🇧 Read in English](README.md)** · **[📖 Manual de usuario completo](docs/USER_GUIDE.es.md)**

![img-settings en macOS](docs/assets/screenshot-macos-dropzone.png)

---

img-settings lee los datos EXIF de tus fotos (apertura, velocidad de obturación, ISO, distancia focal, modelo de cámara) y los estampa como una marca de agua discreta en la parte inferior central de la imagen. El resultado siempre es un JPG optimizado para WhatsApp HD (máx. 2560 px).

Es una app de escritorio nativa para **macOS** (con el estilo Liquid Glass de Apple) y **Windows** (con el estilo de Windows 11), hecha con Qt 6.

---

## Descarga

Entrá a la página de [Releases](https://github.com/esteban-plaza/img-settings/releases/latest) y descargá el archivo para tu plataforma:

| Plataforma | Archivo |
|---|---|
| macOS — Apple Silicon (M1 o posterior), macOS 13+ | `img-settings-macos-arm64.dmg` |
| Windows 10 / 11 (64 bits) | `img-settings-windows-x64.zip` |

**macOS:** abrí el `.dmg` y arrastrá **img-settings** a **Aplicaciones**. La app no está firmada con un certificado de desarrollador de Apple, así que la primera vez macOS la va a bloquear: abrí **Configuración del Sistema → Privacidad y seguridad** y hacé clic en **Abrir de todos modos** (o ejecutá `xattr -d com.apple.quarantine /Applications/img-settings.app`).

**Windows:** descomprimí el archivo y ejecutá `img-settings.exe` dentro de la carpeta `img-settings`. Si SmartScreen muestra una advertencia, hacé clic en **Más información → Ejecutar de todos modos**.

---

## Cómo usarlo

1. Abrí la app
2. **Arrastrá y soltá** tus fotos o una carpeta en la ventana — o hacé clic para elegir una carpeta o fotos
3. Ajustá las opciones:
   - **Subfolders** — procesar también las fotos dentro de subcarpetas (desactivado por defecto)
   - **Opacity** — qué tan visible es la marca de agua (82% por defecto)
   - **Output** (macOS) / **Save to** (Windows) — dónde guardar los resultados (por defecto: una carpeta `out/` junto a tus fotos)
4. Listo — hacé clic en **Reveal in Finder** (macOS) u **Open in File Explorer** (Windows) para ver tus fotos

Tus fotos originales nunca se modifican.

---

## Formatos soportados

| Formato | Soporte |
|---|---|
| JPG / JPEG | ✓ |
| PNG | ✓ |
| JPEG XL (`.jxl`) | ✓ — incluidas imágenes de gama amplia / HDR (se convierten a sRGB) |
| ARW (RAW de Sony) | ✓ — usa la vista previa embebida; si no hay, recurre a `dcraw` o ImageMagick |

La mayoría de los ARW funcionan sin instalar nada. Si alguno falla, instalá una de estas herramientas:
```bash
brew install dcraw          # recomendado
brew install imagemagick    # alternativa
```

---

## Compilar desde el código fuente

Requiere Qt 6.9+ (con Qt Shader Tools) y CMake 3.21+. libjxl es opcional y habilita la entrada JPEG XL.

```bash
brew install qt cmake jpeg-xl pkgconf   # macOS

git clone https://github.com/esteban-plaza/img-settings.git
cd img-settings

make        # compila la app y los tests en build/
make test   # corre los tests unitarios
make run    # abre la app (macOS)
```

Mirá la [guía de desarrollo](ai-specs/specs/development_guide.md) (en inglés) para compilar en Windows y para el proceso de release.

---

## Licencia

[MIT](LICENSE). La marca de agua usa la fuente [Go Regular](https://go.dev/blog/go-fonts), bajo la licencia de [assets/Go-Font-LICENSE](assets/Go-Font-LICENSE).
