#include "mbed.h"
#include "L3_FSMevent.h"
#include "L3_msg.h"
#include "protocol_parameters.h"
#include "time.h"
#include "L3_types.h"

static uint8_t rcvdMsg[L3_MAXDATASIZE];
static uint8_t rcvdSize;
static int16_t rcvdRssi;
static int8_t rcvdSnr;
static uint8_t rcvdSrcId;

// ====== Phase 0: 상태 변수 (L3_FSMmain.cpp에서 이동) ======

// 기본 설정
static uint8_t myId;
static uint8_t currentBoothId = 0;
static uint8_t isAdmin = 0;

// 부스 정보 (관리자)
static Booth_t myBooth;
static uint8_t registeredCount = 0;
static uint8_t quietMode = 0;
static uint8_t waitingNumber = 0;

// 사용자 정보
static uint8_t myWaitingNumber = 0;
static uint8_t totalWaitingUsers = 0;

// 스캔 관리
static BoothScanInfo_t scannedBooths[MAX_BOOTHS];
static uint8_t isScanning = 0;
static uint8_t scanResponseCount = 0;

// 대기열 중복 필터
static uint8_t lastQueuePosition = 0;
static uint8_t lastTotalWaiting = 0;

// 채팅 관리
static uint8_t isTypingChat = 0;
static char chatBuffer[101];
static uint8_t chatIndex = 0;

//Downward primitives
//TX function
void (*L3_LLI_dataReqFunc)(uint8_t* msg, uint8_t size, uint8_t destId);
void (*L3_LLI_reconfigSrcIdReqFunc)(uint8_t myId);

//interface event : DATA_IND, RX data has arrived
void L3_LLI_dataInd(uint8_t* dataPtr, uint8_t srcId, uint8_t size, int8_t snr, int16_t rssi)
{
    debug_if(DBGMSG_L3, "\n[L3] --> DATA IND : src:%i, size:%i, data[0]:%i, RSSI:%i, SNR:%i\n", 
             srcId, size, dataPtr[0], rssi, snr);

    memcpy(rcvdMsg, dataPtr, size*sizeof(uint8_t));
    rcvdSize = size;
    rcvdSnr = snr;
    rcvdRssi = rssi;
    rcvdSrcId = srcId;  // Store source ID

    L3_event_setEventFlag(L3_event_msgRcvd);
}

void L3_LLI_dataCnf(uint8_t res)
{
    debug_if(DBGMSG_L3, "\n --> DATA CNF : res : %i\n", res);
    L3_event_setEventFlag(L3_event_dataSendCnf);
}

void L3_LLI_reconfigSrcIdCnf(uint8_t res)
{
    debug_if(DBGMSG_L3, "\n --> RECONFIG SRCID CNF : res : %i\n", res);
    L3_event_setEventFlag(L3_event_recfgSrcIdCnf);
}

uint8_t* L3_LLI_getMsgPtr()
{
    return rcvdMsg;
}

uint8_t L3_LLI_getSize()
{
    return rcvdSize;
}

uint8_t L3_LLI_getSrcId()
{
    return rcvdSrcId;
}

int16_t L3_LLI_getRssi()
{
    return rcvdRssi;
}

int8_t L3_LLI_getSnr()
{
    return rcvdSnr;
}

void L3_LLI_setDataReqFunc(void (*funcPtr)(uint8_t*, uint8_t, uint8_t))
{
    L3_LLI_dataReqFunc = funcPtr;
}

void L3_LLI_setReconfigSrcIdReqFunc(void (*funcPtr)(uint8_t))
{
    L3_LLI_reconfigSrcIdReqFunc = funcPtr;
}

// ====== Phase 0: 상태 관리 함수 구현 (31개) ======

// 초기화 함수
void L3_LLI_initConfig(uint8_t id)
{
    myId = id;
    isAdmin = (id >= ADMIN_ID_START && id <= ADMIN_ID_END) ? 1 : 0;
    currentBoothId = 0;
}

// Step 0-3: ID/역할 관리 함수 (4개)
uint8_t L3_LLI_getMyId(void)
{
    return myId;
}

uint8_t L3_LLI_isAdmin(void)
{
    return isAdmin;
}

uint8_t L3_LLI_getCurrentBoothId(void)
{
    return currentBoothId;
}

void L3_LLI_setCurrentBoothId(uint8_t boothId)
{
    currentBoothId = boothId;
}

// Step 0-4: 부스 관리 함수 (6개)
Booth_t* L3_LLI_getMyBooth(void)
{
    return &myBooth;
}

uint8_t L3_LLI_getRegisteredCount(void)
{
    return registeredCount;
}

void L3_LLI_incrementRegisteredCount(void)
{
    registeredCount++;
}

uint8_t L3_LLI_isQuietMode(void)
{
    return quietMode;
}

void L3_LLI_toggleQuietMode(void)
{
    quietMode = !quietMode;
}

uint8_t L3_LLI_getWaitingNumber(void)
{
    return waitingNumber;
}

// Step 0-5: 사용자 대기열 함수 (4개)
uint8_t L3_LLI_getMyWaitingNumber(void)
{
    return myWaitingNumber;
}

void L3_LLI_setMyWaitingNumber(uint8_t number)
{
    myWaitingNumber = number;
}

uint8_t L3_LLI_getTotalWaitingUsers(void)
{
    return totalWaitingUsers;
}

void L3_LLI_setTotalWaitingUsers(uint8_t total)
{
    totalWaitingUsers = total;
}

// Step 0-6: 대기열 필터 함수 (4개)
uint8_t L3_LLI_getLastQueuePosition(void)
{
    return lastQueuePosition;
}

void L3_LLI_setLastQueuePosition(uint8_t position)
{
    lastQueuePosition = position;
}

uint8_t L3_LLI_getLastTotalWaiting(void)
{
    return lastTotalWaiting;
}

void L3_LLI_setLastTotalWaiting(uint8_t total)
{
    lastTotalWaiting = total;
}

// Step 0-7: 스캔 관리 함수 (6개)
BoothScanInfo_t* L3_LLI_getScannedBooths(void)
{
    return scannedBooths;
}

uint8_t L3_LLI_isScanning(void)
{
    return isScanning;
}

void L3_LLI_setScanning(uint8_t scanning)
{
    isScanning = scanning;
}

uint8_t L3_LLI_getScanResponseCount(void)
{
    return scanResponseCount;
}

void L3_LLI_incrementScanResponse(void)
{
    scanResponseCount++;
}

void L3_LLI_resetScanResponse(void)
{
    scanResponseCount = 0;
}

// Step 0-8: 채팅 관리 함수 (5개)
uint8_t L3_LLI_isTypingChat(void)
{
    return isTypingChat;
}

void L3_LLI_setTypingChat(uint8_t typing)
{
    isTypingChat = typing;
}

char* L3_LLI_getChatBuffer(void)
{
    return chatBuffer;
}

uint8_t L3_LLI_getChatIndex(void)
{
    return chatIndex;
}

void L3_LLI_setChatIndex(uint8_t index)
{
    chatIndex = index;
}