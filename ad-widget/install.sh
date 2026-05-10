#!/usr/bin/env bash
set -e

echo "Installing Ad Widget Pro™ plasmoid..."

kpackagetool6 -t Plasma/Applet -i . 2>/dev/null || \
kpackagetool6 -t Plasma/Applet -u . 2>/dev/null

echo ""
echo "✓ Ad Widget Pro™ installed!"
echo ""
echo "To add to your desktop:"
echo "  Right-click desktop → Add or Manage Widgets"
echo "  Search for 'Ad Widget Pro'"
echo "  Drag onto desktop"
echo ""
echo "  Then right-click the widget → Configure"
echo "  to select your ad image."
echo ""
echo "  Congratulations! You are now advertising to yourself."
