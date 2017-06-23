//i copied this from tonclib

extern u16 key_curr, key_prev;

#define REG_BASE 0x04000000
#define REG_KEYINPUT	*(volatile u16*)(REG_BASE+0x0130)

#define KEY_A        0x0001
#define KEY_B        0x0002
#define KEY_SELECT   0x0004
#define KEY_START    0x0008
#define KEY_RIGHT    0x0010
#define KEY_LEFT     0x0020
#define KEY_UP       0x0040
#define KEY_DOWN     0x0080
#define KEY_R        0x0100
#define KEY_L        0x0200

#define KEY_MASK     0x03FF

// Polling function
void key_poll() {
    key_prev= key_curr;
    key_curr= ~REG_KEYINPUT & KEY_MASK;
}

// Basic state checks
u32 key_curr_state()         {   return key_curr;          }
u32 key_prev_state()         {   return key_prev;          }
u32 key_is_down(u32 key)     {   return  key_curr & key;   }
u32 key_is_up(u32 key)       {   return ~key_curr & key;   }
u32 key_was_down(u32 key)    {   return  key_prev & key;   }
u32 key_was_up(u32 key)      {   return ~key_prev & key;   }
