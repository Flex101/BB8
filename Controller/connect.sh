sudo fuser -k /dev/rfcomm0
sudo fuser -k /dev/rfcomm1

sudo rfcomm bind rfcomm0 28:CD:C1:08:DE:50 1
sudo rfcomm bind rfcomm1 28:CD:C1:0F:07:95 1