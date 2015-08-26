#!/usr/bin/env bash
set -e

scriptDir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
root="${scriptDir}/../.."

sdkDir=${root}/nrf51_sdk
sdkZip=${sdkDir}/nrf51_sdk_v6_1_0_b2ec2e6.zip
if [ ! -f ${sdkZip} ]; then
    curl -L -o ${sdkZip} http://developer.nordicsemi.com/nRF51_SDK/nRF51_SDK_v6.x.x/nrf51_sdk_v6_1_0_b2ec2e6.zip
    unzip -q ${sdkDir}/nrf51_sdk_v6_1_0_b2ec2e6.zip -d ${sdkDir}
fi

s110Dir=${root}/s110
s110Zip=${s110Dir}/s110_nrf51822_7.0.0.zip
if [ ! -f ${s110Zip} ]; then
    curl -L -o ${s110Zip} http://www.nordicsemi.com/eng/nordic/download_resource/48420/8/62400196
    unzip -q ${s110Dir}/s110_nrf51822_7.0.0.zip -d ${s110Dir}
fi
