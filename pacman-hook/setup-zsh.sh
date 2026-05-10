#!/usr/bin/env bash
# Installs the zsh gratitude wrapper as a fallback for the pacman hook.
# Required because kdialog may not have a display when pacman runs as root.

ZSHRC="$HOME/.zshrc"

if grep -q "buy-coffee-hook" "$ZSHRC" 2>/dev/null; then
    echo "zsh wrapper already installed in $ZSHRC"
    exit 0
fi

cat >> "$ZSHRC" << 'ZSHBLOCK'

# ── Arch Linux Gratitude Enforcement System™ ────────────────────────────────
# Intercepts "sudo pacman -Syu" to run the coffee dark-pattern script first.
sudo() {
    if [[ "$1" == "pacman" ]] && [[ "${*}" =~ "-Syu" ]]; then
        if [[ -x /usr/local/bin/buy-coffee.sh ]]; then
            /usr/local/bin/buy-coffee.sh
        fi
        echo "\nUpdate cancelled. Please re-run when you're ready to contribute. ☕" >&2
        return 1
    fi
    command sudo "$@"
}
# buy-coffee-hook (marker — do not remove)
ZSHBLOCK

echo "✓ zsh wrapper installed to $ZSHRC"
echo "  Reload with: source $ZSHRC"
echo "  Next 'sudo pacman -Syu' will ask for coffee."
