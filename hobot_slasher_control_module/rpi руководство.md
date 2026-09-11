# запуск последовательности ros:
1. source ~/ros2_ws/install/setup.bash
2. ros2 launch ydlidar_ros2_driver x3_ydlidar_launch.py params_file:=/home/mes/ros2_ws/src/ydlidar_ros2_driver/params/ydlidar_x3.yaml
3. ros2 run tf2_ros static_transform_publisher 0 0 0 0 0 0 base_link laser
4. ros2 run ros2_laser_scan_matcher laser_scan_matcher --ros-args -p publish_odom:=/odom -p publish_tf:=true
5. ros2 launch slam_toolbox online_sync_launch.py

# команды ros:
source ~/ros2_ws/install/setup.bash - загрузить окружение
ros2 topic list - список топиков (-t с типом -v подробно)
ros2 topic echo --once /имя_топика - прочитать топик разово
ros2 topic hz /имя_топика - частота
ros2 run nav2_map_server map_saver_cli -f ~/my_map --ros-args -p save_map_timeout:=10.0 - сохранить карту


# Активация COM-порта
sudo raspi-config -> interface options -> serial port -> no -> yes

# Запуск точки доступа
сперва нужно поместить файл Hotspot.nmconnection в /etc/NetworkManager/system-connections/
sudo mv Hotspot.nmconnection /etc/NetworkManager/system-connections/
sudo chown root:root /etc/NetworkManager/system-connections/Hotspot.nmconnection
sudo chmod 600 /etc/NetworkManager/system-connections/Hotspot.nmconnection
sudo systemctl restart NetworkManager
sudo nmcli connection reload

# управление подключением
nmcli connection show - список сетей
sudo nmcli connection up <имя сети>
sudo nmcli connection up netplan-wlan0-Bitrace49

# Автозапуск
поместить файл launch_server.service в /etc/systemd/system
1) sudo mv launch_server.service /etc/systemd/system/
2) sudo systemctl enable launch_server

# Копирование системы
1) wget -O image-backup https://raw.githubusercontent.com/raspberrypi/documentation/develop/documentation/asciidoc/computers/os/scripts/image-backup
2) sudo chmod +x image-backup

1) копируем систему с помощью Win32 Disk Imager
2) Используя систему Linux сжимаем образ: sudo pishrink.sh phoenix.img phoenix_compact.img

sudo mount /dev/sda1 /mnt/flash
sudo dd if=/dev/mmcblk0 of=/mnt/flash/phoenix.img bs=4M status=progress conv=fsync
sudo dd if=/dev/sde of=/mnt/d/img/phoenix.img bs=4M status=progress conv=fsync

1) git clone https://github.com/billw2/rpi-clone.git
2) cd rpi-clone
3) sudo cp rpi-clone* /usr/local/sbin/
4) sudo rpi-clone sda -v