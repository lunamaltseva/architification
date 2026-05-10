#!/usr/bin/env bash
set -e

FASTFETCH_DIR="$HOME/.config/fastfetch"
mkdir -p "$FASTFETCH_DIR"

# Expand ~ in logo source to absolute path
sed "s|/home/lunamaltseva|$HOME|g" config.jsonc > "$FASTFETCH_DIR/config.jsonc"
cp logo.txt     "$FASTFETCH_DIR/logo.txt"

echo "✓ Fastfetch profile installed to $FASTFETCH_DIR"
echo "  Run: fastfetch"
echo ""
echo "  Your distro logo is now locked behind a paywall."
echo "  This is fine. This is the Arch experience now."
