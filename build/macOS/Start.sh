#!/bin/bash -e

echo
echo "Starting build MultiCraft for macOS..."

echo
echo "Build Libraries:"

cd deps
sh libraries.sh
cd ..

echo
echo "Preparing Locales:"

sh locale.sh

echo
echo "Creating App Icon:"

echo "Skipping..."
#sh appicon.sh

echo
echo "Install CocoaPods:"

pod install

echo
echo "All done! You can continue in Xcode!"
open MultiCraft.xcworkspace
