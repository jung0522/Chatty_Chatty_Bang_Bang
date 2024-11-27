CC = gcc
CFLAGS = -pthread -Wall
SRC_DIR = src
OBJ_DIR = obj
SRC_SERVER = $(SRC_DIR)/server.c
SRC_CLIENT = $(SRC_DIR)/client.c
OBJ_SERVER = $(OBJ_DIR)/server.o
OBJ_CLIENT = $(OBJ_DIR)/client.o
EXEC_SERVER = server
EXEC_CLIENT = client

# 기본 타겟
all: $(EXEC_SERVER) $(EXEC_CLIENT)

# 서버 빌드
$(EXEC_SERVER): $(OBJ_SERVER)
	$(CC) $(CFLAGS) -o $@ $^

# 클라이언트 빌드
$(EXEC_CLIENT): $(OBJ_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^

# 서버 오브젝트 파일 빌드
$(OBJ_DIR)/server.o: $(SRC_SERVER) $(SRC_DIR)/chat.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 클라이언트 오브젝트 파일 빌드
$(OBJ_DIR)/client.o: $(SRC_CLIENT) $(SRC_DIR)/chat.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 디렉토리 생성
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# 클린업
clean:
	rm -rf $(OBJ_DIR) $(EXEC_SERVER) $(EXEC_CLIENT)

