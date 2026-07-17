#include <main.h>

#include <sal_api.h>
#include <bsp.h>
#include <sdm.h>
#include <bootctrl.h>

#include "defs.h"

static void Main_StartTask
(
    void *                              pArg
);

static void AppTaskCreate
(
    void
);

void cmain (void)
{
    SALRetCode_t            err;
    SALMcuVersionInfo_t     versionInfo = {0,0,0,0};

    (void)SAL_Init();  

    BSP_PreInit(); /* Initialize basic BSP functions */

    BSP_Init(); /* Initialize BSP functions */

    (void)SAL_GetVersion(&versionInfo);
    mcu_printf("\nMCU BSP Version: V%d.%d.%d\n",
           versionInfo.viMajorVersion,
           versionInfo.viMinorVersion,
           versionInfo.viPatchVersion);
           
    (void)SAL_OsInitFuncs();
    
    BCTRL_Init(); // 각 코어들이 부팅되었는지 확인

    (void)SDM_Init(); // SDM(Serial Data Manager)을 제어하는 데 사용

    // create task...
    err = (SALRetCode_t)createTask(); // 보드 별로 선언된 헤더파일 내부의 함수 실행

    if (err == SAL_RET_SUCCESS)
    {
        // start woring os.... never return from this function 
        (void)SAL_OsStart();
    }
}