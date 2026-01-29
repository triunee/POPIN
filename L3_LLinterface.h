// L3_LLinterface.h - Add RSSI getter
#ifndef L3_LLINTERFACE_H
#define L3_LLINTERFACE_H

#include "mbed.h"
#include "L3_types.h"

extern void (*L3_LLI_dataReqFunc)(uint8_t* msg, uint8_t size, uint8_t destId);

void L3_LLI_dataInd(uint8_t* dataPtr, uint8_t srcId, uint8_t size, int8_t snr, int16_t rssi);
uint8_t* L3_LLI_getMsgPtr();
uint8_t L3_LLI_getSize();
uint8_t L3_LLI_getSrcId();
int16_t L3_LLI_getRssi();  // Add RSSI getter
int8_t L3_LLI_getSnr();     // Add SNR getter
void L3_LLI_setDataReqFunc(void (*funcPtr)(uint8_t*, uint8_t, uint8_t));
void L3_LLI_setReconfigSrcIdReqFunc(void (*funcPtr)(uint8_t));
void L3_LLI_dataCnf(uint8_t res);
void L3_LLI_reconfigSrcIdCnf(uint8_t res);

// ====== Phase 0: 상태 관리 함수 (31개) ======

// 초기화 함수
void L3_LLI_initConfig(uint8_t id);

// Step 0-3: ID/역할 관리 함수 (4개)
uint8_t L3_LLI_getMyId(void);
uint8_t L3_LLI_isAdmin(void);
uint8_t L3_LLI_getCurrentBoothId(void);
void L3_LLI_setCurrentBoothId(uint8_t boothId);

// Step 0-4: 부스 관리 함수 (6개)
Booth_t* L3_LLI_getMyBooth(void);
uint8_t L3_LLI_getRegisteredCount(void);
void L3_LLI_incrementRegisteredCount(void);
uint8_t L3_LLI_isQuietMode(void);
void L3_LLI_toggleQuietMode(void);
uint8_t L3_LLI_getWaitingNumber(void);

// Step 0-5: 사용자 대기열 함수 (4개)
uint8_t L3_LLI_getMyWaitingNumber(void);
void L3_LLI_setMyWaitingNumber(uint8_t number);
uint8_t L3_LLI_getTotalWaitingUsers(void);
void L3_LLI_setTotalWaitingUsers(uint8_t total);

// Step 0-6: 대기열 필터 함수 (4개)
uint8_t L3_LLI_getLastQueuePosition(void);
void L3_LLI_setLastQueuePosition(uint8_t position);
uint8_t L3_LLI_getLastTotalWaiting(void);
void L3_LLI_setLastTotalWaiting(uint8_t total);

// Step 0-7: 스캔 관리 함수 (6개)
BoothScanInfo_t* L3_LLI_getScannedBooths(void);
uint8_t L3_LLI_isScanning(void);
void L3_LLI_setScanning(uint8_t scanning);
uint8_t L3_LLI_getScanResponseCount(void);
void L3_LLI_incrementScanResponse(void);
void L3_LLI_resetScanResponse(void);

// Step 0-8: 채팅 관리 함수 (5개)
uint8_t L3_LLI_isTypingChat(void);
void L3_LLI_setTypingChat(uint8_t typing);
char* L3_LLI_getChatBuffer(void);
uint8_t L3_LLI_getChatIndex(void);
void L3_LLI_setChatIndex(uint8_t index);

#endif // L3_LLINTERFACE_H