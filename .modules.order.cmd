cmd_/home/pi/DALI-driver/modules.order := {   echo /home/pi/DALI-driver/DALI_driver.ko; :; } | awk '!x[$$0]++' - > /home/pi/DALI-driver/modules.order
