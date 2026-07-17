// 보드에 따라 아래의 값 중 하나만 주석 제거
#define CONFIG_FRONT (1) // 전방 보드일 때
//#define CONFIG_MID (1) // 중앙 보드일 때
//#define CONFIG_REAR (1) // 후방 보드일 때

// 각 보드에 맞는 헤더파일 선언
#ifdef CONFIG_FRONT
#include "front_board.h"
#endif

#ifdef CONFIG_MID
#include "mid_board.h"
#endif

#ifdef CONFIG_REAR 
#include "rear_board.h"
#endif

// 태스크의 스택 사이즈 (임의로 지정한 것, 수정 가능)
#define TASK_USER_STK_SIZE         (128U)
#define TASK_NORMAL_STK_SIZE       (128U)
#define TASK_MEDIUM_STK_SIZE       (256U)
#define TASK_HIGH_STK_SIZE         (512U)

// 마스크
#define 	PWM_DUTY_MASK		(0xFFFF0000)		// PWM duty
#define 	DRIVE_MODE_MASK		(0x0000FF00)		// 주행 모드
#define 	AUTOHOLD_MASK		(0x000000FF)		// 오토 홀드 on/off 여부
#define 	ACCEL_PERCENT_MASK 	(0xFFFF0000)		// 가속 페달 압력 
#define 	TRASONIC_MASK			(0) 	// 외부통신 완료 후 작성
#define	    RED_LIGHT_MASK		(0) 	// 외부통신 완료 후 작성

#define     ID_DATA                     (0U)
#define     ID_ABNORMAL                 (1U)
#define     ID_ULTRA_SONIC              (2U)
#define     ID_RED                      (3U)


#define     P                   (0)
#define     D                   (1)
#define     R                   (2)

#define     DANGER_DISTANCE         (7)
#define     IS_DANGER               (1)
#define     IS_NOT_DANGER           (0)