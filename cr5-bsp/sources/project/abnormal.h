#ifndef ABNORMAL_H
#define ABNORMAL_H

#define ABNORMAL_BASE_TIME      (250)
#define ABNORMAL_DELAY          (50)

#define MAX_PRESS               (127)

#define BUFFER_SIZE             (ABNORMAL_BASE_TIME / ABNORMAL_DELAY)

typedef struct CircularQueue{
    unsigned char buffer[BUFFER_SIZE];  // 정적 크기의 배열
    int head;                // 큐의 시작 (읽기 위치)
    int tail;                // 큐의 끝 (쓰기 위치)
    int maxSize;             // 큐의 최대 크기
} CircularQueue;

extern CircularQueue gCircularQueue;

int check_start_abnormal
(
    unsigned int pressAccel,
    unsigned int pwmDuty,
    unsigned int isRed,
    unsigned char isAutohold,
    unsigned char driveMode,
    unsigned int isDanger
);

int check_finish_abnormal
(
    unsigned int pressBreak
);

void init_queue(void);
void add_queue(unsigned char value);
int has_zero(void);
int has_under_value(unsigned char value);

#endif // ABNORMAL_H