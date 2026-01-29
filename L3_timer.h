// L3_timer.h
#ifndef L3_TIMER_H
#define L3_TIMER_H

#include "mbed.h"

// 기존 ARQ/부스선택 타이머 함수
void L3_timer_startTimer();
void L3_timer_stopTimer();
uint8_t L3_timer_getTimerStatus();
void L3_timer_boothSelectionStart();
void L3_timer_boothSelectionStop();

// ====== Phase 1: 타이머 함수 (25개) ======

// Step 1-1: 세션 타이머 (5개)
void L3_timer_sessionStart(void);
void L3_timer_sessionEnd(void);
uint8_t L3_timer_isSessionActive(void);
uint32_t L3_timer_getSessionStartTime(void);
uint32_t L3_timer_getSessionElapsedTime(void);

// Step 1-2: 세션 카운터 (3개)
void L3_timer_sessionCounterTick(void);
void L3_timer_sessionCounterReset(void);
uint32_t L3_timer_getSessionCounter(void);

// Step 1-3: 스캐닝 타이머 (3개)
void L3_timer_scanningTick(void);
void L3_timer_resetScanningTimer(void);
uint32_t L3_timer_getScanningTimer(void);

// Step 1-4: 큐 준비 타이머 (5개)
void L3_timer_queueReadyStart(uint8_t userId);
void L3_timer_queueReadyStop(void);
uint8_t L3_timer_isQueueReadyActive(void);
uint8_t L3_timer_getPendingUserId(void);
uint32_t L3_timer_getQueueReadyElapsedTime(void);

// Step 1-5: 연결 타이머 (6개)
void L3_timer_connectStart(void);
void L3_timer_connectEnd(void);
uint8_t L3_timer_isWaitingForBoothInfo(void);
uint8_t L3_timer_getConnectRetryCount(void);
void L3_timer_incrementConnectRetry(void);
void L3_timer_resetConnectRetry(void);

// Step 1-6: 등록 타이머 (6개)
void L3_timer_registerStart(void);
void L3_timer_registerEnd(void);
uint8_t L3_timer_isWaitingForRegisterResponse(void);
uint8_t L3_timer_getRegisterRetryCount(void);
void L3_timer_incrementRegisterRetry(void);
void L3_timer_resetRegisterRetry(void);

#endif // L3_TIMER_H   