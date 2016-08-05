losetup /dev/loop0 && exit 1 || true
image=arch-linux-banana-m1plus-$(date +%Y%m%d).img
rm -f $image
truncate -s 2G $image
losetup /dev/loop0 $image

parted -s /dev/loop0 unit MiB mklabel msdos mkpart primary ext2 -- 32 -1
parted -s /dev/loop0 print


mkfs.ext4 -L root -b 4096 -E stride=4,stripe_width=1024 /dev/loop0p1

mkdir -p arch-root 

mount /dev/loop0p1 arch-root

bsdtar -xpf ArchLinuxARM-armv7-latest.tar.gz -C arch-root
# sed -i "s/ defaults / defaults,noatime /" arch-root/etc/fstab
rm -f arch-root/etc/resolv.conf
cp -f /etc/resolv.conf arch-root/etc/resolv.conf
cat <<EEE >>arch-root/etc/hosts
137.132.155.168 sg2.mirror.archlinuxarm.org
51.254.158.135  archphile.org
EEE
cp /usr/bin/qemu-arm*  arch-root/usr/bin/
cp mirrorlist arch-root/etc/pacman.d/
cp locale.gen  arch-root/etc/

cp boot.cmd boot.scr arch-root/boot
cp sun7i-a20-bananapi-m1-plus.dtb  arch-root/boot/dtbs
cp proxychains-ng-4.11-1-armv7h.pkg.tar.xz uboot-tools-2016.03-1-armv7h.pkg.tar.xz  arch-root/var/cache/pacman/pkg/
cp /etc/proxychains.conf  arch-root/etc/
cp pkgs/*  arch-root/var/cache/pacman/pkg/
cp brcmfmac43362-sdio.txt arch-root/lib/firmware/brcm/
umount arch-root
losetup -D
sudo dd conv=notrunc if=u-boot-sunxi-with-spl.bin of=$image bs=1024 seek=8
