#include "printf.h"
#include "spinlock.h"
#include "interrupt.h"

uint8_t spinlock;

const char hex_codes[] = {
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'A',
    'B',
    'C',
    'D',
    'E',
    'F'
};

static void kprintf_putchar(char c){
    console_putchar(c);
}

static void kprintf_putstr_len(char* str, int len){
    for(int i = 0; i < len; i++){
        console_putchar(str[i]);
    }
}

static void kprintf_putstr(char* str){
    console_putstr(str);
}

static void kprintf_puthexstr(int i){
    for (int j = 0; j < 8; j++) {
        kprintf_putchar(hex_codes[(i >> (28 - j * 4)) & 0xF]);
    }
}

static void kprintf_putptr(void *ptr){
    unsigned long value = (unsigned long)ptr;
    int digits = (int)(sizeof(void *) * 2);

    kprintf_putstr("0x");
    for (int j = digits - 1; j >= 0; --j) {
        kprintf_putchar(hex_codes[(value >> (j * 4)) & 0xF]);
    }
}

static void kprintf_putuint(unsigned int i){
    int f = 1;

    while((i / f) >= 10){
        f *= 10;
    }

    int r;
    while(f > 0){
        r = i / f;
        kprintf_putchar('0' + r);
        i = i - (r * f);
        f /= 10;
    }
}

static void kprintf_putint(int i){
    int f = 1;

    if(i < 0){
        kprintf_putchar('-');
        i *= -1;
    }

    while((i / f) >= 10){
        f *= 10;
    }

    int r;
    while(f > 0){
        r = i / f;
        kprintf_putchar('0' + r);
        i = i - (r * f);
        f /= 10;
    }
}

static void kprintf_putlong(long long i){
    int f = 1;

    if(i < 0){
        kprintf_putchar('-');
        i *= -1;
    }

    while((i / f) >= 10){
        f *= 10;
    }

    int r;
    while(f > 0){
        r = i / f;
        kprintf_putchar('0' + r);
        i = i - (r * f);
        f /= 10;
    }
}

static void kprintf_putfloat(float n, int len, int precision){
    int f = 1;

    if(n < 0){
        kprintf_putchar('-');
        n *= -1;
    }

    while((n / f) >= 10){
        f *= 10;
    }

    int r;
    while(f > 0){
        r = n / f;
        kprintf_putchar('0' + r);
        n = n - (r * f);
        f /= 10;
    }

    float frac_part = n - (float)(int)n;
    if(frac_part > 0){
        kprintf_putchar('.');
        for (int j = 0; j < 2; ++j) {
            frac_part *= 10;
            int digit = (int)frac_part;
            kprintf_putchar('0' + digit);
            frac_part -= digit;
        }
    }
}

int kprintf(const char* str, ...){

    cli();
    lock(&spinlock);

    va_list args;

    uint32_t u;
	int32_t i;
    int64_t l;
    float f;
	char *string;
    void *ptr;
    int strlen = -1;
    char len_vararg = 0;

	va_start(args, str);

    while(*str){

        if(*str != '%'){
            kprintf_putchar(*str);
        }
        else{
            top:
            str++;
            switch(*str){
                case 'd':
                    i = va_arg(args, int);
                    kprintf_putint(i);
                    break;
                case 'l':
                    l = va_arg(args, long long);
                    kprintf_putlong(l);
                    break;
                case 'u':
                    u = va_arg(args, unsigned int);
                    kprintf_putuint(u);
                    break;
                case 'f':
                    f = va_arg(args, double);

                    int len = -1;
                    int precision = -1;
                    if(is_int(str + 1)){
                        
                        if(str + 2 == '.' && is_int(str + 3)){
                        
                        }
                    }
                    else{
                        if(str + 1 == '.' && is_int(str + 2)){

                        }
                    }

                    kprintf_putfloat(f, -1, -1);
                    break;
                case 'x':
                    i = va_arg(args, int);
                    kprintf_puthexstr(i);
                    break;
                case 'p':
                    ptr = va_arg(args, void*);
                    kprintf_putptr(ptr);
                    break;
                case 's':
                    string = va_arg(args, char*);
                    if (strlen >= 0) {
                        kprintf_putstr_len(string, strlen);
                    } else {
                        if (len_vararg) {
                            strlen = va_arg(args, int);
                            kprintf_putstr_len(string, strlen);
                        } else 
                            kprintf_putstr(string);
                    }
                    strlen = -1;
                    len_vararg = 0;
                    break;
                case 'c':
                    u = va_arg(args, unsigned int);
                    kprintf_putchar(u);
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    if (strlen < 0)
                        strlen = *str - '0';
                    else
                        strlen = strlen * 10 + (*str - '0');
                    goto top;
                case '.':
                    if (*(str + 1) == '*') {
                        len_vararg = 1;
                        str++;
                        goto top;
                    }
                default:
                    kprintf_putchar(*str);
                    break;
            }
        }

        str++;
    }

    unlock(&spinlock);
    sti();
}