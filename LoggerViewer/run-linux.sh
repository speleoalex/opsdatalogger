#!/bin/bash
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
appname="file:"$PWD"/loggermanager.htm"
rm -rf chrome/*
mkdir chrome/Default



# Verifica se si sta utilizzando Wayland
if [ "$XDG_SESSION_TYPE" = "wayland" ]; then
google-chrome -user-data-dir=./chrome --window-size=768,768 --disable-translate --no-first-run --disable-improved-download-protection  --app=$appname -no-default-browser-check  --allow-running-insecure-content --enable-experimental-web-platform-features --enable-features=UseOzonePlatform=wayland
else
    google-chrome -user-data-dir=./chrome --window-size=768,768 --disable-translate --no-first-run --disable-improved-download-protection  --app=$appname -no-default-browser-check  --allow-running-insecure-content --enable-experimental-web-platform-features
fi
