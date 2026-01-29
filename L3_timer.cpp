#include "mbed.h"
#include "L3_FSMevent.h"
#include "protocol_parameters.h"

// ARQ 재전송 타이머
static Timeout timer;
static uint8_t timerStatus = 0;

// 부스 선택 타이머
static Timeout boothSelectionTimer;
static uint8_t boothSelectionTimerStatus = 0;

// ====== Phase 1: 타이머 변수 (L3_FSMmain.cpp에서 이동) ======

// Step 1-1: 세션 타이머 변수
static uint32_t sessionStartTime = 0;
static uint8_t isSessionActive = 0;

// Step 1-2: 세션 카운터 변수
static uint32_t sessionTimerCounter = 0;

// Step 1-3: 스캐닝 타이머 변수
static uint32_t scanningTimer = 0;

// Step 1-4: 큐 준비 타이머 변수
static uint8_t pendingUserId = 0;
static uint32_t queueReadyStartTime = 0;
static uint8_t isQueueReadyTimerActive = 0;

// Step 1-5: 연결 타이머 변수
static uint8_t connectRetryCount = 0;
static uint32_t connectRequestTime = 0;
static uint8_t isWaitingForBoothInfo = 0;

// Step 1-6: 등록 타이머 변수
static uint8_t registerRetryCount = 0;
static uint32_t registerRequestTime = 0;
static uint8_t isWaitingForRegisterResponse = 0;


// 타이머 만료 핸들러: ARQ 타임아웃
void L3_timer_timeoutHandler(void) 
{
    timerStatus = 0;
    //L3_event_setEventFlag(L3_event_arqTimeout);  // 필요 시 활성화
}

// 타이머 만료 핸들러: 부스 선택 타임아웃 (신규)
void L3_timer_boothSelectionHandler(void)
{
    boothSelectionTimerStatus = 0;
    L3_event_setEventFlag(L3_event_boothSelectionTimeout);
}

// ARQ 재전송 타이머 시작
void L3_timer_startTimer()
{
    uint8_t waitTime = 1;  // 1초 대기
    timer.attach(L3_timer_timeoutHandler, waitTime);
    timerStatus = 1;
}

// ARQ 재전송 타이머 중지
void L3_timer_stopTimer()
{
    timer.detach();
    timerStatus = 0;
}

// ARQ 재전송 타이머 상태 조회
uint8_t L3_timer_getTimerStatus()
{
    return timerStatus;
}

// 부스 선택 타이머 시작
void L3_timer_boothSelectionStart()
{
    boothSelectionTimer.attach(L3_timer_boothSelectionHandler, 10.0);  // 10초 후 타임아웃
    boothSelectionTimerStatus = 1;
}

// 부스 선택 타이머 중지
void L3_timer_boothSelectionStop()
{
    boothSelectionTimer.detach();
    boothSelectionTimerStatus = 0;
}

// ====== Phase 1: 타이머 함수 구현 (25개) ======

// Step 1-1: 세션 타이머 (5개)
void L3_timer_sessionStart(void)
{
    sessionStartTime = us_ticker_read() / 1000;  // ms 단위
    isSessionActive = 1;
}

void L3_timer_sessionEnd(void)
{
    isSessionActive = 0;
    sessionStartTime = 0;
}

uint8_t L3_timer_isSessionActive(void)
{
    return isSessionActive;
}

uint32_t L3_timer_getSessionStartTime(void)
{
    return sessionStartTime;
}

uint32_t L3_timer_getSessionElapsedTime(void)
{
    if (!isSessionActive) return 0;
    return (us_ticker_read() / 1000) - sessionStartTime;
}

// Step 1-2: 세션 카운터 (3개)
void L3_timer_sessionCounterTick(void)
{
    sessionTimerCounter++;
}

void L3_timer_sessionCounterReset(void)
{
    sessionTimerCounter = 0;
}

uint32_t L3_timer_getSessionCounter(void)
{
    return sessionTimerCounter;
}

// Step 1-3: 스캐닝 타이머 (3개)
void L3_timer_scanningTick(void)
{
    scanningTimer++;
}

void L3_timer_resetScanningTimer(void)
{
    scanningTimer = 0;
}

uint32_t L3_timer_getScanningTimer(void)
{
    return scanningTimer;
}

// Step 1-4: 큐 준비 타이머 (5개)
void L3_timer_queueReadyStart(uint8_t userId)
{
    pendingUserId = userId;
    queueReadyStartTime = us_ticker_read() / 1000;
    isQueueReadyTimerActive = 1;
}

void L3_timer_queueReadyStop(void)
{
    isQueueReadyTimerActive = 0;
    pendingUserId = 0;
}

uint8_t L3_timer_isQueueReadyActive(void)
{
    return isQueueReadyTimerActive;
}

uint8_t L3_timer_getPendingUserId(void)
{
    return pendingUserId;
}

uint32_t L3_timer_getQueueReadyElapsedTime(void)
{
    if (!isQueueReadyTimerActive) return 0;
    return (us_ticker_read() / 1000) - queueReadyStartTime;
}

// Step 1-5: 연결 타이머 (6개)
void L3_timer_connectStart(void)
{
    connectRequestTime = us_ticker_read() / 1000;
    isWaitingForBoothInfo = 1;
}

void L3_timer_connectEnd(void)
{
    isWaitingForBoothInfo = 0;
    connectRetryCount = 0;
}

uint8_t L3_timer_isWaitingForBoothInfo(void)
{
    return isWaitingForBoothInfo;
}

uint8_t L3_timer_getConnectRetryCount(void)
{
    return connectRetryCount;
}

void L3_timer_incrementConnectRetry(void)
{
    connectRetryCount++;
}

void L3_timer_resetConnectRetry(void)
{
    connectRetryCount = 0;
}

// Step 1-6: 등록 타이머 (6개)
void L3_timer_registerStart(void)
{
    registerRequestTime = us_ticker_read() / 1000;
    isWaitingForRegisterResponse = 1;
}

void L3_timer_registerEnd(void)
{
    isWaitingForRegisterResponse = 0;
    registerRetryCount = 0;
}

uint8_t L3_timer_isWaitingForRegisterResponse(void)
{
    return isWaitingForRegisterResponse;
}

uint8_t L3_timer_getRegisterRetryCount(void)
{
    return registerRetryCount;
}

void L3_timer_incrementRegisterRetry(void)
{
    registerRetryCount++;
}

void L3_timer_resetRegisterRetry(void)
{
    registerRetryCount = 0;
}
