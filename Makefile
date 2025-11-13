# JavaScript语法解析器 Makefile
# 纯手工实现，不依赖任何词法/语法生成工具

# 编译器设置
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
LDFLAGS = 

# 目标文件
TARGET = js_parser
OBJS = main.o lexer.o parser.o common.o

# 测试目录
TEST_DIR = tests
VALID_DIR = $(TEST_DIR)/valid
INVALID_DIR = $(TEST_DIR)/invalid

# 默认目标
all: $(TARGET)

# 链接目标
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "构建完成: $(TARGET)"

# 编译规则
main.o: main.c parser.h lexer.h common.h
	$(CC) $(CFLAGS) -c main.c

lexer.o: lexer.c lexer.h common.h
	$(CC) $(CFLAGS) -c lexer.c

parser.o: parser.c parser.h lexer.h common.h
	$(CC) $(CFLAGS) -c parser.c

common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c

# 清理
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "清理完成"

# 创建测试目录
test-dirs:
	@mkdir -p $(VALID_DIR)
	@mkdir -p $(INVALID_DIR)

# 运行所有测试
test: $(TARGET) test-dirs
	@echo "========================================="
	@echo "运行JavaScript语法解析器测试"
	@echo "========================================="
	@echo ""
	@echo "测试1: 合法的JavaScript脚本"
	@echo "-----------------------------------------"
	@for file in $(VALID_DIR)/*.js; do \
		if [ -f "$$file" ]; then \
			echo "测试文件: $$file"; \
			./$(TARGET) "$$file"; \
			echo ""; \
		fi \
	done
	@echo ""
	@echo "测试2: 包含语法错误的JavaScript脚本"
	@echo "-----------------------------------------"
	@for file in $(INVALID_DIR)/*.js; do \
		if [ -f "$$file" ]; then \
			echo "测试文件: $$file"; \
			./$(TARGET) "$$file"; \
			echo ""; \
		fi \
	done
	@echo "========================================="
	@echo "测试完成"
	@echo "========================================="

# 快速测试（内联代码）
quick-test: $(TARGET)
	@echo "快速测试 - 合法代码:"
	@./$(TARGET) -s "let x = 10; console.log(x);"
	@echo ""
	@echo "快速测试 - ASI (return换行):"
	@./$(TARGET) -s "function test() { return\n42 }"
	@echo ""
	@echo "快速测试 - 箭头函数:"
	@./$(TARGET) -s "const add = (a, b) => a + b;"

# 帮助信息
help:
	@echo "JavaScript语法解析器 Makefile"
	@echo ""
	@echo "可用目标:"
	@echo "  all         - 编译解析器（默认）"
	@echo "  clean       - 清理编译文件"
	@echo "  test        - 运行所有测试用例"
	@echo "  quick-test  - 快速测试基本功能"
	@echo "  test-dirs   - 创建测试目录结构"
	@echo "  help        - 显示此帮助信息"

.PHONY: all clean test quick-test test-dirs help
