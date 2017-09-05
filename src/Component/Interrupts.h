#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#define INTERRUPT_VBLANK    (1 << 0)
#define INTERRUPT_LCDSTAT   (1 << 1)
#define INTERRUPT_TIMER     (1 << 2)
#define INTERRUPT_SERIAL    (1 << 3)
#define INTERRUPT_JOYPAD    (1 << 4)
#define INTERRUPT_ENABLE_REGISTER 0xFFFF
#define INTERRUPT_REQUEST_REGISTER 0xFF0F

#define OFFSET_VBLANK       0x40
#define OFFSET_LCDSTAT      0x48
#define OFFSET_TIMER        0x50
#define OFFSET_SERIAL       0x58
#define OFFSET_JOYPAD       0x60


#endif