cmd_/home/pi/DALI-driver/Module.symvers := sed 's/\.ko$$/\.o/' /home/pi/DALI-driver/modules.order | scripts/mod/modpost -m -a  -o /home/pi/DALI-driver/Module.symvers -e -i Module.symvers   -T -
