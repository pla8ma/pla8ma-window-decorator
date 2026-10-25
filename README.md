# Pla8ma Window Decorator

[![Copr build status](https://copr.fedorainfracloud.org/coprs/n0x073/pla8ma/package/pla8ma-window-decorator/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/n0x073/pla8ma/package/pla8ma-window-decorator/)

A flat KWin window decoration for Plasma 6.7. It is a native KDecoration3
decoration written in C++, like Breeze, not an Aurorae/SVG theme.

- flat, single colored frame with square corners (8 px including the outline with border size "Normal")
- minimize / maximize / close buttons attached to the top edge, red close button
- title centered over the full width, window icon on the left
- hover: blue with a white glyph (or a lighter frame color), close button turns bright red
- inactive windows: light gray frame with a gray close button (or a faded frame color)
- thin dark outline around the window and the window content, soft shadow

## Color

By default the frame uses the **accent color of the KDE color scheme** and
follows it immediately when you change the color scheme or accent color in
System Settings. This also works for color schemes set by individual applications.

Configurable under *System Settings → Colors & Themes → Window Decorations → Pla8ma Window Decorator → Edit*:

| Option | Values |
| --- | --- |
| Frame color | accent color · title bar color of the color scheme · custom color |
| Color intensity | 0–100 % (blends towards the window background color) |
| Inactive windows | classic (light gray, gray close button) · faded · same as active · inactive title bar color of the scheme · custom color |
| Title text | automatic black/white by contrast · text color of the scheme · custom color |
| Button hover color | classic (blue) · lighter frame color |
| More | title alignment, button size, red close button, outline, shadow |

As with Breeze, the border width comes from the global "Border size" setting.

## Build and install (Fedora)

```sh
sudo dnf install kdecoration-devel kf6-kcmutils-devel kf6-kcolorscheme-devel \
    kf6-kconfig-devel kf6-kcoreaddons-devel kf6-kguiaddons-devel kf6-ki18n-devel \
    kf6-kwidgetsaddons-devel qt6-qtbase-devel extra-cmake-modules gcc-c++ cmake gettext

cmake -B build -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
sudo cmake --install build
```

Then select "Pla8ma Window Decorator" under *Window Decorations* in System Settings.
If it does not show up, log out and back in (or reload KWin).

Installed files:

- `/usr/lib64/qt6/plugins/org.kde.kdecoration3/pla8madecoration.so`
- `/usr/lib64/qt6/plugins/org.kde.kdecoration3.kcm/kcm_pla8madecoration.so`
- `/usr/share/locale/de/LC_MESSAGES/pla8ma_kwin_deco.mo`

Settings are stored in `~/.config/pla8madecorationrc`.

**Uninstall:** `sudo xargs rm < build/install_manifest.txt`

## RPM / Fedora COPR

The spec file is `pla8ma-window-decorator.spec`. `.copr/Makefile` builds the
SRPM from the current git state (`HEAD`).

Create a package in COPR with:

- **Source Type:** SCM
- **Clone URL:** `https://github.com/pla8ma/pla8ma-window-decorator.git`
- **Spec File:** `pla8ma-window-decorator.spec`
- **SRPM build method:** `make_srpm`

Build an SRPM locally: `make -f .copr/Makefile srpm` (output goes to `srpm/`),
then build the binary RPM with `rpmbuild --rebuild srpm/*.src.rpm`.

Rebuild the package for every new Plasma feature release (e.g. 6.8), and update
`Version`/`Release` and `%changelog` in the spec when something changes.

## Translations

The strings in the code are English. The German translation is in
`po/de/pla8ma_kwin_deco.po` and is installed during the build. Plasma shows
German or English depending on the system language.

After changing strings, regenerate the template and merge it into the `.po` file:

```sh
cd src
XGETTEXT="xgettext --from-code=UTF-8 -C --kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3" \
    podir=/tmp bash Messages.sh
msgmerge -U ../po/de/pla8ma_kwin_deco.po /tmp/pla8ma_kwin_deco.pot
```

To add another language, create `po/<language>/pla8ma_kwin_deco.po`.

## Layout

| File | Contents |
| --- | --- |
| `src/pla8madecoration.*` | frame, borders, title, colors, shadow |
| `src/pla8mabutton.*` | buttons and glyphs |
| `src/pla8masettingsdata.kcfg` | configuration schema (KConfigXT) |
| `src/pla8masettingsprovider.*` | settings shared by all windows |
| `src/pla8maboxshadowrenderer.*` | shadow renderer, taken from Breeze |
| `src/config/` | settings module (KCM) |
| `po/` | translations |

## License

The code taken from Breeze keeps its original license,
GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL. The shadow
renderer is GPL-2.0-or-later and the settings module is MIT. The license
texts are in `LICENSES/`.
