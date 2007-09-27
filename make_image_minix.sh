dd if=/dev/zero of=putkaos.img count=2880
/sbin/mkfs.minix putkaos.img
mkdir putkaos
mount -o loop -t minix putkaos.img putkaos
mkdir -p putkaos/boot/grub/
cp /boot/grub/stage1 putkaos/boot/grub/
cp /boot/grub/stage2 putkaos/boot/grub/
echo -e "default 0\ntimeout 5\ntitle=PutkaOS v0.002\nroot (fd0)\nkernel /kernel\nboot" > putkaos/boot/grub/menu.lst
/sbin/grub --batch --device-map=/dev/null <<EOF
device (fd0) putkaos.img
root (fd0)
setup (fd0)
quit
EOF
umount putkaos.img
