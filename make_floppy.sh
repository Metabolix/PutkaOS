#!/bin/bash

GRUB="grub"
MKFS="mkfs"
LEVY="$1"
MNT=${LEVY}.mnt
FS="$2"

which ${GRUB} > /dev/null 2>&1
if [ $? -ne 0 ] ; then
	GRUB='/sbin/grub'
	if [ -e "$GRUB" ] ; then
		echo "Grubia ei löydy, tee itse levysi!"
		exit -1
	fi
fi

which ${MKFS} > /dev/null 2>&1
if [ $? -ne 0 ] ; then
	MKFS='/sbin/mkfs'
	if [ -e "$MKFS" ] ; then
		echo "Komentoa mkfs ei löydy, tee itse levysi!"
		exit -1
	fi
fi

if [ -z "$LEVY" ] ; then
	echo "Et valinnut nimeä levylle, luodaan putkaos.img."
	LEVY="putkaos.img"
fi

if [ -z "$FS" ] ; then
	echo "Et valinnut tiedostojärjestelmää, luodaan ext2."
	FS="ext2"
fi

case "$FS" in
	'ext2')
		FLAGS="-F"
		;;
	*)
		FLAGS=""
		;;
esac

echo "Luodaan levy (${LEVY})..."
dd if=/dev/zero of=${LEVY} count=2880 > /dev/null 2>&1
echo "Luodaan tiedostojärjestelmä..."
${MKFS} -t ${FS} ${FLAGS} ${LEVY} > /dev/null 2>&1
if [ $? -ne 0 ] ; then
	echo "Virhe tiedostojärjestelmän luomisessa, lopetetaan."
	exit -1
fi

MNT_EXISTS=0
mkdir ${MNT} || MNT_EXISTS=1
mount -o loop -t ${FS} ${LEVY} ${MNT}
if [ $? -ne 0 ] ; then
	echo "Virhe mountissa, lopetetaan."
	exit -1
fi

mkdir -p ${MNT}/boot/grub/
cp /boot/grub/stage1 ${MNT}/boot/grub/
cp /boot/grub/stage2 ${MNT}/boot/grub/
cp misc/menu.lst ${MNT}/boot/grub/menu.lst
if [ $? -ne 0 ] ; then
	echo "Virhe tiedostojen kopioinnissa, lopetetaan."
	exit -1
fi

${GRUB} --batch --device-map=/dev/null <<EOF
device (fd0) ${LEVY}
root (fd0)
setup (fd0)
quit
EOF

if [ $? -ne 0 ] ; then
	echo "Virhe GRUBin asentamisessa."
	exit -1
fi
umount ${LEVY}
[ "$MNT_EXISTS" -ne 0 ] || rmdir ${MNT}
