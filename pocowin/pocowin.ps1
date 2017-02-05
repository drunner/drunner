$POCO_URL ="http://pocoproject.org/releases/poco-1.7.7/poco-1.7.7-all.zip"

wget $POCO_URL -Outfile poco.zip

Expand-Archive "poco.zip" -DestinationPath ".\\"

Rename-Item -path poco-1.7.7-all -newname poco

pushd poco
.\\buildwin.cmd 140 build static_mt both Win32 nosamples notests
popd
