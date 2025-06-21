# 컴파일러 및 플래그 설정
CC = gcc
# -Wall: 모든 경고 출력
# -Wextra: 추가적인 경고 출력
# -g: 디버깅 정보 포함
# -pthread: POSIX 스레드 라이브러리와 링크
CFLAGS = -Wall -Wextra -g -pthread

# 실행 파일 이름
TARGET = multi_t

# 소스 파일
SOURCES = multi_t.c

# 기본 빌드 규칙
all: $(TARGET)

# 실행 파일 생성 규칙
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

# 정리 규칙
clean:
	rm -f $(TARGET)

.PHONY: all clean