#!/bin/bash
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
appname="file:"$PWD"/terminal_V2.htm"
rm -rf /tmp/chrometerminal/*
mkdir /tmp/chrometerminal

echo '{"download": {"directory_upgrade": true,"extensions_to_open": "","prompt_for_download": true,"experimental-web-platform-features":true}}' > /tmp/chrometerminal/Default/Preferences
google-chrome -user-data-dir=/tmp/chrometerminal/ --window-size=768,768 --disable-translate --no-first-run --disable-improved-download-protection  --app=$appname -no-default-browser-check  --allow-running-insecure-content --enable-experimental-web-platform-features

