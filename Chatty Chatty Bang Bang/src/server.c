#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "chat.h"

// 클라이언트 구조체 배열과 관련된 변수들
Client *clients[MAX_CLIENTS];  // 연결된 클라이언트들 저장
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;  // 클라이언트 리스트를 위한 뮤텍스
int client_count = 0;  // 현재 연결된 클라이언트 수 추적

// 현재 시간을 포맷하여 출력하는 함수
void get_current_time(char *buffer, size_t size) {
    time_t t = time(NULL);  // 현재 시간을 가져옴
    struct tm *tm_info = localtime(&t);  // 현지 시간으로 변환
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);  // 시간 포맷
}

// 클라이언트와 연결된 스레드에서 처리하는 함수
void *handle_client(void *arg) {
    int client_sock = *((int*)arg);  // 클라이언트 소켓
    char buffer[BUFFER_SIZE];  // 수신할 메시지 버퍼
    char name[50];  // 클라이언트 이름
    int read_size;

    // 클라이언트 이름 수신
    if ((read_size = recv(client_sock, name, sizeof(name), 0)) <= 0) {
        printf("Failed to receive name\n");
        close(client_sock);  // 이름을 받지 못하면 연결 종료
        return NULL;
    }
    name[read_size] = '\0';  // 문자열 끝에 NULL 문자 추가

    // 클라이언트가 연결될 때 클라이언트 배열에 추가
    pthread_mutex_lock(&clients_mutex);  // 동기화: 클라이언트 리스트 수정 시 락을 사용
    if (client_count < MAX_CLIENTS) {
        clients[client_count++] = (Client*)malloc(sizeof(Client));  // 클라이언트 구조체 동적 할당
        clients[client_count - 1]->socket = client_sock;  // 소켓 저장
        strcpy(clients[client_count - 1]->name, name);  // 이름 저장
    } else {
        printf("Max clients reached, rejecting connection...\n");
        close(client_sock);  // 최대 클라이언트 수를 초과하면 연결 종료
        pthread_mutex_unlock(&clients_mutex);  // 동기화 해제
        return NULL;
    }
    pthread_mutex_unlock(&clients_mutex);  // 동기화 해제

    // 클라이언트가 채팅에 참여했음을 출력
    char time_buffer[20];
    get_current_time(time_buffer, sizeof(time_buffer));
    printf("[%s] Client %s has joined the chat.\n", time_buffer, name);

    // 클라이언트로부터 메시지를 수신하여 처리
    while ((read_size = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[read_size] = '\0';  // 수신한 메시지에 NULL 문자 추가
        get_current_time(time_buffer, sizeof(time_buffer));
        printf("[%s] %s: %s\n", time_buffer, name, buffer);  // 메시지 출력

        // 모든 클라이언트에게 메시지 전달
        char message_with_name[BUFFER_SIZE + 400];  // 이름과 메시지를 합치기 위한 충분한 크기 버퍼
        snprintf(message_with_name, sizeof(message_with_name), "%s: %s", name, buffer);  // 이름과 메시지 결합

        // 다른 클라이언트들에게 메시지 전송
        pthread_mutex_lock(&clients_mutex);  // 클라이언트 리스트 수정 시 락을 사용
        for (int i = 0; i < client_count; i++) {
            if (clients[i]->socket != client_sock) {
                send(clients[i]->socket, message_with_name, strlen(message_with_name), 0);
            }
        }
        pthread_mutex_unlock(&clients_mutex);  // 동기화 해제
    }

    // 클라이언트가 나갔을 때, 모든 클라이언트에게 알림 전송
    get_current_time(time_buffer, sizeof(time_buffer));
    pthread_mutex_lock(&clients_mutex);  // 클라이언트 리스트 수정 시 락을 사용
    for (int i = 0; i < client_count; i++) {
        if (clients[i]->socket != client_sock) {
            char leave_msg[BUFFER_SIZE];
            snprintf(leave_msg, sizeof(leave_msg), "Client %s has left the chat.", name);
            send(clients[i]->socket, leave_msg, strlen(leave_msg), 0);
        }
    }

    // 클라이언트 제거 및 소켓 종료
    close(client_sock);
    for (int i = 0; i < client_count; i++) {
        if (clients[i]->socket == client_sock) {
            free(clients[i]);  // 메모리 해제
            clients[i] = clients[client_count - 1];  // 마지막 클라이언트를 현재 클라이언트 자리로 이동
            client_count--;  // 클라이언트 수 감소
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);  // 동기화 해제

    // 클라이언트 나갔다는 메시지 출력
    printf("[%s] Client %s has left the chat.\n", time_buffer, name);
    return NULL;
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pthread_t thread_id;

    // 서버 소켓 생성
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // 서버 소켓 바인딩
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        exit(1);
    }

    // 클라이언트 연결 대기
    if (listen(server_sock, MAX_CLIENTS) == -1) {
        perror("Listen failed");
        exit(1);
    }

    printf("Server waiting for connections on port %d...\n", SERVER_PORT);

    // 클라이언트 연결 처리
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_sock == -1) {
            perror("Accept failed");
            continue;
        }

        printf("New client connected: %s\n", inet_ntoa(client_addr.sin_addr));

        // 클라이언트 처리용 스레드 생성
        if (pthread_create(&thread_id, NULL, handle_client, (void*)&client_sock) != 0) {
            perror("Thread creation failed");
            close(client_sock);
            continue;
        }

        pthread_detach(thread_id);  // 스레드가 종료될 때 자원 해제
    }

    close(server_sock);
    return 0;
}

