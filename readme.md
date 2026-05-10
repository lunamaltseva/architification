## Project 4: Death by "Userbase Left"

The project includes:

- An "AI" assistant, which tells users to switch to Windows OR executes requests incredibly poorly
- A monthly subscription to the distribution, the payment processor for which does not work (presumably because it was vibecoded)
- An "ad-widget" which comes preinstalled by default
- A fastfetch profile which will not display the logo of the distribution unless on a paid tier
- An override for pacman -Syu which demands that the user purchase a paid tier to have access to updates
- And a "restart system" pop up which activates every thirty seconds.

Media coverage of the change:

[lunamaltseva.dev/breakingnews](lunamaltseva.dev/breakingnews)

Install at your own risk.

---

## Installation

**Requirements:** KDE Plasma 6, Arch Linux, Qt6, CMake, Ninja, `kdialog`, `fastfetch`, `zsh`

```bash
sudo pacman -S qt6-base cmake ninja kdialog fastfetch
```

### 1. Build the Qt apps

```bash
bash build-all.sh
```

Builds `info-center`, `restart-popup`, and `ai-assistant` into their respective `build/` directories.

### 2. Pacman hook + zsh wrapper

```bash
# Install the hook system-wide (requires sudo)
sudo bash pacman-hook/install.sh

# Install the zsh wrapper so "sudo pacman -Syu" is intercepted
bash pacman-hook/setup-zsh.sh
source ~/.zshrc
```

### 3. Fastfetch profile

```bash
cd fastfetch-profile
bash install.sh
```

### 4. KDE Plasma ad widget

Place your ad image at `ad-widget/horrible_ad.png`, then:

```bash
kpackagetool6 --type Plasma/Applet --install ad-widget
```

Restart Plasma (`plasmashell --replace &`), then add **Ad Widget Pro™** from the widget picker.

### 5. App launcher entries

```bash
REPO=$(pwd)
sed "s|{REPO}|$REPO|" ai-assistant/arch-ai-assistant.desktop > ~/.local/share/applications/arch-ai-assistant.desktop
sed "s|{REPO}|$REPO|" info-center/arch-info-center.desktop   > ~/.local/share/applications/arch-info-center.desktop
update-desktop-database ~/.local/share/applications/
```

To change the AI assistant icon, edit `~/.local/share/applications/arch-ai-assistant.desktop` and set `Icon=` to any theme icon name or an absolute path to an image.

### 6. Autostart (not recommended)

To have the restart popup and AI assistant launch on login, add their binaries to **System Settings → Autostart**:

- `restart-popup/build/restart-popup`
- `ai-assistant/build/ai-assistant`