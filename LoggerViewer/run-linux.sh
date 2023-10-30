#!/bin/bash
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
appname="file:"$PWD"/index.html"
google-chrome  --disable-translate --no-first-run   --disable-improved-download-protection  --app=$appname

