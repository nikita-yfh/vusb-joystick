#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "usbdrv/usbdrv.h"

static uint8_t report [4];
static uint8_t report_out [4];

static void init_joy() {
	DDRC  &= ~0xff;
	PORTC |=  0xff;

	DDRD &= ~0xC8;
	PORTD |= 0xC8;

	DDRB &= ~0x01;
	PORTB |= 0x01;
}

static void read_joy() {
	report [0] = 0;
	report [1] = 0;
	report [2] = 0;
	report [3] = 0;

	if (!(PIND & 0x80)) report [0] = -127; // X
	if (!(PIND & 0x40)) report [0] = +127;

	if (!(PINB & 0x01)) report [1] = -127; // Y
	if (!(PIND & 0x08)) report [1] = +127;
	
	if (!(PINC & 0x01)) report [3] |= 0x04; // SELECT
	if (!(PINC & 0x02)) report [2] |= 0x01; // A
	if (!(PINC & 0x04)) report [2] |= 0x02; // B
	if (!(PINC & 0x08)) report [3] |= 0x08; // START
	if (!(PINC & 0x10)) report [2] |= 0x08; // X
	if (!(PINC & 0x20)) report [2] |= 0x10; // Y
}

PROGMEM const char usbHidReportDescriptor [USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
	0x05, 0x01,     // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,     // USAGE (Game Pad)
	0xa1, 0x01,     // COLLECTION (Application)
	0x09, 0x01,     //   USAGE (Pointer)
	0xa1, 0x00,     //   COLLECTION (Physical)
	0x09, 0x30,     //     USAGE (X)
	0x09, 0x31,     //     USAGE (Y)
	0x15, 0x81,     //   LOGICAL_MINIMUM (-127)
	0x25, 0x7f,     //   LOGICAL_MAXIMUM (127)
	0x75, 0x08,     //   REPORT_SIZE (8)
	0x95, 0x02,     //   REPORT_COUNT (2)
	0x81, 0x02,     //   INPUT (Data,Var,Abs)
	0xc0,           // END_COLLECTION
	0x05, 0x09,     // USAGE_PAGE (Button)
	0x19, 0x01,     //   USAGE_MINIMUM (Button 1)
	0x29, 0x0c,     //   USAGE_MAXIMUM (Button 12)
	0x15, 0x00,     //   LOGICAL_MINIMUM (0)
	0x25, 0x01,     //   LOGICAL_MAXIMUM (1)
	0x75, 0x01,     // REPORT_SIZE (2)
	0x95, 0x10,     // REPORT_COUNT (8)
	0x81, 0x02,     // INPUT (Data,Var,Abs)
	0xc0            // END_COLLECTION
};

uint8_t usbFunctionSetup(uint8_t data [8]) {
	usbRequest_t const* rq = (usbRequest_t const*) data;

	if ( (rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_CLASS )
		return 0;
	
	switch ( rq->bRequest ) {
	case USBRQ_HID_GET_REPORT:
		usbMsgPtr = (usbMsgPtr_t) report_out;
		return sizeof(report_out);
	}
	return 0;
}


void main() {
	usbInit();
	init_joy();
	sei();
	
	while(1) {
		usbPoll();
		if (usbInterruptIsReady()) {
			read_joy();
			if (memcmp(report_out, report, sizeof(report))) {
				memcpy(report_out, report, sizeof(report));
				usbSetInterrupt(report_out, sizeof report_out);
			}
		}
	}
}
