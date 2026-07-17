#ifndef PROJECT_SEGMENT
#define PROJECT_SEGMENT

#define TM1637_CLK_PIN "84"
#define TM1637_DIO_PIN "85"
#define GPIO_PATH "/sys/class/gpio"
#define GPIO_EXPORT_PATH GPIO_PATH "/export"
#define GPIO_UNEXPORT_PATH GPIO_PATH "/unexport"
#define TM1637_CLK_DIRECTION_PATH GPIO_PATH "/gpio" TM1637_CLK_PIN "/direction"
#define TM1637_CLK_VALUE_PATH GPIO_PATH "/gpio" TM1637_CLK_PIN "/value"
#define TM1637_DIO_DIRECTION_PATH GPIO_PATH "/gpio" TM1637_DIO_PIN "/direction"
#define TM1637_DIO_VALUE_PATH GPIO_PATH "/gpio" TM1637_DIO_PIN "/value"

#define TM1637_BIT_DELAY    (20)
#define TM1637_COMM1    0x40    // data set
#define TM1637_COMM2    0xC0    // IP address set
#define TM1637_COMM3    0x8F    // display control command set

void display_segment(void);

const char tm1637_data[] =
  {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f}; // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9

#endif