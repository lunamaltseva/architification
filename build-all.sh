#!/usr/bin/env bash
set -e

ROOT="$(cd "$(dirname "$0")" && pwd)"
APPS=(info-center restart-popup ai-assistant)

echo "════════════════════════════════════════════"
echo "  Enshittify Arch Linux — Build System™"
echo "  (Not responsible for emotional damage)"
echo "════════════════════════════════════════════"
echo ""

for app in "${APPS[@]}"; do
    echo "──── Building $app ────"
    cd "$ROOT/$app"
    mkdir -p build
    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev 2>&1 | tail -3
    ninja -C build 2>&1
    echo "✓  $app built → $ROOT/$app/build/$app"
    echo ""
done

echo "════════════════════════════════════════════"
echo "  All apps built. Run them with:"
echo ""
for app in "${APPS[@]}"; do
    echo "    ./$app/build/$app &"
done
echo ""
echo "  Install the plasmoid:"
echo "    cd ad-widget && bash install.sh"
echo ""
echo "  Install fastfetch profile:"
echo "    cd fastfetch-profile && bash install.sh"
echo ""
echo "  Install pacman hook (requires root):"
echo "    cd pacman-hook && sudo bash install.sh"
echo "════════════════════════════════════════════"
