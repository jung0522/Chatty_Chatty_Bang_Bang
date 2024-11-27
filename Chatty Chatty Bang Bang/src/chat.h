#ifndef CHAT_H
#define CHAT_H

// 기본적인 상수 정의
#define SERVER_PORT 12345  // 서버 포트 번호
#define SERVER_IP "127.0.0.1"  // 서버 IP 주소 (로컬호스트)
#define MAX_CLIENTS 10  // 최대 클라이언트 수
#define BUFFER_SIZE 1024  // 버퍼 크기

// 클라이언트 구조체 정의
// 클라이언트의 소켓과 이름을 저장하는 구조체
typedef struct {
    int socket;  // 클라이언트 소켓
    char name[50];  // 클라이언트 이름
} Client;

// 메시지 구조체 정의 (필요한 경우 확장 가능)
// 서버와 클라이언트 간에 전송할 메시지의 형식을 정의할 수 있다.
// 예시: 메시지 내용과 발신자 이름을 포함
typedef struct {
    char message[BUFFER_SIZE];  // 메시지 내용
    char sender[50];  // 메시지 발신자
} Message;

// 함수 선언

// 현재 시간을 포맷하여 문자열로 반환하는 함수
// 예시: "2024-11-27 15:30:45"
void get_current_time(char *buffer, size_t size);

// 클라이언트와 서버가 소켓을 통해 데이터를 송수신할 때 사용할 함수들
// 서버는 클라이언트에서 보내는 메시지를 다른 클라이언트에게 전달하는 기능을 포함
// 클라이언트는 메시지를 서버로 보내고 서버에서 오는 메시지를 받는 기능을 포함
void *handle_client(void *arg);  // 서버에서 클라이언트의 연결을 처리하는 함수
void *receive_messages(void *arg);  // 클라이언트가 서버로부터 메시지를 받는 함수

#endif  // CHAT_H

