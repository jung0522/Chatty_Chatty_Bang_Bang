#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "chat.h"

// 시간을 포맷하여 반환하는 함수
void get_current_time(char *buffer, size_t size) {
    time_t now = time(NULL);  // 현재 시간을 가져옴
    struct tm *t = localtime(&now);  // 현지 시간으로 변환
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);  // 시간 포맷
}

// 서버로부터 메시지를 수신하는 스레드 함수
void *receive_messages(void *arg) {
    int sock = *((int *)arg);  // 서버 소켓
    char buffer[BUFFER_SIZE];  // 수신할 메시지 버퍼
    int read_size;

    // 서버에서 오는 메시지를 수신하여 출력
    while ((read_size = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[read_size] = '\0';  // 수신한 메시지에 NULL 문자 추가

        // 현재 시간을 구하여 출력
        char time_buffer[20];
        get_current_time(time_buffer, sizeof(time_buffer));

        printf("[%s] %s\n", time_buffer, buffer);  // 시간과 함께 메시지 출력
    }

    return NULL;
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    pthread_t recv_thread;
    char name[50];

    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    // 서버 연결
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        exit(1);
    }

    // 사용자 이름 입력
    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;  // 끝의 개행 문자를 제거

    // 이름을 서버로 전송
    send(sock, name, strlen(name), 0);

    // 서버로부터 오는 메시지를 받는 스레드 생성
    pthread_create(&recv_thread, NULL, receive_messages, (void*)&sock);
    pthread_detach(recv_thread);  // 스레드가 종료될 때 자원 해제

    // 사용자로부터 메시지를 입력받아 서버로 전송
    while (1) {
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;  // 끝의 개행 문자를 제거
        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        // 서버로 메시지 전송
        send(sock, buffer, strlen(buffer), 0);
    }

    close(sock);  // 소켓 종료
    return 0;
}

