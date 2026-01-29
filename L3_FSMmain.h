// L3_FSMmain.h
#ifndef L3_FSMMAIN_H
#define L3_FSMMAIN_H

#include "mbed.h"
#include "L3_types.h"

// FSM 초기화 및 실행 함수
void L3_initFSM(uint8_t id);
void L3_FSMrun(void);

// ====== Phase 2: 부스 스캔 함수 (4개) ======
void L3_initBoothScanList(BoothScanInfo_t* booths, uint8_t maxBooths);
void L3_updateBoothScanInfo(BoothScanInfo_t* booths, uint8_t maxBooths,
                            uint8_t boothId, int16_t rssi,
                            uint8_t currentCount, uint8_t capacity,
                            uint8_t waitingCount);
uint8_t L3_selectOptimalBooth(BoothScanInfo_t* booths, uint8_t maxBooths);
void L3_displayScannedBooths(BoothScanInfo_t* booths, uint8_t maxBooths);

// ====== Phase 5: 범용 헬퍼 함수 (4개) ======
uint8_t L3_checkUserInList(User_t* list, uint8_t listSize, uint8_t userId);
uint8_t L3_addUserToList(User_t* list, uint8_t* count, uint8_t userId);
void L3_removeUserFromList(User_t* list, uint8_t* count, uint8_t userId);
int L3_getUserWaitingPosition(User_t* waitingQueue, int waitingCount, int srcId);

#endif // L3_FSMMAIN_H
