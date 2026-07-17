#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "project_segment.h"


void tm1637_delay_us(int delay)
{
  while (delay > 0)
  {
    delay--;
  }
}

// 파일에 값을 쓰는 함수
void write_to_file(const char *path, const char *value) {

    int fd = open(path, O_WRONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    if (write(fd, value, strlen(value)) == -1) {
        perror("Error writing to file");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}

void segment_pin_init(void)
{
    // GPIO 핀 활성화
    write_to_file(GPIO_EXPORT_PATH, TM1637_CLK_PIN);
    write_to_file(GPIO_EXPORT_PATH, TM1637_DIO_PIN);
    
    // GPIO 방향 설정 (출력)
    write_to_file(TM1637_CLK_DIRECTION_PATH, "out");
    write_to_file(TM1637_DIO_DIRECTION_PATH, "out");
}

void segment_pin_deinit(void)
{
    // GPIO 핀 비활성화
    write_to_file(GPIO_UNEXPORT_PATH, TM1637_CLK_PIN);
    write_to_file(GPIO_UNEXPORT_PATH, TM1637_DIO_PIN);
}

void tm1637_start(const int dio_fd)
{
    if (write(dio_fd, "0", 1) == -1) {
        perror("Error writing to file");
        close(dio_fd);
        exit(EXIT_FAILURE);
    }
    tm1637_delay_us(TM1637_BIT_DELAY);
}

void tm1637_stop(const int dio_fd, const int clk_fd) 
{
    if (write(dio_fd, "0", 1) == -1) {
        perror("Error writing to file");
        close(dio_fd);
        exit(EXIT_FAILURE);
    }
    tm1637_delay_us(TM1637_BIT_DELAY);

    if (write(clk_fd, "1", 1) == -1) {
        perror("Error writing to file");
        close(clk_fd);
        exit(EXIT_FAILURE);
    }
    tm1637_delay_us(TM1637_BIT_DELAY);

    if (write(dio_fd, "1", 1) == -1) {
        perror("Error writing to file");
        close(dio_fd);
        exit(EXIT_FAILURE);
    }
    tm1637_delay_us(TM1637_BIT_DELAY);
}

void tm1637_write_byte(const int dio_fd, const int clk_fd, char data)
{
    //  write 8 bit data
     for (char i = 0; i < 8; ++i)
     {
         if (write(clk_fd, "0", 1) == -1) {
            perror("Error writing to file");
            close(clk_fd);
            exit(EXIT_FAILURE);
        }
        tm1637_delay_us(TM1637_BIT_DELAY);
    
        if (data & 0x01){
          if (write(dio_fd, "1", 1) == -1) {
                perror("Error writing to file");
                close(dio_fd);
                exit(EXIT_FAILURE);
            }
        }
        else{
            if (write(dio_fd, "0", 1) == -1) {
                perror("Error writing to file");
                close(dio_fd);
                exit(EXIT_FAILURE);
            }
        }
        tm1637_delay_us(TM1637_BIT_DELAY);

        if (write(clk_fd, "1", 1) == -1) {
            perror("Error writing to file");
            close(clk_fd);
            exit(EXIT_FAILURE);
        }
        tm1637_delay_us(TM1637_BIT_DELAY);

        data = data >> 1;
    }

    // wait for acknowledge
    if (write(clk_fd, "0", 1) == -1) {
        perror("Error writing to file");
        close(clk_fd);
        exit(EXIT_FAILURE);
    }

    if (write(dio_fd, "1", 1) == -1) {
        perror("Error writing to file");
        close(dio_fd);
        exit(EXIT_FAILURE);
    }
    tm1637_delay_us(TM1637_BIT_DELAY);

    if (write(clk_fd, "1", 1) == -1) {
        perror("Error writing to file");
        close(clk_fd);
        exit(EXIT_FAILURE);
    }
    tm1637_delay_us(TM1637_BIT_DELAY);

    char ack;
    read(dio_fd, &ack, 1);
    if (ack == '0'){
        if (write(dio_fd, "0", 1) == -1) {
            perror("Error writing to file");
            close(dio_fd);
            exit(EXIT_FAILURE);
        }
        tm1637_delay_us(TM1637_BIT_DELAY);
    }
    
    if (write(clk_fd, "0", 1) == -1) {
        perror("Error writing to file");
        close(clk_fd);
        exit(EXIT_FAILURE);
    }
    tm1637_delay_us(TM1637_BIT_DELAY);
}

void tm1637_write_raw(const char *data)
{
    int clk_fd = open(TM1637_CLK_VALUE_PATH, O_WRONLY);
    int dio_fd = open(TM1637_DIO_VALUE_PATH, O_RDWR);

    // write COMM1
    tm1637_start(dio_fd);
    tm1637_write_byte(dio_fd, clk_fd, TM1637_COMM1);
    tm1637_stop(dio_fd, clk_fd);

    // write COMM2 + first digit address
    tm1637_start(dio_fd);
    tm1637_write_byte(dio_fd, clk_fd, TM1637_COMM2);

    // write the data bytes
    for (int i = 0; i < 4; ++i){
        tm1637_write_byte(dio_fd, clk_fd, data[i]);
    }
    tm1637_stop(dio_fd, clk_fd);

    // write COMM3 + brightness
    tm1637_start(dio_fd);
    tm1637_write_byte(dio_fd, clk_fd, TM1637_COMM3);
    tm1637_stop(dio_fd, clk_fd);

    close(dio_fd);
    close(clk_fd);
}

void get_accel_data(int accel_press, char* segment_data)
{
    int     tmp = 0;

    segment_data[0] = 0;
    tmp = accel_press % 1000;
    segment_data[1] = tm1637_data[tmp/100];
    tmp = accel_press % 100;
    segment_data[2] = tm1637_data[tmp/10];
    tmp = accel_press % 10;
    segment_data[3] = tm1637_data[tmp];
}

void display_segment(int percent){
    char segment_data[4] = {0};

    segment_pin_init();

    get_accel_data(percent, &segment_data[0]);
    tm1637_write_raw(&segment_data[0]);

    segment_pin_deinit();
}

int main(void)
{
    int test_arr[10] = {100, 42, 15, 75, 65, 59, 256, 24, 95, 37};

    for (int i = 0; i < 10; ++i) { // 10번 반복 (필요에 따라 무한 반복으로 수정 가능)
        display_segment(test_arr[i]);
        
        // 1초 대기
        sleep(1);
    }

    return 0;
}
