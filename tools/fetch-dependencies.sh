
#!/usr/bin/env bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR=$SCRIPT_DIR/..
pushd $ROOT_DIR

mkdir -p vendor
cd vendor

echo "9eb8b18c140135ab05066ad7394260f95413cb6cb403c623d389d131ceb5e381 nrf5sdk1230.zip" | sha256sum -c -

# Download if file not correct
if [ $? -ne 0 ]; then
    echo "Downloading nrf5sdk1230.zip"
    rm -f nrf5sdk1230.zip
    wget https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/sdks/nrf5/binaries/nrf5sdk1230.zip
fi

# Unzip the SDK making sure we have a clean copy
rm -rf nrf5sdk
mkdir -p nrf5sdk
unzip -o nrf5sdk1230.zip -d nrf5sdk
mv nrf5sdk/nRF5_SDK_12.3.0_d7731ad/* nrf5sdk
rmdir nrf5sdk/nRF5_SDK_12.3.0_d7731ad

# convert line endings to unix
mv nrf5sdk/components/toolchain/gcc/Makefile.posix nrf5sdk/components/toolchain/gcc/Makefile.posix.orig
tr -d '\r' < nrf5sdk/components/toolchain/gcc/Makefile.posix.orig > nrf5sdk/components/toolchain/gcc/Makefile.posix
mv nrf5sdk/components/libraries/mailbox/app_mailbox.c nrf5sdk/components/libraries/mailbox/app_mailbox.c.orig
tr -d '\r' < nrf5sdk/components/libraries/mailbox/app_mailbox.c.orig > nrf5sdk/components/libraries/mailbox/app_mailbox.c
# Patch SDK compiler configuration
patch -p0 < ../tools/nrf5sdk.patch

popd
