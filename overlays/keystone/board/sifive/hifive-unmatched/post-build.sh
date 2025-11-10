#!/bin/sh

FUSDK_VER="2023.08.00"

# Copy extlinux config
mkdir -p $TARGET_DIR/boot/extlinux
cp $BR2_EXTERNAL_KEYSTONE_PATH/board/sifive/hifive-unmatched/extlinux.conf $TARGET_DIR/boot/extlinux/extlinux.conf

# Install udev rules & systemd units
mkdir -p $TARGET_DIR/usr/lib/systemd/system
mkdir -p $TARGET_DIR/etc/udev/rules.d
# Install udev rules & systemd units
mkdir -p $TARGET_DIR/usr/lib/systemd/system
mkdir -p $TARGET_DIR/etc/udev/rules.d

if [ ! -d $BR2_EXTERNAL_KEYSTONE_PATH/board/sifive/hifive-unmatched/sdk/rules.d ]; then
    wget -P "${BR2_EXTERNAL_KEYSTONE_PATH}/board/sifive/hifive-unmatched/sdk/rules.d" "https://raw.githubusercontent.com/sifive/freedom-u-sdk/${FUSDK_VER}/recipes-sifive/unmatched-udev-rules/files/unmatched/99-pwm-leds.rules"
fi

cp -r $BR2_EXTERNAL_KEYSTONE_PATH/board/sifive/hifive-unmatched/sdk/rules.d $TARGET_DIR/etc/udev/rules.d

if [ ! -f $BR2_EXTERNAL_KEYSTONE_PATH/board/sifive/hifive-unmatched/sdk/led-bootstate-green.service ]; then
    wget -P "${BR2_EXTERNAL_KEYSTONE_PATH}/board/sifive/hifive-unmatched/sdk/led-bootstate-green.service" "https://raw.githubusercontent.com/sifive/freedom-u-sdk/${FUSDK_VER}/recipes-sifive/unmatched-systemd-units/files/led-bootstate-green.service"
fi

cp $BR2_EXTERNAL_KEYSTONE_PATH/board/sifive/hifive-unmatched/sdk/led-bootstate-green.service $TARGET_DIR/usr/lib/systemd/system

if [ ! -f $BR2_EXTERNAL_KEYSTONE_PATH/board/sifive/hifive-unmatched/sdk/led-bootstate-green.timer ]; then
    wget -P "${BR2_EXTERNAL_KEYSTONE_PATH}/board/sifive/hifive-unmatched/sdk/led-bootstate-green.timer" "https://raw.githubusercontent.com/sifive/freedom-u-sdk/${FUSDK_VER}/recipes-sifive/unmatched-systemd-units/files/led-bootstate-green.timer"
fi

cp $BR2_EXTERNAL_KEYSTONE_PATH/board/sifive/hifive-unmatched/sdk/led-bootstate-green.timer $TARGET_DIR/usr/lib/systemd/system
