$POCO_URL ="http://pocoproject.org/releases/poco-1.7.3/poco-1.7.3.zip"

wget $POCO_URL -Outfile poco.zip

Expand-Archive "poco.zip" -DestinationPath ".\\"

pushd poco-1.7.3
.\\buildwin.cmd 140 build static_mt both Win32 nosamples notests
popd
