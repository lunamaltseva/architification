#!/usr/bin/env bash
set -e

echo "Installing Arch Linux Gratitude Enforcement System™..."

install -m 755 buy-coffee.sh /usr/local/bin/buy-coffee.sh

install -d /etc/pacman.d/hooks
install -m 644 buy-coffee.hook /etc/pacman.d/hooks/buy-coffee.hook

echo "✓ Pacman hook installed."
echo "  The hook triggers on every 'pacman -Syu'."
echo "  Users will be encouraged (non-optionally) to donate."
