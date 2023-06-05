VERTSHADER = $(wildcard *.vert)
FRAGSHADER = $(wildcard *.frag)
VERTSPV = $(patsubst %.vert, $(SPV_DIR)/%_vert.spv, $(VERTSHADER))
FRAGSPV = $(patsubst %.frag, $(SPV_DIR)/%_frag.spv, $(FRAGSHADER))

all: $(VERTSPV) $(FRAGSPV)
$(VERTSPV): $(SPV_DIR)/%_vert.spv: $(SHADER_DIR)/%.vert
	glslc $< -o $@
$(FRAGSPV): $(SPV_DIR)/%_frag.spv: $(SHADER_DIR)/%.frag
	glslc $< -o $@
