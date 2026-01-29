# Pop-in Protocol Stack

팝업스토어를 위한 LoRa 기반 스마트 체험 관리 시스템

> 이 프로젝트는 [원본 레포지토리](https://github.com/username/repo)를 
리팩토링한 버전입니다.

## 개요

Pop-in은 RSSI 기반 자동 부스 탐색과 실시간 대기열 관리를 통해 효율적인 팝업스토어 운영을 지원하는 임베디드 프로토콜 스택입니다.

### 주요 기능

- **자동 부스 탐색**: RSSI 신호 강도 기반 최적 부스 자동 선택
- **실시간 대기열**: Pull 기반 대기열 시스템으로 순번 관리
- **Push-to-Talk 채팅**: 부스 내 사용자 간 실시간 채팅
- **세션 관리**: 120초 체험 세션, 1인 1회 제한

## 시스템 요구사항

### 하드웨어

- STM32 NUCLEO-F446RE 보드
- LoRa 통신 모듈
- USB 시리얼 케이블

### 소프트웨어

- GCC ARM Embedded Toolchain
- mbed OS
- Make

## 빌드

```bash
# 빌드
make

# 클린 빌드
make clean && make
```

빌드 결과물은 `BUILD/` 폴더에 생성됩니다:
- `myProtocol.bin` - 바이너리 파일
- `myProtocol.hex` - HEX 파일

## 프로젝트 구조

```
popin-main/
├── main.cpp              # 메인 엔트리 포인트
├── L2_*.cpp/h            # Layer 2 - Stop-and-Wait ARQ, 데이터 분할/조립
├── L3_*.cpp/h            # Layer 3 - FSM, 메시지 처리, 세션 제어
├── protocol_parameters.h # 프로토콜 파라미터 설정
├── mbed/                 # mbed OS 라이브러리
├── lib/                  # 추가 라이브러리
└── Makefile              # 빌드 설정
```

## 프로토콜 아키텍처

```
┌─────────────────────────────────┐
│     L3 Layer                    │  Pop-in 프로토콜 로직
│  - 메시지 처리 (부스/대기열)          │  FSM, 세션 제어, 채팅
│  - FSM 상태 관리                  │  키보드 입력 처리
│  - 타이머 & 세션 제어               │
├─────────────────────────────────┤
│     L2 Layer                    │  Stop-and-Wait ARQ
│  - 신뢰성 전송                     │  재전송, ACK 관리
│  - 데이터 분할/조립                 │
├─────────────────────────────────┤
│     PHY/MAC Layer               │  LoRa 무선 통신
│  - 물리 계층 송수신                 │  RSSI 측정
└─────────────────────────────────┘
```

**주요 특징:**
- **L3**: Pop-in 프로토콜의 모든 로직 구현 (부스 관리, 대기열, 세션, 채팅, FSM)
- **L2**: 신뢰성 있는 전송을 위한 ARQ 프로토콜
- **PHY/MAC**: LoRa 기반 물리적 통신 및 RSSI 측정

## 사용 방법

### 관리자 모드 (ID: 1-3)

프로그램 실행 후 ID 입력 (1, 2, 또는 3)하면 자동으로 부스 운영이 시작됩니다.

**키보드 명령어:**
```
m - 메시지 전송 (부스 내 브로드캐스트)
a - 공지 전송 (전체 브로드캐스트)
s - 상태 확인 (부스 현황)
t - 타이머 확인 (세션 타이머)
w - 대기열 확인
q - Quiet 모드 토글 (자동 방송 on/off)
```

### 사용자 모드 (ID: 4+)

프로그램 실행 후 ID 입력 (4 이상)하면 자동으로 부스 탐색 및 연결이 시작됩니다.

**키보드 명령어:**
```
y - 체험 수락 (부스 정보 확인 후)
n - 체험 거절
c - 채팅 시작 (체험 중에만 가능)
e - 퇴장/대기열 이탈
```

**사용 흐름:**
1. 프로그램 시작 후 ID 입력
2. 자동으로 가장 강한 RSSI의 부스 탐색
3. 부스 정보 확인 후 y/n 선택
4. 체험 가능 시 바로 입장, 만원 시 자동 대기열 등록
5. 체험 중 채팅(c) 또는 타이머 만료 시 자동 퇴장
6. 동일 부스 재입장 불가 (1인 1회 제한)

## 시스템 파라미터

| 파라미터 | 값 | 설명 |
|---------|-----|------|
| `MAX_BOOTH_CAPACITY` | 1 | 부스 최대 수용 인원 |
| `MAX_USERS` | 20 | 최대 사용자 수 |
| `MAX_BOOTHS` | 3 | 최대 부스 수 |
| `SESSION_DURATION_MS` | 120,000 | 세션 시간 (120초 = 2분) |
| `L2_ARQ_MAXRETRANSMISSION` | 10 | ARQ 최대 재전송 횟수 |
| `L2_ARQ_MAXWAITTIME` | 3초 | ARQ 최대 대기 시간 |
| `CONNECT_TIMEOUT_MS` | 3,000 | 부스 연결 타임아웃 |
| `REGISTER_TIMEOUT_MS` | 3,000 | 등록 응답 타임아웃 |
| `QUEUE_READY_TIMEOUT_MS` | 10,000 | 입장 준비 타임아웃 |

## 메시지 타입

### 탐색 및 연결
| 메시지 | 코드 | 설명 |
|--------|------|------|
| BOOTH_SCAN | 0x0E | 부스 스캔 요청 (브로드캐스트) |
| BOOTH_ANNOUNCE | 0x0F | 부스 정보 방송 (RSSI 기반 선택용) |
| CONNECT_REQUEST | 0x02 | 부스 연결 요청 |
| CONNECT_RESPONSE | 0x03 | 부스 연결 응답 |
| BOOTH_INFO | 0x04 | 부스 상세 정보 전달 |

### 등록 및 세션
| 메시지 | 코드 | 설명 |
|--------|------|------|
| USER_RESPONSE | 0x05 | 사용자 y/n 응답 |
| REGISTER_REQUEST | 0x06 | 등록(체험) 요청 |
| REGISTER_RESPONSE | 0x07 | 등록 응답 (성공/실패/대기열) |
| EXIT_REQUEST | 0x0A | 부스 퇴장 요청 |
| EXIT_RESPONSE | 0x10 | 퇴장 응답 |
| TIMEOUT_ALERT | 0x0B | 세션 시간 만료 알림 |

### 대기열 관리
| 메시지 | 코드 | 설명 |
|--------|------|------|
| QUEUE_JOIN_REQUEST | 0x08 | 대기열 합류 요청 |
| QUEUE_INFO | 0x09 | 대기열 정보 (순번/총 대기자) |
| QUEUE_READY | 0x11 | 입장 준비 알림 |
| QUEUE_READY_ACK | 0x12 | 입장 준비 확인 |
| QUEUE_UPDATE | 0x13 | 대기 순번 업데이트 |
| QUEUE_LEAVE | 0x14 | 대기열 이탈 요청 |
| ALREADY_WAITING | 0x15 | 이미 대기열에 있음 |

### 통신
| 메시지 | 코드 | 설명 |
|--------|------|------|
| ADMIN_MESSAGE | 0x0C | 관리자 방송 메시지 |
| CHAT_MESSAGE | 0x0D | 채팅 메시지 (부스 내) |

## FSM 상태

### 사용자 FSM
- **SCANNING**: 부스 탐색 중
- **CONNECTED**: 부스 연결 완료 (정보 확인 중)
- **WAITING**: 대기열 대기 중
- **IN_USE**: 체험 세션 진행 중

### 관리자 동작
관리자는 별도 FSM 없이 이벤트 기반으로 동작:
- 부스 정보 주기적 방송
- 연결 요청 처리
- 등록 요청 처리 (중복 체크)
- 대기열 관리 (입장 순서 제어)
- 세션 타이머 관리 (자동 퇴장)

## 주요 특징

### ARQ 프로토콜
- **방식**: Stop-and-Wait ARQ
- **재전송**: 최대 10회
- **타임아웃**: 1-3초 (동적 조정)
- **흐름 제어**: 송신 측에서 ACK 수신 대기

### RSSI 기반 부스 선택
- 모든 부스로부터 ANNOUNCE 메시지 수신
- RSSI 값 비교하여 가장 강한 신호의 부스 자동 선택
- 네트워크 품질 기반 최적 부스 연결

### 1인 1회 제한
- 부스 측에서 등록 이력 영구 저장
- 동일 사용자의 재등록 요청 거부
- 공정한 체험 기회 보장

### Pull 기반 대기열
- 관리자가 입장 가능 시 QUEUE_READY 전송
- 사용자가 y/n 응답으로 입장 여부 결정
- 무응답 시 다음 대기자에게 자동 전환

## 디버그 설정

`protocol_parameters.h`에서 디버그 메시지 제어:
```c
#define DBGMSG_L2  0  // L2 레이어 디버그 (0: off, 1: on)
#define DBGMSG_L3  0  // L3 레이어 디버그 (0: off, 1: on)
```

## 기술 스택

- **플랫폼**: mbed OS
- **MCU**: STM32 NUCLEO-F446RE (ARM Cortex-M4)
- **통신**: LoRa (장거리 저전력)
- **언어**: C++
- **아키텍처**: Event-driven FSM
- **ARQ**: Stop-and-Wait
