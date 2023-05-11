SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC))

all: $(OBJ)
$(OBJ): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -c $< -o $@ $(CFLAG)