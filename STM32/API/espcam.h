#ifndef __ESPCAM_H
#define __ESPCAM_H


void command_state_machine_run(void);
void uart_dma_process(uint8_t *rx_buffer, uint16_t received_length);






#endif
