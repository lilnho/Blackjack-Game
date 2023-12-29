#ifndef INC_UART_H_
#define INC_UART_H_
#include "stm32l4xx_hal.h"

void UART_init(void);
void UART_print_char(char string);
void UART_print(char* word);
void USART_ESC_Code(char* code);
void toString(uint32_t value, char *str, int max_index);
char read_input(void);

#endif /* INC_UART_H_ */
