#!/usr/bin/env bash
# Enshittify Arch Linux — Full Installation Script
# Run from the repository root. Sudo is required for the pacman hook.
set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# ── Output helpers ────────────────────────────────────────────────────────────

G='\033[0;32m'; Y='\033[1;33m'; R='\033[0;31m'; B='\033[1m'; N='\033[0m'
ok()      { echo -e "  ${G}✓${N}  $*"; }
info()    { echo -e "  ${Y}→${N}  $*"; }
fail()    { echo -e "  ${R}✗${N}  $*"; }
section() { echo ""; echo -e "${B}── $* ──${N}"; }

# ── Dependency check ──────────────────────────────────────────────────────────

section "Checking dependencies"

declare -A PKGMAP=(
    [cmake]="cmake"
    [ninja]="ninja"
    [kdialog]="kdialog"
    [fastfetch]="fastfetch"
    [kpackagetool6]="plasma-framework"
)

MISSING_PKGS=()
for cmd in "${!PKGMAP[@]}"; do
    if command -v "$cmd" &>/dev/null; then
        ok "$cmd"
    else
        fail "$cmd — not found"
        MISSING_PKGS+=("${PKGMAP[$cmd]}")
    fi
done

if [ "${#MISSING_PKGS[@]}" -gt 0 ]; then
    echo ""
    fail "Install missing packages, then re-run:"
    echo -e "      sudo pacman -S ${MISSING_PKGS[*]}"
    exit 1
fi

# ── Build Qt apps ─────────────────────────────────────────────────────────────

section "Building Qt apps"

for app in info-center restart-popup ai-assistant; do
    info "Building $app…"
    cd "$REPO/$app"
    mkdir -p build
    # Capture cmake output; show it only on failure
    cmake_out="$(cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev 2>&1)" \
        || { echo "$cmake_out"; exit 1; }
    ninja -C build 2>&1 | grep -v "^\[" || true   # suppress progress lines
    ok "$app  →  $REPO/$app/build/$app"
done
cd "$REPO"

# ── Pacman hook ───────────────────────────────────────────────────────────────

section "Pacman hook  (requires sudo)"

if sudo install -m 755 pacman-hook/buy-coffee.sh  /usr/local/bin/buy-coffee.sh  2>/dev/null \
&& sudo install -d /etc/pacman.d/hooks                                           2>/dev/null \
&& sudo install -m 644 pacman-hook/buy-coffee.hook /etc/pacman.d/hooks/buy-coffee.hook 2>/dev/null; then
    ok "Hook installed → /etc/pacman.d/hooks/buy-coffee.hook"
    ok "Script installed → /usr/local/bin/buy-coffee.sh"
else
    fail "Skipped — no sudo access. Run manually:"
    info "sudo bash pacman-hook/install.sh"
fi

# ── Zsh wrapper ───────────────────────────────────────────────────────────────

section "Zsh wrapper"

bash "$REPO/pacman-hook/setup-zsh.sh"

# ── Fastfetch profile ─────────────────────────────────────────────────────────

section "Fastfetch profile"

cd "$REPO/fastfetch-profile"
bash install.sh
cd "$REPO"

# ── Plasma ad widget ──────────────────────────────────────────────────────────

section "Plasma ad widget"

# Always do a clean install to pick up the latest metadata
rm -rf ~/.local/share/plasma/plasmoids/org.enshittify.adwidget
if kpackagetool6 -t Plasma/Applet -i "$REPO/ad-widget" 2>/dev/null; then
    ok "Ad Widget Pro™ installed"
else
    fail "Plasmoid install failed — is kpackagetool6 working?"
fi

# ── Desktop entries ───────────────────────────────────────────────────────────

section "App launcher entries"

mkdir -p ~/.local/share/applications
sed "s|{REPO}|$REPO|g" "$REPO/ai-assistant/arch-ai-assistant.desktop" \
    > ~/.local/share/applications/arch-ai-assistant.desktop
sed "s|{REPO}|$REPO|g" "$REPO/info-center/arch-info-center.desktop" \
    > ~/.local/share/applications/arch-info-center.desktop
update-desktop-database ~/.local/share/applications/ 2>/dev/null
ok "arch-ai-assistant.desktop"
ok "arch-info-center.desktop"

# ── Restart Plasma ────────────────────────────────────────────────────────────

section "Restarting Plasma shell"

plasmashell --replace &>/dev/null &
disown
ok "Plasma restarting…"

# ── Done ──────────────────────────────────────────────────────────────────────

echo ""
echo -e "${B}════════════════════════════════════════════════════════${N}"
echo -e "${B}  Installation complete. Your Arch experience is ruined.${N}"
echo -e "${B}════════════════════════════════════════════════════════${N}"
echo ""
echo "  Activate the pacman wrapper in your current shell:"
echo "    source ~/.zshrc"
echo ""
echo "  Add the ad widget:"
echo "    Right-click desktop → Add Widgets → search 'Ad Widget Pro™'"
echo ""
echo "  Optional — add to autostart via System Settings → Autostart:"
echo "    $REPO/restart-popup/build/restart-popup"
echo "    $REPO/ai-assistant/build/ai-assistant"
echo ""
