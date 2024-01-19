# DALI Driver for Raspberry

1. sudo rpi-update
2. sudo reboot
3. sudo apt-get update && apt-get upgrade
4. sudo apt install git bc bison flex libssl-dev
5. sudo wget https://raw.githubusercontent.com/RPi-Distro/rpi-source/master/rpi-source -O /usr/local/bin/rpi-source && sudo chmod +x /usr/local/bin/rpi-source && /usr/local/bin/rpi-source -q --tag-update
6. rpi-source
7. git clone https://github.com/alice-trade/DALI-driver
8. cd DALI-driver
9. make
10. make util
11. insmod DALI_driver.ko


./dali_init - сканирование dali-устройств на шине, назначение им коротких номеров и формирование файла config.json.
./dali_init readonly - сканирование dali-устройств на шине, считывание присвоенных коротких номеров и формирование файла config.json.

dali_send - утилита посылает команду на шину DALI, формат ./dali_send <команда> где команда это 3-байтовое число в шестнадцатиричной системе (6 HEX символов), первый байт это любое число от 0 до ff представляющий собой сиквенс запроса, и два остальных DALI-команда. В случае если команда подразумевает ответ, то ответ будет с этим же сиквенсом. Сиквенс должен быть в диапазоне от 0x00..0xfe.

Пример:

./dali_send 230040 - команда с сиквенсом 0x23 прямой установки уровня яркости для устройства с номером 0 и уровнем яркости 0x40. Ответ: ответа на эту команду не предусмотрено.

./dali_send 2401A0 - команда с сиквенсом 0x24 получение текущего уровня яркости от устройства с номером 0. Ответ: 240040 - уровень якрости 0x40, второй байт не используется и равен у ответов всегда нулю.

./dali_cmd используется для отправки команд на устройство или группу в человекочитаемом виде, а так же для получения актуального значения яркости

./dali_cmd {device:group} {Short ID} status - получить актуальное значение яркости

./dali_cmd {device:group} {Short ID} off - выключить устройство млм группу

./dali_cmd {device:group} {Short ID} dim 100 - установить яркость в процентах

./dali_cmd {device:group} {Short ID} dimraw 254 - установить яркость в числе ШИМ

./dali_config - используется для анализа шины DALI и установки параметров для групп и устройств

./dali_config add {Group} {DeviceShortID} - Добавить устройство в группу

./dali_config remove {Group} {DeviceShortID} - Убрать устройство из группы

./dali_config all - добавить все устройства в группах из файла config.json

./dali_config device {set_minval:set_maxval} {Value} - установить минимальное/максимальное значение устройства

./dali_config group {set_minval:set_maxval} {Value} - установить минимальное/максимальное значение группы

./dali_config info device {DeviceShortID} - информация об устройстве

./dali_config info group {Group} - информация о группе
