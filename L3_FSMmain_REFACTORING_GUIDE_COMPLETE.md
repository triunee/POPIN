# L3_FSMmain.cpp 리팩토링 가이드 (완전판)

## 📋 목차
1. [리팩토링 개요](#리팩토링-개요)
2. [모듈 구조 평가](#모듈-구조-평가)
3. [Phase 0: L3_LLinterface 확장](#phase-0-l3_llinterface-확장)
4. [Phase 0.5: L3_msg 디코딩 함수 추가](#phase-05-l3_msg-디코딩-함수-추가)
5. [Phase 1: L3_timer 모듈 완성](#phase-1-l3_timer-모듈-완성)
6. [Phase 2: 부스 스캔 함수 구현](#phase-2-부스-스캔-함수-구현)
7. [Phase 3: L3_FSMmain 변수 제거](#phase-3-l3_fsmmain-변수-제거)
8. [Phase 4: 함수 내부 리팩토링](#phase-4-함수-내부-리팩토링)
9. [Phase 5: 범용 헬퍼 함수 정리](#phase-5-범용-헬퍼-함수-정리)

---

## 리팩토링 개요

### 현재 상태
- **L3_FSMmain.cpp**: 모든 전역 변수 포함 (50개 이상) ❌
- **L3_LLinterface.cpp**: 메시지 수신 관련 변수만 포함 (5개) ⚠️
- **L3_timer.cpp**: ARQ 타이머와 부스 선택 타이머만 구현 (2개) ⚠️
- **L3_msg.cpp**: 인코딩만 구현, 디코딩 없음 ⚠️
- **L3_FSMevent.cpp**: 이벤트 플래그 관리 (완성됨) ✅

### 목표
- **L3_LLinterface.cpp**: FSM 상태 변수 이동 (getter/setter 31개 추가)
- **L3_timer.cpp**: 모든 타이머 변수 이동 (함수 25개 추가)
- **L3_msg.cpp**: 디코딩 함수 추가 (6개 추가)
- **L3_FSMmain.cpp**: FSM 로직만 남기기 (변수 5개만 유지)

---

## 모듈 구조 평가

### 📊 모듈별 캡슐화 품질 평가

#### 1️⃣ L3_FSMevent (이벤트 관리)

**현재 상태: ✅ 완벽**

**구조:**
```c
// L3_FSMevent.cpp
static uint32_t eventFlag;  // 완전히 숨겨짐

// 공개 인터페이스만 제공
void L3_event_setEventFlag(L3_event_e event);
void L3_event_clearEventFlag(L3_event_e event);
void L3_event_clearAllEventFlag(void);
int L3_event_checkEventFlag(L3_event_e event);
```

**평가:**
- ✅ **완벽한 캡슐화**: 내부 변수 완전히 숨김
- ✅ **단일 책임**: 이벤트 플래그 관리만 담당
- ✅ **명확한 인터페이스**: 직관적인 함수명
- ✅ **확장성**: enum으로 이벤트 타입 관리

**개선 필요 없음**

---

#### 2️⃣ L3_timer (타이머 관리)

**현재 상태: ⚠️ 불완전 (50% 완성)**

**구조:**
```c
// L3_timer.cpp
// ✅ 잘된 부분
static Timeout timer;
static uint8_t timerStatus;
void L3_timer_startTimer();
uint8_t L3_timer_getTimerStatus();

// ❌ 부족한 부분
// - 세션 타이머 변수/함수 없음
// - 연결/등록 타이머 변수/함수 없음
// - 큐 준비 타이머 변수/함수 없음
```

**문제점:**
- ❌ **불완전한 캡슐화**: 타이머 변수가 L3_FSMmain에 분산
- ❌ **책임 분산**: 타이머 관련 로직이 여러 파일에 존재
- ❌ **일관성 부족**: 일부는 함수, 일부는 직접 접근

**개선 방안:**
```
Phase 1에서 해결:
- 25개 타이머 함수 추가
- 모든 타이머 변수를 L3_timer.cpp로 이동
- 일관된 getter/setter 패턴 적용
```

**평가:**
- ⚠️ **부분적 캡슐화**: 일부만 구현됨
- ✅ **구조는 양호**: 기존 구조 확장 가능
- ⚠️ **완성 필요**: Phase 1에서 완성 예정

---

#### 3️⃣ L3_LLinterface (하위 레이어 인터페이스)

**현재 상태: ⚠️ 불완전 (20% 완성)**

**구조:**
```c
// L3_LLinterface.cpp
// ✅ 잘된 부분
static uint8_t rcvdMsg[L3_MAXDATASIZE];
static uint8_t rcvdSize;
uint8_t* L3_LLI_getMsgPtr();
uint8_t L3_LLI_getSize();

// ❌ 부족한 부분
// - FSM 상태 변수들이 L3_FSMmain에 있음
// - myId, isAdmin, myBooth 등 접근 함수 없음
```

**문제점:**
- ❌ **캡슐화 부족**: 상태 변수가 외부에 노출
- ❌ **책임 불명확**: 메시지만? 상태도?
- ❌ **모듈 경계 모호**: L3_FSMmain과 역할 중복

**개선 방안:**
```
Phase 0에서 해결:
- 31개 getter/setter 함수 추가
- FSM 상태 변수를 L3_LLinterface로 이동
- 명확한 인터페이스 정의
```

**평가:**
- ❌ **캡슐화 미흡**: 대부분 변수가 외부에 있음
- ⚠️ **확장 가능**: 구조는 확장하기 적합
- ⚠️ **완성 필요**: Phase 0에서 완성 예정

**네이밍 고려사항:**
```
현재: L3_LLinterface (Lower Layer Interface)
→ 메시지 인터페이스 + 상태 관리 혼재

대안:
1. L3_state (상태 관리 전담) ← 의미적으로 더 명확
2. L3_LLinterface 유지 (기존 호환성)

권장: 기존 L3_LLinterface 유지 (변경 비용 최소화)
```

---

#### 4️⃣ L3_msg (메시지 인코딩/디코딩)

**현재 상태: ⚠️ 불완전 (50% 완성)**

**구조:**
```c
// L3_msg.cpp
// ✅ 잘된 부분
uint8_t L3_msg_encodeBoothInfo(...);
uint8_t L3_msg_encodeQueueInfo(...);
uint8_t L3_msg_getType(uint8_t* msg);

// ❌ 부족한 부분
// - 디코딩 함수 없음
// - L3_FSMmain에서 직접 배열 인덱스 사용
//   예: data[0], data[1], data[2]
```

**문제점:**
- ❌ **비대칭 구조**: 인코딩은 함수, 디코딩은 직접 접근
- ❌ **가독성 저하**: `data[0]`이 무엇인지 불명확
- ❌ **유지보수 어려움**: 메시지 구조 변경 시 전체 수정 필요

**현재 사용 방식:**
```c
// L3_FSMmain.cpp - handleBoothInfo()
uint8_t currentUsers = data[0];      // ❌ 매직 넘버
uint8_t capacity = data[1];          // ❌ 매직 넘버
uint8_t waitingUsers = data[2];      // ❌ 매직 넘버
char *description = (char *)(data + 3);  // ❌ 매직 넘버
```

**개선 방안:**
```c
// Phase 0.5에서 추가할 디코딩 함수
void L3_msg_decodeBoothInfo(uint8_t* data, 
                            uint8_t* currentUsers,
                            uint8_t* capacity,
                            uint8_t* waitingUsers,
                            char** description);

// 사용 예시
uint8_t currentUsers, capacity, waitingUsers;
char* description;
L3_msg_decodeBoothInfo(data, &currentUsers, &capacity, 
                      &waitingUsers, &description);
```

**평가:**
- ⚠️ **절반만 완성**: 인코딩만 구현됨
- ✅ **구조는 양호**: 일관된 함수 네이밍
- ⚠️ **완성 필요**: Phase 0.5에서 디코딩 추가

---

#### 5️⃣ L3_FSMmain (메인 FSM 로직)

**현재 상태: ❌ 과부하**

**문제점:**
```c
// ❌ 너무 많은 책임
- FSM 로직 (본래 역할)
- 타이머 관리 (→ L3_timer로 이동 필요)
- 상태 변수 관리 (→ L3_LLinterface로 이동 필요)
- 부스 스캔 로직 (→ 별도 함수로 분리 필요)
- 사용자 관리 로직 (→ 별도 함수로 분리 필요)
```

### 📈 모듈 구조 개선 요약

| 모듈 | 현재 상태 | 캡슐화 품질 | 개선 필요도 | 담당 Phase |
|------|----------|-----------|-----------|-----------|
| **L3_FSMevent** | ✅ 완성 | ⭐⭐⭐⭐⭐ 완벽 | 없음 | - |
| **L3_timer** | ⚠️ 불완전 | ⭐⭐⭐ 보통 | 높음 | Phase 1 |
| **L3_LLinterface** | ⚠️ 불완전 | ⭐⭐ 낮음 | 매우 높음 | Phase 0 |
| **L3_msg** | ⚠️ 불완전 | ⭐⭐⭐ 보통 | 중간 | Phase 0.5 |
| **L3_FSMmain** | ❌ 과부하 | ⭐ 매우 낮음 | 매우 높음 | Phase 0~5 |

---

### 🎯 리팩토링 후 이상적인 구조

```
┌─────────────────────────────────────────┐
│         L3_FSMmain.cpp                  │
│  - FSM 상태 전이 로직                      │
│  - 메시지 핸들러                           │
│  - 최소한의 변수 (5개)                      │
└─────────────────────────────────────────┘
         │ 호출
         ↓
┌──────────────┬──────────────┬──────────────┬──────────────┐
│ L3_FSMevent  │  L3_timer    │ L3_LLI...    │   L3_msg     │
│              │              │              │              │
│ 이벤트 관리     │ 타이머 관리     │   상태 관리    │ 메시지 변환     │
│ (완성 ✅)      │ (확장 필요)    │ (확장 필요)    │ (확장 필요)    │
│              │              │              │              │
│ setFlag()    │ sessionStart │ getMyId()    │ encode...()  │
│ checkFlag()  │ connectStart │ getMyBooth() │ decode...()  │
│ clearFlag()  │ ...25개       │ ...31개      │ ...17개       │
└──────────────┴──────────────┴──────────────┴──────────────┘
```

---

## Phase 0: L3_LLinterface 확장

### 목표
L3_FSMmain.cpp의 상태 변수를 L3_LLinterface.cpp로 이동하고 getter/setter 함수 구현

### 작업 파일
- `L3_LLinterface.h` (함수 선언 추가)
- `L3_LLinterface.cpp` (변수 이동 및 함수 구현)

---

### Step 0-1: 변수 이동

**L3_LLinterface.cpp 상단에 추가:**
```c
// 기존 코드 (유지)
#include "mbed.h"
#include "L3_FSMevent.h"
#include "L3_msg.h"
#include "protocol_parameters.h"
#include "time.h"
#include "L3_types.h"  // ← 추가 필요

static uint8_t rcvdMsg[L3_MAXDATASIZE];
static uint8_t rcvdSize;
static int16_t rcvdRssi;
static int8_t rcvdSnr;
static uint8_t rcvdSrcId;

// ====== 추가할 변수 (L3_FSMmain.cpp에서 이동) ======

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
```

---

### Step 0-2: 초기화 함수

**L3_LLinterface.h에 추가:**
```c
void L3_LLI_initConfig(uint8_t id);
```

**L3_LLinterface.cpp에 추가:**
```c
void L3_LLI_initConfig(uint8_t id)
{
    myId = id;
    isAdmin = (id >= ADMIN_ID_START && id <= ADMIN_ID_END) ? 1 : 0;
    currentBoothId = 0;
}
```

---

### Step 0-3 ~ 0-8: Getter/Setter 함수 구현

### Step 0-3: ID/역할 관리 함수 (5개)

**L3_LLinterface.h에 추가:**
```c
uint8_t L3_LLI_getMyId(void);
uint8_t L3_LLI_isAdmin(void);
uint8_t L3_LLI_getCurrentBoothId(void);
void L3_LLI_setCurrentBoothId(uint8_t boothId);
```

**L3_LLinterface.cpp에 추가:**
```c
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
```

---

### Step 0-4: 부스 관리 함수 (6개)

**L3_LLinterface.h에 추가:**
```c
Booth_t* L3_LLI_getMyBooth(void);
uint8_t L3_LLI_getRegisteredCount(void);
void L3_LLI_incrementRegisteredCount(void);
uint8_t L3_LLI_isQuietMode(void);
void L3_LLI_toggleQuietMode(void);
uint8_t L3_LLI_getWaitingNumber(void);
```

**L3_LLinterface.cpp에 추가:**
```c
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
```

---

### Step 0-5: 사용자 대기열 함수 (4개)

**L3_LLinterface.h에 추가:**
```c
uint8_t L3_LLI_getMyWaitingNumber(void);
void L3_LLI_setMyWaitingNumber(uint8_t number);
uint8_t L3_LLI_getTotalWaitingUsers(void);
void L3_LLI_setTotalWaitingUsers(uint8_t total);
```

**L3_LLinterface.cpp에 추가:**
```c
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
```

---

### Step 0-6: 대기열 필터 함수 (4개)

**L3_LLinterface.h에 추가:**
```c
uint8_t L3_LLI_getLastQueuePosition(void);
void L3_LLI_setLastQueuePosition(uint8_t position);
uint8_t L3_LLI_getLastTotalWaiting(void);
void L3_LLI_setLastTotalWaiting(uint8_t total);
```

**L3_LLinterface.cpp에 추가:**
```c
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
```

---

### Step 0-7: 스캔 관리 함수 (6개)

**L3_LLinterface.h에 추가:**
```c
BoothScanInfo_t* L3_LLI_getScannedBooths(void);
uint8_t L3_LLI_isScanning(void);
void L3_LLI_setScanning(uint8_t scanning);
uint8_t L3_LLI_getScanResponseCount(void);
void L3_LLI_incrementScanResponse(void);
void L3_LLI_resetScanResponse(void);
```

**L3_LLinterface.cpp에 추가:**
```c
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
```

---

### Step 0-8: 채팅 관리 함수 (5개)

**L3_LLinterface.h에 추가:**
```c
uint8_t L3_LLI_isTypingChat(void);
void L3_LLI_setTypingChat(uint8_t typing);
char* L3_LLI_getChatBuffer(void);
uint8_t L3_LLI_getChatIndex(void);
void L3_LLI_setChatIndex(uint8_t index);
```

**L3_LLinterface.cpp에 추가:**
```c
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
```

---

총 31개 함수:
- Step 0-3: ID/역할 관리 (5개)
- Step 0-4: 부스 관리 (6개)
- Step 0-5: 사용자 대기열 (4개)
- Step 0-6: 대기열 필터 (4개)
- Step 0-7: 스캔 관리 (6개)
- Step 0-8: 채팅 관리 (5개)

---

### Phase 0 완료 체크리스트
- [ ] L3_LLinterface.cpp에 변수 추가
- [ ] #include "L3_types.h" 추가
- [ ] L3_LLinterface.h에 함수 선언 31개 추가
- [ ] L3_LLinterface.cpp에 함수 구현 31개 추가

---

## Phase 0.5: L3_msg 디코딩 함수 추가

### 목표
메시지 디코딩 함수를 추가하여 인코딩/디코딩 대칭성 확보

### 작업 파일
- `L3_msg.h` (함수 선언 6개 추가)
- `L3_msg.cpp` (함수 구현 6개 추가)

---

### 추가할 디코딩 함수

**L3_msg.h에 추가:**
```c
// 메시지 디코딩 함수 (Phase 0.5 추가)
void L3_msg_decodeBoothInfo(uint8_t* data, 
                            uint8_t* currentCount,
                            uint8_t* capacity,
                            uint8_t* waitingCount,
                            char** description);

void L3_msg_decodeQueueInfo(uint8_t* data,
                            uint8_t* queuePosition,
                            uint8_t* totalWaiting);

void L3_msg_decodeRegisterResponse(uint8_t* data,
                                   uint8_t* success,
                                   uint8_t* reason);

void L3_msg_decodeBoothAnnounce(uint8_t* data,
                               uint8_t* boothId,
                               uint8_t* currentCount,
                               uint8_t* capacity,
                               uint8_t* waitingCount);

void L3_msg_decodeAdminMessage(uint8_t* data, char** message);

void L3_msg_decodeChatMessage(uint8_t* data, 
                              uint8_t* senderId,
                              char** message);
```

---

**L3_msg.cpp에 추가:**
```c
// ====== 디코딩 함수 구현 (Phase 0.5) ======

// BOOTH_INFO 디코딩
void L3_msg_decodeBoothInfo(uint8_t* data, 
                            uint8_t* currentCount,
                            uint8_t* capacity,
                            uint8_t* waitingCount,
                            char** description)
{
    *currentCount = data[0];
    *capacity = data[1];
    *waitingCount = data[2];
    *description = (char*)(data + 3);
}

// QUEUE_INFO 디코딩
void L3_msg_decodeQueueInfo(uint8_t* data,
                            uint8_t* queuePosition,
                            uint8_t* totalWaiting)
{
    *queuePosition = data[0];
    *totalWaiting = data[1];
}

// REGISTER_RESPONSE 디코딩
void L3_msg_decodeRegisterResponse(uint8_t* data,
                                   uint8_t* success,
                                   uint8_t* reason)
{
    *success = data[0];
    *reason = data[1];
}

// BOOTH_ANNOUNCE 디코딩
void L3_msg_decodeBoothAnnounce(uint8_t* data,
                               uint8_t* boothId,
                               uint8_t* currentCount,
                               uint8_t* capacity,
                               uint8_t* waitingCount)
{
    *boothId = data[0];
    *currentCount = data[1];
    *capacity = data[2];
    *waitingCount = data[3];
}

// ADMIN_MESSAGE 디코딩
void L3_msg_decodeAdminMessage(uint8_t* data, char** message)
{
    *message = (char*)data;
}

// CHAT_MESSAGE 디코딩 (발신자 ID 포함)
void L3_msg_decodeChatMessage(uint8_t* data, 
                              uint8_t* senderId,
                              char** message)
{
    *senderId = data[0];
    *message = (char*)(data + 1);
}
```

---

### 사용 예시 (Before & After)

**Before (직접 파싱):**
```c
static void handleBoothInfo(uint8_t *data, uint8_t size)
{
    uint8_t currentUsers = data[0];      // ❌ 매직 넘버
    uint8_t capacity = data[1];          // ❌ 매직 넘버
    uint8_t waitingUsers = data[2];      // ❌ 매직 넘버
    char *description = (char *)(data + 3);  // ❌ 매직 넘버
    
    pc.printf("Description: %s\n", description);
}
```

**After (디코딩 함수 사용):**
```c
static void handleBoothInfo(uint8_t *data, uint8_t size)
{
    uint8_t currentUsers, capacity, waitingUsers;
    char *description;
    
    // ✅ 명확한 디코딩
    L3_msg_decodeBoothInfo(data, &currentUsers, &capacity, 
                          &waitingUsers, &description);
    
    pc.printf("Description: %s\n", description);
}
```

---

### Phase 0.5 완료 체크리스트
- [ ] L3_msg.h에 디코딩 함수 선언 6개 추가
- [ ] L3_msg.cpp에 디코딩 함수 구현 6개 추가
- [ ] 인코딩/디코딩 대칭성 확보

---

### Phase 0.5 효과

**개선 효과:**
- ✅ **가독성 향상**: `data[0]` → `currentUsers`
- ✅ **유지보수성**: 메시지 구조 변경 시 디코딩 함수만 수정
- ✅ **일관성**: 인코딩/디코딩 모두 함수로 처리
- ✅ **안전성**: 타입 체크 가능

**Before vs After 비교:**
```c
// Before: 17개 함수
11개 인코딩 함수 + 2개 유틸 함수 + 0개 디코딩 함수 = 13개

// After: 19개 함수  
11개 인코딩 함수 + 2개 유틸 함수 + 6개 디코딩 함수 = 19개
```

---

## Phase 1: L3_timer 모듈 완성

### 목표
L3_timer.cpp/.h에 누락된 타이머 함수 25개 구현

### 작업 파일
- `L3_timer.h` (함수 선언 추가)
- `L3_timer.cpp` (함수 구현 추가)

---

### Step 1-1: 세션 타이머 (5개)

**L3_timer.cpp에 추가할 변수:**
```c
static uint32_t sessionStartTime = 0;
static uint8_t isSessionActive = 0;
```

**L3_timer.h에 추가할 선언:**
```c
void L3_timer_sessionStart(void);
void L3_timer_sessionEnd(void);
uint8_t L3_timer_isSessionActive(void);
uint32_t L3_timer_getSessionStartTime(void);
uint32_t L3_timer_getSessionElapsedTime(void);
```

**L3_timer.cpp에 추가할 구현:**
```c
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
```

---

### Step 1-2: 세션 카운터 (3개)

**L3_timer.cpp에 추가할 변수:**
```c
static uint32_t sessionTimerCounter = 0;
```

**L3_timer.h에 추가할 선언:**
```c
void L3_timer_sessionCounterTick(void);
void L3_timer_sessionCounterReset(void);
uint32_t L3_timer_getSessionCounter(void);
```

**L3_timer.cpp에 추가할 구현:**
```c
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
```

---

### Step 1-3: 스캐닝 타이머 (3개)

**L3_timer.cpp에 추가할 변수:**
```c
static uint32_t scanningTimer = 0;
```

**L3_timer.h에 추가할 선언:**
```c
void L3_timer_scanningTick(void);
void L3_timer_resetScanningTimer(void);
uint32_t L3_timer_getScanningTimer(void);
```

**L3_timer.cpp에 추가할 구현:**
```c
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
```

---

### Step 1-4: 큐 준비 타이머 (5개)

**L3_timer.cpp에 추가할 변수:**
```c
static uint8_t pendingUserId = 0;
static uint32_t queueReadyStartTime = 0;
static uint8_t isQueueReadyTimerActive = 0;
```

**L3_timer.h에 추가할 선언:**
```c
void L3_timer_queueReadyStart(uint8_t userId);
void L3_timer_queueReadyStop(void);
uint8_t L3_timer_isQueueReadyActive(void);
uint8_t L3_timer_getPendingUserId(void);
uint32_t L3_timer_getQueueReadyElapsedTime(void);
```

**L3_timer.cpp에 추가할 구현:**
```c
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
```

---

### Step 1-5: 연결 타이머 (6개)

**L3_timer.cpp에 추가할 변수:**
```c
static uint8_t connectRetryCount = 0;
static uint32_t connectRequestTime = 0;
static uint8_t isWaitingForBoothInfo = 0;
```

**L3_timer.h에 추가할 선언:**
```c
void L3_timer_connectStart(void);
void L3_timer_connectEnd(void);
uint8_t L3_timer_isWaitingForBoothInfo(void);
uint8_t L3_timer_getConnectRetryCount(void);
void L3_timer_incrementConnectRetry(void);
void L3_timer_resetConnectRetry(void);
```

**L3_timer.cpp에 추가할 구현:**
```c
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
```

---

### Step 1-6: 등록 타이머 (6개)

**L3_timer.cpp에 추가할 변수:**
```c
static uint8_t registerRetryCount = 0;
static uint32_t registerRequestTime = 0;
static uint8_t isWaitingForRegisterResponse = 0;
```

**L3_timer.h에 추가할 선언:**
```c
void L3_timer_registerStart(void);
void L3_timer_registerEnd(void);
uint8_t L3_timer_isWaitingForRegisterResponse(void);
uint8_t L3_timer_getRegisterRetryCount(void);
void L3_timer_incrementRegisterRetry(void);
void L3_timer_resetRegisterRetry(void);
```

**L3_timer.cpp에 추가할 구현:**
```c
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
```

---

### Phase 1 완료 체크리스트
- [ ] L3_timer.h에 함수 선언 28개 추가 (기존 3개 + 신규 25개)
- [ ] L3_timer.cpp에 함수 구현 25개 추가
---

25개 타이머 함수 추가:
- Step 1-1: 세션 타이머 (5개)
- Step 1-2: 세션 카운터 (3개)
- Step 1-3: 스캐닝 타이머 (3개)
- Step 1-4: 큐 준비 타이머 (5개)
- Step 1-5: 연결 타이머 (6개)
- Step 1-6: 등록 타이머 (6개)

---

## Phase 2: 부스 스캔 함수 구현

### 목표
매개변수를 받는 범용 부스 스캔 함수 4개 구현

### 작업 파일
- `L3_FSMmain.h` (새로 생성 또는 기존 수정)
- `L3_FSMmain.cpp` (함수 구현 추가)

---

### 함수 1: L3_initBoothScanList

**L3_FSMmain.h에 추가:**
```c
#ifndef L3_FSMMAIN_H
#define L3_FSMMAIN_H

#include "L3_types.h"

void L3_initBoothScanList(BoothScanInfo_t* booths, uint8_t maxBooths);
void L3_updateBoothScanInfo(BoothScanInfo_t* booths, uint8_t maxBooths,
                            uint8_t boothId, int16_t rssi,
                            uint8_t currentCount, uint8_t capacity,
                            uint8_t waitingCount);
uint8_t L3_selectOptimalBooth(BoothScanInfo_t* booths, uint8_t maxBooths);
void L3_displayScannedBooths(BoothScanInfo_t* booths, uint8_t maxBooths);

#endif
```

**L3_FSMmain.cpp에 추가 (함수 프로토타입 섹션 다음):**
```c
void L3_initBoothScanList(BoothScanInfo_t* booths, uint8_t maxBooths)
{
    for (uint8_t i = 0; i < maxBooths; i++)
    {
        booths[i].boothId = 0;
        booths[i].rssi = -127;
        booths[i].currentCount = 0;
        booths[i].capacity = 0;
        booths[i].waitingCount = 0;
        booths[i].lastSeenTime = 0;
        booths[i].isValid = 0;
    }
}
```

---

### 함수 2: L3_updateBoothScanInfo

**L3_FSMmain.cpp에 추가:**
```c
void L3_updateBoothScanInfo(BoothScanInfo_t* booths, uint8_t maxBooths,
                            uint8_t boothId, int16_t rssi,
                            uint8_t currentCount, uint8_t capacity,
                            uint8_t waitingCount)
{
    for (uint8_t i = 0; i < maxBooths; i++)
    {
        if (booths[i].boothId == boothId || !booths[i].isValid)
        {
            booths[i].boothId = boothId;
            booths[i].rssi = rssi;
            booths[i].currentCount = currentCount;
            booths[i].capacity = capacity;
            booths[i].waitingCount = waitingCount;
            booths[i].isValid = 1;
            break;
        }
    }
}
```

---

### 함수 3: L3_selectOptimalBooth

**L3_FSMmain.cpp에 추가:**
```c
uint8_t L3_selectOptimalBooth(BoothScanInfo_t* booths, uint8_t maxBooths)
{
    uint8_t bestBoothId = 0;
    int16_t bestScore = -1000;

    for (uint8_t i = 0; i < maxBooths; i++)
    {
        if (booths[i].isValid)
        {
            int16_t score = booths[i].rssi;
            
            uint8_t availableSpace = booths[i].capacity - booths[i].currentCount;
            if (availableSpace > 0)
            {
                score += 20;
            }
            
            score -= (booths[i].waitingCount * 10);

            if (score > bestScore)
            {
                bestScore = score;
                bestBoothId = booths[i].boothId;
            }
        }
    }

    return bestBoothId;
}
```

---

### 함수 4: L3_displayScannedBooths

**L3_FSMmain.cpp에 추가:**
```c
void L3_displayScannedBooths(BoothScanInfo_t* booths, uint8_t maxBooths)
{
    pc.printf("\n=== SCANNED BOOTHS (RSSI-based) ===\n");
    uint8_t foundCount = 0;

    for (uint8_t i = 0; i < maxBooths; i++)
    {
        if (booths[i].isValid)
        {
            pc.printf("Booth %d: RSSI=%d dBm, Users=%d/%d, Waiting=%d\n",
                      booths[i].boothId, booths[i].rssi,
                      booths[i].currentCount, booths[i].capacity,
                      booths[i].waitingCount);
            foundCount++;
        }
    }

    if (foundCount == 0)
    {
        pc.printf("No booths detected.\n");
    }
    pc.printf("===================================\n");
}
```

---

### Phase 2 완료 체크리스트
- [ ] L3_FSMmain.h 생성 및 함수 선언 4개 추가
- [ ] L3_FSMmain.cpp에 함수 구현 4개 추가
- [ ] 기존 static 함수는 아직 유지

---

4개 부스 스캔 함수:
- L3_initBoothScanList()
- L3_updateBoothScanInfo()
- L3_selectOptimalBooth()
- L3_displayScannedBooths()

---

## Phase 3: L3_FSMmain 변수 제거

### 목표
L3_FSMmain.cpp에서 다른 모듈로 이동된 전역 변수 제거

### 제거할 변수 목록

**다음 변수들을 L3_FSMmain.cpp에서 삭제:**
```c
// ❌ 삭제 (L3_LLinterface로 이동됨)
static uint8_t lastQueuePosition = 0;
static uint8_t lastTotalWaiting = 0;
static uint8_t myId;
static uint8_t currentBoothId = 0;
static uint8_t isAdmin = 0;
static Booth_t myBooth;
static uint8_t waitingNumber = 0;
static uint8_t registeredCount = 0;
static uint8_t quietMode = 0;
static uint8_t myWaitingNumber = 0;
static uint8_t totalWaitingUsers = 0;
static BoothScanInfo_t scannedBooths[MAX_BOOTHS];
static uint8_t isScanning = 0;
static uint8_t scanResponseCount = 0;
static uint8_t isTypingChat = 0;
static char chatBuffer[101];
static uint8_t chatIndex = 0;

// ❌ 삭제 (L3_timer로 이동됨)
static uint32_t scanningTimer = 0;
static uint32_t sessionTimerCounter = 0;
static uint32_t sessionStartTime = 0;
static uint8_t isSessionActive = 0;
static uint8_t pendingUserId = 0;
static uint32_t queueReadyStartTime = 0;
static uint8_t isQueueReadyTimerActive = 0;
static uint8_t connectRetryCount = 0;
static uint32_t connectRequestTime = 0;
static uint8_t isWaitingForBoothInfo = 0;
static uint8_t registerRetryCount = 0;
static uint32_t registerRequestTime = 0;
static uint8_t isWaitingForRegisterResponse = 0;
```

### 유지할 변수 목록

**다음 변수들은 유지:**
```c
// ✅ 유지 (FSM 상태)
static uint8_t main_state = L3STATE_SCANNING;
static uint8_t prev_state = main_state;

// ✅ 유지 (메시지 버퍼)
static uint8_t txBuffer[L3_MAXDATASIZE];
static uint8_t rxBuffer[L3_MAXDATASIZE];

// ✅ 유지 (시리얼 포트)
static Serial pc(USBTX, USBRX);
```

### Phase 3 완료 체크리스트
- [ ] 변수 제거 완료
- [ ] 유지할 변수 5개만 남음
- [ ] 컴파일 에러 발생 (정상, Phase 4에서 해결)

---

## Phase 4: 함수 내부 리팩토링

Phase 4는 L3_FSMmain.cpp의 모든 함수 내부에서 **직접 변수 접근**을 제거하고, **Phase 0~3에서 구현한 모듈 함수**를 사용하도록 변경하는 단계입니다.

### 목표
- 전역 변수 직접 접근 → getter/setter 함수로 변경
- 타이머 변수 직접 접근 → L3_timer 함수로 변경
- 메시지 데이터 직접 파싱 → L3_msg 디코딩 함수로 변경
- 부스 스캔 관련 코드 → L3_부스스캔 함수로 변경

---

### 1. 변수 접근 패턴 변경
```c
// ❌ 변경 전: 직접 접근
myId = 10;
currentBoothId = 5;
isScanning = 1;

// ✅ 변경 후: getter/setter 사용
// (읽기 전용이므로 setter 없음)
uint8_t id = L3_LLI_getMyId();
L3_LLI_setCurrentBoothId(5);
L3_LLI_setScanning(1);
```

### 2. 타이머 접근 패턴 변경
```c
// ❌ 변경 전: 직접 접근
sessionStartTime = us_ticker_read() / 1000;
isSessionActive = 1;

// ✅ 변경 후: L3_timer 함수 사용
L3_timer_sessionStart();
```

### 3. 메시지 파싱 패턴 변경
```c
// ❌ 변경 전: 직접 인덱싱
uint8_t boothId = data[0];
uint8_t currentCount = data[1];
uint8_t capacity = data[2];

// ✅ 변경 후: 디코딩 함수 사용
uint8_t boothId, currentCount, capacity, waitingCount;
L3_msg_decodeBoothAnnounce(data, &boothId, &currentCount, 
                          &capacity, &waitingCount);
```

### 4. 구조체 접근 패턴 변경
```c
// ❌ 변경 전: 전역 구조체 직접 접근
myBooth.currentCount++;
myBooth.waitingQueue[0].userId;

// ✅ 변경 후: 포인터를 통한 접근
Booth_t* myBooth = L3_LLI_getMyBooth();
myBooth->currentCount++;
myBooth->waitingQueue[0].userId;
```

---

## 📝 리팩토링 대상 함수 목록

### 1. 초기화 함수 (1개)
- ✅ `L3_initFSM()` - ID 초기화 및 부스 스캔 리스트 초기화

### 2. 메인 FSM 함수 (1개)
- ✅ `L3_FSMrun()` - 메시지 수신 처리 및 타이머 체크

### 3. 핸들러 함수 (8개)
- ✅ `handleConnectRequest()` - 연결 요청 처리
- ✅ `handleBoothInfo()` - 부스 정보 처리
- ✅ `handleRegisterResponse()` - 등록 응답 처리
- ✅ `handleQueueInfo()` - 대기열 정보 처리
- ✅ `handleChatMessage()` - 채팅 메시지 처리
- ✅ `checkSessionTimer()` - 세션 타이머 체크
- ✅ `checkQueueReadyTimeout()` - 대기열 준비 타임아웃 체크
- ✅ `handleQueueReadyTimeout()` - 대기열 준비 타임아웃 처리

### 4. 헬퍼 함수 (10개)
- ✅ `printWaitingStatus()` - 대기 상태 출력
- ✅ `displayBoothInfo()` - 부스 정보 표시
- ✅ `scanForBooths()` - 부스 스캔
- ✅ `L3service_processKeyboardInput()` - 사용자 키보드 입력 처리
- ✅ `L3admin_processKeyboardInput()` - 관리자 키보드 입력 처리
- ✅ `startSessionTimer()` - 세션 타이머 시작
- ✅ `endUserSession()` - 사용자 세션 종료
- ✅ `admitNextWaitingUser()` - 다음 대기 사용자 입장
- ✅ `removeFromWaitingQueue()` - 대기열에서 제거
- ✅ `updateAllWaitingUsers()` - 모든 대기 사용자 업데이트

---

## 🔧 함수별 상세 리팩토링 가이드

### 1. L3_initFSM() 함수

#### 변경 포인트
1. `myId = id` → `L3_LLI_initConfig(id)` 호출
2. `isAdmin = ...` → 자동으로 L3_LLI_initConfig에서 처리됨
3. `currentBoothId = 0` → 자동으로 L3_LLI_initConfig에서 처리됨
4. `initializeBoothScanList()` → `L3_initBoothScanList()` 호출

#### 변경 전
```c
void L3_initFSM(uint8_t id)
{
    myId = id;
    isAdmin = (id >= ADMIN_ID_START && id <= ADMIN_ID_END) ? 1 : 0;
    currentBoothId = 0;
    
    initializeBoothScanList();
    
    main_state = isAdmin ? L3STATE_ADMIN_READY : L3STATE_IDLE;
    prev_state = main_state;
}
```

#### 변경 후
```c
void L3_initFSM(uint8_t id)
{
    // ID, 역할, 부스 ID 초기화
    L3_LLI_initConfig(id);
    
    // 부스 스캔 리스트 초기화
    L3_initBoothScanList(L3_LLI_getScannedBooths(), MAX_BOOTHS);
    
    // FSM 상태 초기화
    main_state = L3_LLI_isAdmin() ? L3STATE_ADMIN_READY : L3STATE_IDLE;
    prev_state = main_state;
}
```

---

### 2. L3_FSMrun() 함수

#### 변경 포인트 (MSG_TYPE_BOOTH_ANNOUNCE 케이스)
1. `isAdmin` → `L3_LLI_isAdmin()`
2. `isScanning` → `L3_LLI_isScanning()`
3. 메시지 데이터 직접 파싱 → `L3_msg_decodeBoothAnnounce()` 사용
4. `updateBoothScanInfo()` → `L3_updateBoothScanInfo()` 호출
5. `scanResponseCount++` → `L3_LLI_incrementScanResponse()`

#### 변경 전
```c
case MSG_TYPE_BOOTH_ANNOUNCE:
    if (!isAdmin && isScanning)
    {
        uint8_t boothId = msgData[0];
        uint8_t currentCount = msgData[1];
        uint8_t capacity = msgData[2];
        uint8_t waitingCount = msgData[3];
        
        updateBoothScanInfo(boothId, rssi, currentCount, 
                           capacity, waitingCount);
        scanResponseCount++;
    }
    break;
```

#### 변경 후
```c
case MSG_TYPE_BOOTH_ANNOUNCE:
    if (!L3_LLI_isAdmin() && L3_LLI_isScanning())
    {
        uint8_t boothId, currentCount, capacity, waitingCount;
        
        L3_msg_decodeBoothAnnounce(msgData, &boothId, &currentCount,
                                  &capacity, &waitingCount);
        
        L3_updateBoothScanInfo(L3_LLI_getScannedBooths(), MAX_BOOTHS,
                              boothId, rssi, currentCount, 
                              capacity, waitingCount);
        L3_LLI_incrementScanResponse();
    }
    break;
```

#### 변경 포인트 (MSG_TYPE_CHAT_MESSAGE 케이스)
1. `isAdmin` → `L3_LLI_isAdmin()`
2. 관리자: 메시지 직접 캐스팅 → `L3_msg_decodeAdminMessage()` 사용
3. 사용자: 메시지 직접 파싱 → `L3_msg_decodeChatMessage()` 사용

#### 변경 전
```c
case MSG_TYPE_CHAT_MESSAGE:
    if (isAdmin)
    {
        uint8_t senderId = srcId;
        char* message = (char*)msgData;
        handleChatMessage(senderId, message);
    }
    else
    {
        uint8_t senderId = msgData[0];
        char* message = (char*)(msgData + 1);
        pc.printf("\n[User %d]: %s\n", senderId, message);
    }
    break;
```

#### 변경 후
```c
case MSG_TYPE_CHAT_MESSAGE:
    if (L3_LLI_isAdmin())
    {
        char* message;
        L3_msg_decodeAdminMessage(msgData, &message);
        handleChatMessage(srcId, message);
    }
    else
    {
        uint8_t senderId;
        char* message;
        L3_msg_decodeChatMessage(msgData, &senderId, &message);
        pc.printf("\n[User %d]: %s\n", senderId, message);
    }
    break;
```

#### 변경 포인트 (타이머 체크 부분)
1. ARQ 타이머: 기존 `L3_timer_checkARQ()` 사용 (변경 없음)
2. 부스 선택 타이머: 기존 `L3_timer_checkBoothSelection()` 사용 (변경 없음)
3. 연결 타이머: `isConnectTimerActive` → `L3_timer_isConnectActive()` 등
4. 등록 타이머: `isRegisterTimerActive` → `L3_timer_isRegisterActive()` 등
5. 세션 카운터: `sessionTimerCounter++` → `L3_timer_sessionCounterTick()`
6. 스캔 타이머: `scanningElapsedTime++` → `L3_timer_scanningTick()`

#### 변경 전
```c
void L3_FSMrun(void)
{
    // 메시지 처리...
    
    // 타이머 체크
    if (L3_timer_checkARQ())
    {
        sendMessage(currentBoothId, MSG_TYPE_ACK, txBuffer, 0);
    }
    
    if (L3_timer_checkBoothSelection())
    {
        uint8_t selectedBoothId = selectOptimalBooth();
        if (selectedBoothId != 0)
        {
            currentBoothId = selectedBoothId;
            isScanning = 0;
            sendMessage(selectedBoothId, MSG_TYPE_CONNECT_REQ, txBuffer, 0);
        }
    }
    
    // 연결 타이머 체크
    if (isConnectTimerActive)
    {
        uint32_t currentTime = us_ticker_read() / 1000;
        if (currentTime - connectStartTime >= CONNECT_TIMEOUT_MS)
        {
            isConnectTimerActive = 0;
            main_state = L3STATE_SCANNING;
            currentBoothId = 0;
            initializeBoothScanList();
        }
    }
    
    // 등록 타이머 체크
    if (isRegisterTimerActive)
    {
        uint32_t currentTime = us_ticker_read() / 1000;
        if (currentTime - registerStartTime >= REGISTER_TIMEOUT_MS)
        {
            isRegisterTimerActive = 0;
            main_state = L3STATE_SCANNING;
            currentBoothId = 0;
            initializeBoothScanList();
        }
    }
    
    // 세션 카운터 증가
    if (isSessionActive)
    {
        sessionTimerCounter++;
    }
    
    // 스캔 타이머 증가
    if (isScanning)
    {
        scanningElapsedTime++;
    }
}
```

#### 변경 후
```c
void L3_FSMrun(void)
{
    // 메시지 처리...
    
    // 타이머 체크
    if (L3_timer_checkARQ())
    {
        sendMessage(L3_LLI_getCurrentBoothId(), MSG_TYPE_ACK, txBuffer, 0);
    }
    
    if (L3_timer_checkBoothSelection())
    {
        uint8_t selectedBoothId = L3_selectOptimalBooth(
            L3_LLI_getScannedBooths(), MAX_BOOTHS);
        if (selectedBoothId != 0)
        {
            L3_LLI_setCurrentBoothId(selectedBoothId);
            L3_LLI_setScanning(0);
            sendMessage(selectedBoothId, MSG_TYPE_CONNECT_REQ, txBuffer, 0);
        }
    }
    
    // 연결 타이머 체크
    if (L3_timer_isConnectActive())
    {
        if (L3_timer_getConnectElapsedTime() >= CONNECT_TIMEOUT_MS)
        {
            L3_timer_connectStop();
            main_state = L3STATE_SCANNING;
            L3_LLI_setCurrentBoothId(0);
            L3_initBoothScanList(L3_LLI_getScannedBooths(), MAX_BOOTHS);
        }
    }
    
    // 등록 타이머 체크
    if (L3_timer_isRegisterActive())
    {
        if (L3_timer_getRegisterElapsedTime() >= REGISTER_TIMEOUT_MS)
        {
            L3_timer_registerStop();
            main_state = L3STATE_SCANNING;
            L3_LLI_setCurrentBoothId(0);
            L3_initBoothScanList(L3_LLI_getScannedBooths(), MAX_BOOTHS);
        }
    }
    
    // 세션 카운터 증가
    if (L3_timer_isSessionActive())
    {
        L3_timer_sessionCounterTick();
    }
    
    // 스캔 타이머 증가
    if (L3_LLI_isScanning())
    {
        L3_timer_scanningTick();
    }
    
    // 관리자 전용 타이머 체크
    if (L3_LLI_isAdmin())
    {
        checkSessionTimer();
        checkQueueReadyTimeout();
    }
}
```

---

### 3. handleBoothInfo() 함수

#### 변경 포인트
1. 메시지 데이터 직접 파싱 → `L3_msg_decodeBoothInfo()` 사용
2. `myWaitingNumber` → `L3_LLI_setMyWaitingNumber()`
3. `totalWaitingUsers` → `L3_LLI_setTotalWaitingUsers()`

#### 변경 전
```c
static void handleBoothInfo(uint8_t *data, uint8_t size)
{
    uint8_t currentUsers = data[0];
    uint8_t capacity = data[1];
    uint8_t waitingUsers = data[2];
    char *description = (char*)(data + 3);
    
    pc.printf("\n[Booth Info]\n");
    pc.printf("Current Users: %d/%d\n", currentUsers, capacity);
    pc.printf("Waiting: %d\n", waitingUsers);
    pc.printf("Description: %s\n", description);
    
    if (currentUsers < capacity)
    {
        myWaitingNumber = 0;
        totalWaitingUsers = 0;
        sendMessage(currentBoothId, MSG_TYPE_REGISTER_REQ, txBuffer, 0);
        isRegisterTimerActive = 1;
        registerStartTime = us_ticker_read() / 1000;
    }
    else
    {
        myWaitingNumber = waitingUsers + 1;
        totalWaitingUsers = waitingUsers + 1;
        sendMessage(currentBoothId, MSG_TYPE_REGISTER_REQ, txBuffer, 0);
        isRegisterTimerActive = 1;
        registerStartTime = us_ticker_read() / 1000;
    }
}
```

#### 변경 후
```c
static void handleBoothInfo(uint8_t *data, uint8_t size)
{
    uint8_t currentUsers, capacity, waitingUsers;
    char *description;
    
    L3_msg_decodeBoothInfo(data, &currentUsers, &capacity, 
                          &waitingUsers, &description);
    
    pc.printf("\n[Booth Info]\n");
    pc.printf("Current Users: %d/%d\n", currentUsers, capacity);
    pc.printf("Waiting: %d\n", waitingUsers);
    pc.printf("Description: %s\n", description);
    
    if (currentUsers < capacity)
    {
        L3_LLI_setMyWaitingNumber(0);
        L3_LLI_setTotalWaitingUsers(0);
        sendMessage(L3_LLI_getCurrentBoothId(), MSG_TYPE_REGISTER_REQ, 
                   txBuffer, 0);
        L3_timer_registerStart();
    }
    else
    {
        L3_LLI_setMyWaitingNumber(waitingUsers + 1);
        L3_LLI_setTotalWaitingUsers(waitingUsers + 1);
        sendMessage(L3_LLI_getCurrentBoothId(), MSG_TYPE_REGISTER_REQ, 
                   txBuffer, 0);
        L3_timer_registerStart();
    }
}
```

---

### 4. handleRegisterResponse() 함수

#### 변경 포인트
1. 메시지 데이터 직접 파싱 → `L3_msg_decodeRegisterResponse()` 사용
2. `sessionStartTime` → `L3_timer_sessionStart()`
3. `isSessionActive = 1` → `L3_timer_sessionStart()`에서 자동 처리
4. `currentBoothId = 0` → `L3_LLI_setCurrentBoothId(0)`
5. `initializeBoothScanList()` → `L3_initBoothScanList()` 호출

#### 변경 전
```c
static void handleRegisterResponse(uint8_t *data)
{
    uint8_t success = data[0];

    if (success)
    {
        main_state = L3STATE_IN_USE;
        sessionStartTime = us_ticker_read() / 1000;
        isSessionActive = 1;
    }
    else
    {
        main_state = L3STATE_SCANNING;
        currentBoothId = 0;
        initializeBoothScanList();
    }
}
```

#### 변경 후
```c
static void handleRegisterResponse(uint8_t *data)
{
    uint8_t success, reason;
    
    L3_msg_decodeRegisterResponse(data, &success, &reason);

    if (success)
    {
        main_state = L3STATE_IN_USE;
        L3_timer_sessionStart();
    }
    else
    {
        main_state = L3STATE_SCANNING;
        L3_LLI_setCurrentBoothId(0);
        L3_initBoothScanList(L3_LLI_getScannedBooths(), MAX_BOOTHS);
    }
}
```

---

### 5. handleQueueInfo() 함수

#### 변경 포인트
1. 메시지 데이터 직접 파싱 → `L3_msg_decodeQueueInfo()` 사용
2. `myWaitingNumber = ...` → `L3_LLI_setMyWaitingNumber()`
3. `totalWaitingUsers = ...` → `L3_LLI_setTotalWaitingUsers()`

#### 변경 전
```c
static void handleQueueInfo(uint8_t *data)
{
    myWaitingNumber = data[0];
    totalWaitingUsers = data[1];
    main_state = L3STATE_WAITING;
}
```

#### 변경 후
```c
static void handleQueueInfo(uint8_t *data)
{
    uint8_t queuePosition, totalWaiting;
    
    L3_msg_decodeQueueInfo(data, &queuePosition, &totalWaiting);
    
    L3_LLI_setMyWaitingNumber(queuePosition);
    L3_LLI_setTotalWaitingUsers(totalWaiting);
    main_state = L3STATE_WAITING;
}
```

---

### 6. checkSessionTimer() 함수

#### 변경 포인트
1. `myBooth` 직접 접근 → `L3_LLI_getMyBooth()` 포인터 사용
2. 모든 `myBooth.xxx` → `myBooth->xxx`로 변경

#### 변경 전
```c
static void checkSessionTimer(void)
{   
    uint32_t currentTime = us_ticker_read() / 1000;

    for (uint8_t i = 0; i < myBooth.currentCount; i++)
    {
        uint32_t sessionTime = currentTime - myBooth.activeList[i].sessionStartTime;
        
        if (sessionTime >= SESSION_TIME_LIMIT_MS)
        {
            uint8_t userId = myBooth.activeList[i].userId;
            endUserSession(userId);
            break;
        }
    }
}
```

#### 변경 후
```c
static void checkSessionTimer(void)
{   
    Booth_t* myBooth = L3_LLI_getMyBooth();
    uint32_t currentTime = us_ticker_read() / 1000;

    for (uint8_t i = 0; i < myBooth->currentCount; i++)
    {
        uint32_t sessionTime = currentTime - myBooth->activeList[i].sessionStartTime;
        
        if (sessionTime >= SESSION_TIME_LIMIT_MS)
        {
            uint8_t userId = myBooth->activeList[i].userId;
            endUserSession(userId);
            break;
        }
    }
}
```

---

### 7. admitNextWaitingUser() 함수

#### 변경 포인트
1. `myBooth` 직접 접근 → `L3_LLI_getMyBooth()` 포인터 사용
2. `isQueueReadyTimerActive` → `L3_timer_isQueueReadyActive()`
3. `pendingUserId = ...` → `L3_timer_queueReadyStart(userId)`에서 자동 처리
4. `queueReadyStartTime` → 내부에서 자동 처리
5. `isQueueReadyTimerActive = 1` → 자동 처리

#### 변경 전
```c
static void admitNextWaitingUser(void)
{
    if (myBooth.waitingCount > 0 && !isQueueReadyTimerActive)
    {
        uint8_t nextUserId = myBooth.waitingQueue[0].userId;
        
        uint8_t msgData[1];
        msgData[0] = 1;
        sendMessage(nextUserId, MSG_TYPE_QUEUE_READY, msgData, 1);
        
        pendingUserId = nextUserId;
        queueReadyStartTime = us_ticker_read() / 1000;
        isQueueReadyTimerActive = 1;
    }
}
```

#### 변경 후
```c
static void admitNextWaitingUser(void)
{
    Booth_t* myBooth = L3_LLI_getMyBooth();
    
    if (myBooth->waitingCount > 0 && !L3_timer_isQueueReadyActive())
    {
        uint8_t nextUserId = myBooth->waitingQueue[0].userId;
        
        uint8_t msgData[1];
        msgData[0] = 1;
        sendMessage(nextUserId, MSG_TYPE_QUEUE_READY, msgData, 1);
        
        L3_timer_queueReadyStart(nextUserId);
    }
}
```

---

### 8. checkQueueReadyTimeout() 함수

#### 변경 포인트
1. `isQueueReadyTimerActive` → `L3_timer_isQueueReadyActive()`
2. `us_ticker_read()` 직접 호출 → `L3_timer_getQueueReadyElapsedTime()` 사용
3. `pendingUserId` → `L3_timer_getPendingUserId()` 사용

#### 변경 전
```c
static void checkQueueReadyTimeout(void)
{
    if (isQueueReadyTimerActive)
    {
        uint32_t currentTime = us_ticker_read() / 1000;
        uint32_t elapsedTime = currentTime - queueReadyStartTime;

        if (elapsedTime >= QUEUE_READY_TIMEOUT_MS)
        {
            handleQueueReadyTimeout(pendingUserId);
        }
    }
}
```

#### 변경 후
```c
static void checkQueueReadyTimeout(void)
{
    if (L3_timer_isQueueReadyActive())
    {
        if (L3_timer_getQueueReadyElapsedTime() >= QUEUE_READY_TIMEOUT_MS)
        {
            handleQueueReadyTimeout(L3_timer_getPendingUserId());
        }
    }
}
```

---

### 9. handleQueueReadyTimeout() 함수

#### 변경 포인트
1. `isQueueReadyTimerActive = 0` → `L3_timer_queueReadyStop()`
2. `pendingUserId = 0` → `L3_timer_queueReadyStop()`에서 자동 처리
3. `myBooth` 직접 접근 → `L3_LLI_getMyBooth()` 포인터 사용

#### 변경 전
```c
static void handleQueueReadyTimeout(uint8_t userId)
{
    isQueueReadyTimerActive = 0;
    pendingUserId = 0;
    
    removeFromWaitingQueue(userId);

    if (myBooth.waitingCount > 0)
    {
        updateAllWaitingUsers();
    }
}
```

#### 변경 후
```c
static void handleQueueReadyTimeout(uint8_t userId)
{
    L3_timer_queueReadyStop();
    
    removeFromWaitingQueue(userId);

    Booth_t* myBooth = L3_LLI_getMyBooth();
    if (myBooth->waitingCount > 0)
    {
        updateAllWaitingUsers();
    }
}
```

---

### 10. 기타 헬퍼 함수들

나머지 모든 함수들도 동일한 패턴으로 변경:

```c
// printWaitingStatus()
myWaitingNumber → L3_LLI_getMyWaitingNumber()
totalWaitingUsers → L3_LLI_getTotalWaitingUsers()

// scanForBooths()
isScanning = 1 → L3_LLI_setScanning(1)
scanResponseCount = 0 → L3_LLI_resetScanResponse()
initializeBoothScanList() → L3_initBoothScanList(L3_LLI_getScannedBooths(), MAX_BOOTHS)

// L3service_processKeyboardInput()
isTypingChat → L3_LLI_isTypingChat()
chatBuffer → L3_LLI_getChatBuffer()
chatIndex → L3_LLI_getChatIndex()
currentBoothId → L3_LLI_getCurrentBoothId()

// L3admin_processKeyboardInput()
myBooth → L3_LLI_getMyBooth()
quietMode → L3_LLI_isQuietMode()
L3_LLI_toggleQuietMode()

// startSessionTimer()
sessionStartTime = ... → L3_timer_sessionStart()에 통합
isSessionActive = 1 → 자동 처리

// endUserSession()
myBooth → L3_LLI_getMyBooth()
isSessionActive = 0 → L3_timer_sessionEnd()
sessionStartTime = 0 → 자동 처리

// removeFromWaitingQueue()
myBooth → L3_LLI_getMyBooth()

// updateAllWaitingUsers()
myBooth → L3_LLI_getMyBooth()
```

---

## Phase 4 완료 체크리스트

### 핵심 함수 리팩토링
- [ ] `L3_initFSM()` - L3_LLI_initConfig() 사용
- [ ] `L3_FSMrun()` - 모든 타이머 및 상태 함수 변경
- [ ] `handleBoothInfo()` - L3_msg_decodeBoothInfo() 사용
- [ ] `handleRegisterResponse()` - L3_msg_decodeRegisterResponse() 사용
- [ ] `handleQueueInfo()` - L3_msg_decodeQueueInfo() 사용
- [ ] `checkSessionTimer()` - L3_LLI_getMyBooth() 포인터 사용
- [ ] `admitNextWaitingUser()` - L3_timer_queueReady 함수 사용
- [ ] `checkQueueReadyTimeout()` - L3_timer_getQueueReadyElapsedTime() 사용
- [ ] `handleQueueReadyTimeout()` - L3_timer_queueReadyStop() 사용

### 추가 헬퍼 함수
- [ ] `printWaitingStatus()` - getter 함수 사용
- [ ] `displayBoothInfo()` - getter 함수 사용
- [ ] `scanForBooths()` - L3_부스스캔 함수 사용
- [ ] `L3service_processKeyboardInput()` - getter/setter 사용
- [ ] `L3admin_processKeyboardInput()` - L3_LLI_getMyBooth() 사용
- [ ] `startSessionTimer()` - L3_timer_sessionStart() 사용
- [ ] `endUserSession()` - L3_timer_sessionEnd() 사용
- [ ] `removeFromWaitingQueue()` - L3_LLI_getMyBooth() 사용
- [ ] `updateAllWaitingUsers()` - L3_LLI_getMyBooth() 사용

### 컴파일 검증
- [ ] 컴파일 에러 0개
- [ ] 워닝 최소화
- [ ] 링킹 성공

---

## Phase 5: 범용 헬퍼 함수 정리

### 목표
재사용 가능한 범용 함수에 L3_ 접두사 추가

### 함수 리네이밍

**L3_FSMmain.h에 추가:**
```c
uint8_t L3_checkUserInList(User_t *list, uint8_t listSize, uint8_t userId);
uint8_t L3_addUserToList(User_t *list, uint8_t *count, uint8_t userId);
void L3_removeUserFromList(User_t *list, uint8_t *count, uint8_t userId);
int L3_getUserWaitingPosition(User_t *waitingQueue, int waitingCount, int srcId);
```

**L3_FSMmain.cpp에서 변경:**

```c
// static 제거, L3_ 접두사 추가
uint8_t L3_checkUserInList(User_t *list, uint8_t listSize, uint8_t userId)
{
    for (uint8_t i = 0; i < listSize; i++)
    {
        if (list[i].userId == userId)
        {
            return 1;
        }
    }
    return 0;
}

uint8_t L3_addUserToList(User_t *list, uint8_t *count, uint8_t userId)
{
    if (userId <= 3)
    {
        return 1;
    }
    
    for (uint8_t i = 0; i < *count; i++)
    {
        if (list[i].userId == userId)
        {
            return 1;
        }
    }

    if (*count < MAX_USERS)
    {
        list[*count].userId = userId;
        list[*count].isActive = 1;
        (*count)++;
        return 1;
    }
    return 0;
}

void L3_removeUserFromList(User_t *list, uint8_t *count, uint8_t userId)
{
    for (uint8_t i = 0; i < *count; i++)
    {
        if (list[i].userId == userId)
        {
            for (uint8_t j = i; j < *count - 1; j++)
            {
                list[j] = list[j + 1];
            }
            (*count)--;
            break;
        }
    }
}

int L3_getUserWaitingPosition(User_t *waitingQueue, int waitingCount, int srcId)
{
    for (int i = 0; i < waitingCount; i++)
    {
        if (waitingQueue[i].userId == srcId)
        {
            return i + 1;
        }
    }
    return 0;
}
```

**모든 호출 위치 변경:**
```c
// 변경 전
checkUserInList(myBooth.activeList, myBooth.currentCount, srcId)

// 변경 후
Booth_t* myBooth = L3_LLI_getMyBooth();
L3_checkUserInList(myBooth->activeList, myBooth->currentCount, srcId)
```

---

### Static으로 유지할 함수

**다음 함수들은 FSM 내부 전용이므로 static 유지:**
```c
static void sendMessage(...)
static void handleConnectRequest(...)
static void handleBoothInfo(...)
static void handleRegisterResponse(...)
static void handleQueueInfo(...)
static void printWaitingStatus()
static void displayBoothInfo(void)
static void scanForBooths(void)
static void L3service_processKeyboardInput(void)
static void L3admin_processKeyboardInput(void)
static void startSessionTimer(...)
static void checkSessionTimer(void)
static void endUserSession(...)
static void admitNextWaitingUser(void)
static void removeFromWaitingQueue(...)
static void updateAllWaitingUsers(void)
static void checkQueueReadyTimeout(void)
static void handleQueueReadyTimeout(...)
static void handleChatMessage(...)
static void broadcastChatToActiveUsers(...)
static void removeUserFromActiveList(...)
```

**제거할 함수 (새 함수로 대체됨):**
```c
// ❌ 삭제 (L3_initBoothScanList로 대체)
static void initializeBoothScanList(void);

// ❌ 삭제 (L3_updateBoothScanInfo로 대체)
static void updateBoothScanInfo(...);

// ❌ 삭제 (L3_selectOptimalBooth로 대체)
static uint8_t selectOptimalBooth(void);

// ❌ 삭제 (L3_displayScannedBooths로 대체)
static void displayScannedBooths(void);
```

---

## 최종 검증

### 전체 체크리스트
- [ ] Phase 0: L3_LLinterface 확장 (31개 함수)
- [ ] Phase 0.5: L3_msg 디코딩 추가 (6개 함수)
- [ ] Phase 1: L3_timer 모듈 완성 (25개 함수)
- [ ] Phase 2: 부스 스캔 함수 구현 (4개 함수)
- [ ] Phase 3: L3_FSMmain 변수 제거
- [ ] Phase 4: 함수 내부 리팩토링 (디코딩 함수 적용)
- [ ] Phase 5: 범용 헬퍼 함수 정리 (4개 함수)
- [ ] 전체 프로젝트 컴파일 성공
- [ ] 워닝 없음

---

## 컴파일 체크 방법

```bash
# mbed CLI 사용
mbed compile --target AUTO --toolchain GCC_ARM

# 또는 문법 체크만
g++ -fsyntax-only -I. -std=c++11 \
    L3_FSMmain.cpp \
    L3_timer.cpp \
    L3_LLinterface.cpp \
    L3_msg.cpp \
    L3_FSMevent.cpp
```

**성공 기준:**
- ✅ 0 errors
- ✅ 0 warnings (가능하면)
- ✅ Linking 성공

---

## 리팩토링 원칙 요약

### ✅ 지켜진 원칙

1. **단일 책임 원칙 (SRP)**
   - 각 모듈이 하나의 책임만 담당
   
2. **캡슐화 원칙**
   - 모든 내부 변수는 static으로 숨김
   - 공개 인터페이스만 헤더에 선언

3. **일관성 원칙**
   - 모든 접근은 getter/setter 패턴
   - 인코딩/디코딩 대칭 구조

4. **명명 규칙**
   - 모듈명_카테고리_동작 패턴
   - 예: L3_timer_sessionStart()

5. **확장성**
   - 새 기능 추가 시 기존 코드 수정 최소화
   - 새 메시지 타입 추가 시 디코딩 함수만 추가

---

**축하합니다! 완벽한 모듈 구조를 갖춘 리팩토링이 완료되었습니다!** 🎉
