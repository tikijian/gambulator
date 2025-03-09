#include "../types.h"
#include "../constants.h"
#include "../routines.h"
#include "interrupts.h"
#include "timer.h"
#include "memory.h"
#include "clock.h"
#include "cpu.h"
#include "cpu_helpers.h"

#define IR_VBLANK_VECTOR 0x0040
#define IR_LCD_VECTOR 0x0048
#define IR_TIMER_VECTOR 0x0050
#define IR_SERIAL_VECTOR 0x0058
#define IR_JOYPAD_VECTOR 0x0060

static void set_interrupt_flag(word_t type, int flag_bit) {
    byte_t val = mem_get_value(type);
    val = SET_BIT(val, flag_bit);
    mem_set_value(type, val);
}

static void handle_interrupt(int flag_bit, word_t ir_vector, byte_t ir_flags) {
    cpu_push_pc();
    cpu.PC = ir_vector;
    ir_flags = TOGGLE_BIT(ir_flags, flag_bit);
    mem_write(REG_IR_FLAGS, ir_flags); 
}

void ir_req_timer()
{
    set_interrupt_flag(REG_IR_FLAGS, REG_IR_TIMER_BIT);
}

void ir_handle()
{
    if (!cpu.IME) { // TODO: possible OR HALTED check
        return;
    }

    cpu.halted = 0;
    cpu.IME = 0;

    byte_t ir_requested = mem_read(REG_IR_FLAGS);
    byte_t ir_enables   = mem_read(REG_IR_ENABLE);

    if (CHECK_BIT(ir_enables, REG_IR_VBLANK_BIT) && CHECK_BIT(ir_requested, REG_IR_VBLANK_BIT)) {
        
        handle_interrupt(REG_IR_VBLANK_BIT, IR_VBLANK_VECTOR, ir_requested);
    
    } else if (CHECK_BIT(ir_enables, REG_IR_LCD_BIT) && CHECK_BIT(ir_requested, REG_IR_LCD_BIT)) {
        
        handle_interrupt(REG_IR_LCD_BIT, IR_LCD_VECTOR, ir_requested);
    
    } else if (CHECK_BIT(ir_enables, REG_IR_TIMER_BIT) && CHECK_BIT(ir_requested, REG_IR_TIMER_BIT)) {
        
        handle_interrupt(REG_IR_TIMER_BIT, IR_TIMER_VECTOR, ir_requested);
    
    } else if (CHECK_BIT(ir_enables, REG_IR_JOYPAD_BIT) && CHECK_BIT(ir_requested, REG_IR_JOYPAD_BIT)) {
        
        handle_interrupt(REG_IR_JOYPAD_BIT, IR_JOYPAD_VECTOR, ir_requested);
    
    }
}
