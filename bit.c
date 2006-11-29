#include <bit.h>

unsigned int set_bit(unsigned int num, unsigned char bit, char value) {
        if(bit < 32) {
                if(value)
                        num |= (1 << bit);
                else
                        num &= ~(1 << bit);
        }
        return num;
}

unsigned int get_bit(unsigned int num, unsigned char bit) {
        if(bit < 32)
                return (num) & (1 << bit);
        else
                return 0;
}

