all: main.c
	avr-gcc -mmcu=atmega8 -DF_CPU=16000000 -Wall -funsigned-char -Os -o main.elf -I. \
		main.c usbdrv/usbdrv.c usbdrv/usbdrvasm.S usbdrv/oddebug.c
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex

flash: all
	avrdude -v -p m8 -c stk500v1 -b 19200 -U main.hex -P /dev/ttyUSB0

clean:
	-rm main.elf main.hex
