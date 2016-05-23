#!/bin/bash

wget -O sun7i-a20.dtsi.orig https://raw.githubusercontent.com/torvalds/linux/master/arch/arm/boot/dts/sun7i-a20.dtsi
wget https://raw.githubusercontent.com/torvalds/linux/master/arch/arm/boot/dts/skeleton.dtsi
wget https://raw.githubusercontent.com/torvalds/linux/master/arch/arm/boot/dts/sunxi-common-regulators.dtsi
