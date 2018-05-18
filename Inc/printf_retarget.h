#ifndef INC_PRINTF_RETARGET_H_

int __io_putchar(int ch);
int _write(int file,char *ptr, int len);

#define INC_PRINTF_RETARGET_H_

#endif /* INC_PRINTF_RETARGET_H_ */
