#!/bin/bash
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
appname="file:"$PWD"/loggermanager.htm"
google-chrome --window-size=768,768 --disable-translate --no-first-run --disable-improved-download-protection  --app=$appname

