#include "abnormal.h"
// #include <sal_internal.h>

#define TRUE            (1)
#define FALSE           (0)

#define IDLE_PWM        (127)

CircularQueue gCircularQueue;

static int check_start_abnormal_with_autohold
(
    unsigned int pressAccel, 
    unsigned int pwmDuty,
    unsigned int isRed,
    unsigned int driveMode,
    unsigned int isDanger
)
{
    if(isDanger || ((driveMode == 1) && isRed)){
        if(pressAccel > 0 && has_zero()){
            return TRUE;
        }
    }

    return FALSE;
}

static int check_start_abnormal_without_autohold
(
    unsigned int pressAccel, 
    unsigned int pwmDuty
)
{
    if(pressAccel >= MAX_PRESS && has_under_value(IDLE_PWM)){
        return TRUE;
    }

    return FALSE;
}

int check_start_abnormal
(
    unsigned int pressAccel, 
    unsigned int pwmDuty, 
    unsigned int isRed, 
    unsigned char isAutohold,
    unsigned char driveMode,
    unsigned int isDanger
)
{
    if(isAutohold){
        return check_start_abnormal_with_autohold(pressAccel, pwmDuty, isRed, driveMode, isDanger);
        
    }
    else if(isDanger){
        return check_start_abnormal_without_autohold(pressAccel, pwmDuty);
    }

    return FALSE;
}


int check_finish_abnormal
(
    unsigned int pressBreak
)
{
    return (pressBreak >= MAX_PRESS);
}

// -------------------------------------------------------
// 원형 큐 초기화 함수
void init_queue(void) {
    gCircularQueue.head = 0;
    gCircularQueue.tail = 0;
    gCircularQueue.maxSize = BUFFER_SIZE;

    // 큐를 0으로 초기화
    for (int i = 0; i < BUFFER_SIZE; i++) {
        gCircularQueue.buffer[i] = 0;
    }
}

// 원형 큐에 값 추가 함수
void add_queue(unsigned char value) {
    // 값 추가
    gCircularQueue.buffer[gCircularQueue.tail] = value;

    // Tail 포인터 이동
    gCircularQueue.tail = (gCircularQueue.tail + 1) % gCircularQueue.maxSize;

    // 큐가 가득 찬 경우 Head를 Tail과 동일하게 이동하여 가장 오래된 값을 덮어씀
    if (gCircularQueue.tail == gCircularQueue.head) {
        gCircularQueue.head = (gCircularQueue.head + 1) % gCircularQueue.maxSize;
    }
}

// 원형 큐에서 tail부터 head까지 탐색하며 0을 발견하면 true 반환
int has_zero(void) {
    int index;

    // 큐가 비어있는 경우 false 반환
    if (gCircularQueue.head == gCircularQueue.tail) {
        return FALSE;  // 큐가 비어있음
    }

    index = (gCircularQueue.tail - 1 + gCircularQueue.maxSize) % gCircularQueue.maxSize;

    // tail부터 head까지 역방향으로 탐색
    while (index != gCircularQueue.head) {
        if (gCircularQueue.buffer[index] == 0) {
            return TRUE;  // 0을 발견하면 true 반환
        }
        index = (index - 1 + gCircularQueue.maxSize) % gCircularQueue.maxSize;  // 이전 위치로 이동
    }

    // head 위치도 검사
    if (gCircularQueue.buffer[index] == 0) {
        return TRUE;  // 0을 발견하면 true 반환
    }

    return FALSE;  // 0이 발견되지 않음
}

// 원형 큐에서 tail부터 head까지 탐색
int has_under_value(unsigned char value) {
    int index;

    // 큐가 비어있는 경우 false 반환
    if (gCircularQueue.head == gCircularQueue.tail) {
        return FALSE;  // 큐가 비어있음
    }

    index = (gCircularQueue.tail - 1 + gCircularQueue.maxSize) % gCircularQueue.maxSize;

    // tail부터 head까지 역방향으로 탐색
    while (index != gCircularQueue.head) {
        if (gCircularQueue.buffer[index] <= value) {
            return TRUE;  
        }
        index = (index - 1 + gCircularQueue.maxSize) % gCircularQueue.maxSize;  // 이전 위치로 이동
    }

    // head 위치도 검사
    if (gCircularQueue.buffer[index] <= value) {
        return TRUE;  
    }

    return FALSE;  // 0이 발견되지 않음
}