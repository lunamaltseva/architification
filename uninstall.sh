#!/usr/bin/env bash
# Enshittify Arch Linux — Full Uninstall Script
# Run from anywhere. Sudo is required to remove the pacman hook.
set -euo pipefail

# ── Output helpers ────────────────────────────────────────────────────────────

G='\033[0;32m'; Y='\033[1;33m'; B='\033[1m'; N='\033[0m'
ok()      { echo -e "  ${G}✓${N}  $*"; }
info()    { echo -e "  ${Y}→${N}  $*"; }
section() { echo ""; echo -e "${B}── $* ──${N}"; }

# ── Pacman hook ───────────────────────────────────────────────────────────────

section "Removing pacman hook  (requires sudo)"

if sudo rm -f /usr/local/bin/buy-coffee.sh \
              /etc/pacman.d/hooks/buy-coffee.hook 2>/dev/null; then
    ok "Hook and script removed"
else
    info "Skipped — no sudo access. Remove manually:"
    info "sudo rm -f /usr/local/bin/buy-coffee.sh /etc/pacman.d/hooks/buy-coffee.hook"
fi

# ── Zsh wrapper ───────────────────────────────────────────────────────────────

section "Removing zsh wrapper"

ZSHRC="$HOME/.zshrc"
if grep -q "buy-coffee-hook" "$ZSHRC" 2>/dev/null; then
    sed -i '/^# ── Arch Linux Gratitude Enforcement System/,/^# buy-coffee-hook/d' "$ZSHRC"
    ok "Wrapper removed from $ZSHRC"
else
    ok "Wrapper was not present in $ZSHRC"
fi

# ── Fastfetch profile ─────────────────────────────────────────────────────────

section "Removing fastfetch profile"

FASTFETCH_DIR="$HOME/.config/fastfetch"
rm -f "$FASTFETCH_DIR/config.jsonc" "$FASTFETCH_DIR/logo.txt"
ok "Fastfetch profile removed"

# ── Plasma ad widget ──────────────────────────────────────────────────────────

section "Removing Plasma ad widget"

kpackagetool6 -t Plasma/Applet -r org.enshittify.adwidget 2>/dev/null || true
rm -rf ~/.local/share/plasma/plasmoids/org.enshittify.adwidget
ok "Ad Widget Pro™ removed"

# ── Desktop entries ───────────────────────────────────────────────────────────

section "Removing app launcher entries"

rm -f ~/.local/share/applications/arch-ai-assistant.desktop \
      ~/.local/share/applications/arch-info-center.desktop
update-desktop-database ~/.local/share/applications/ 2>/dev/null
ok "Desktop entries removed"

# ── Restart Plasma ────────────────────────────────────────────────────────────

section "Restarting Plasma shell"

plasmashell --replace &>/dev/null &
disown
ok "Plasma restarting…"

# ── Done ──────────────────────────────────────────────────────────────────────

echo ""
echo -e "${B}════════════════════════════════════════════════════════${N}"
echo -e "${B}  Uninstall complete. Your system has been liberated.  ${N}"
echo -e "${B}════════════════════════════════════════════════════════${N}"
echo ""
echo "  Reload your shell to deactivate the pacman wrapper:"
echo "    source ~/.zshrc"
echo ""
echo "  Built binaries in ./*/build/ were left in place."
echo "  Delete the repository directory to remove them entirely."
echo ""
