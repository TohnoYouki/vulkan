.PHONY: all clean

PWD_DIR = $(shell pwd)
SRC_DIR = $(PWD_DIR)/source
OBJ_DIR = $(PWD_DIR)/object
MAIN_DIR = $(PWD_DIR)/main
INC_DIR = $(PWD_DIR)/include

CC = g++ -std=c++17 -O2
CFLAG = -I$(INC_DIR) -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

export PWD_DIR SRC_DIR OBJ_DIR MAIN_DIR CC CFLAG

all:
	make -C $(SRC_DIR)
	make -C $(MAIN_DIR)
	make -C $(OBJ_DIR)

clean:
	$(RM) -rf $(OBJ_DIR)/*.o
	$(RM) application