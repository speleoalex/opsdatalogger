#!/bin/bash
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
appname="file:"$PWD"/index.html"
google-chrome  --disable-translate --no-first-run  --app-window-size=1024,768 --disable-improved-download-protection  --app=$appname

