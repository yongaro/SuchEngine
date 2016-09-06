#variables specifiques a vulkan
VULKAN_SDK=/sdks/VulkanSDK/1.0.17.0/x86_64
LD_LIBRARY=$(VULKAN_SDK)/lib 
VK_LAYER=$(VULKAN_SDK)/etc/explicit_layer.d

#shadersExtensions = vert tesc tese geom frag comp
compiledShaderExt = spirv
shadersDir = shaders
shaders = $(wildcard ./$(shadersDir)/*.*)
#SI SEULEMENT ON POUVAIT FAIRE SIMPLEMENT DES BOUCLES AVEC INSTRUCTIONS >:C
shaders := $(patsubst %.vert, %.$(compiledShaderExt), $(shaders))
shaders := $(patsubst %.tesc, %.$(compiledShaderExt), $(shaders))
shaders := $(patsubst %.tese, %.$(compiledShaderExt), $(shaders))
shaders := $(patsubst %.geom, %.$(compiledShaderExt), $(shaders))
shaders := $(patsubst %.frag, %.$(compiledShaderExt), $(shaders))
shaders := $(patsubst %.comp, %.$(compiledShaderExt), $(shaders))
shaders := $(subst ./$(shadersDir),./$(shadersDir)/SPIR-V,$(shaders))
################################################

#variables pour la compilation C++
fileExtension = cpp
compiler = g++
progName = VkSuchEngine

dialectFlag = -std=c++11 -I$(VULKAN_SDK)/include
optionsFlags = -Wall -O3
linkFlags = -L$(VULKAN_SDK)/lib `pkg-config --static --libs glfw3` -lvulkan -lassimp


OBJDIR = obj

SRC = $(wildcard  $(addprefix ./,*.$(fileExtension)))

OBJ = $(SRC:.$(fileExtension)=.o)
OBJ := $(subst ./, ./$(OBJDIR)/, $(OBJ))
##################################################

#Couleurs d affichages ansi
darkOrange256 = \e[38;5;202m
darkPurple256 = \e[38;5;129m

reset = \e[0m
stepColor = \e[1;33m
successColor = \e[1;32m
###################################################

all: mkOBJDIR mkShadersDir $(shaders) $(OBJ)
	@/bin/echo -e "$(stepColor)Linking process$(reset)"
	$(compiler) $(OBJ) -o $(progName) $(linkFlags)
	@/bin/echo -e "$(successColor)[DONE]$(reset)\n"

run: all
	LD_LIBRARY_PATH=$(LD_LIBRARY) VK_LAYER_PATH=$(VK_LAYER) ./$(progName)

cleanRun: run clean


mkOBJDIR:
	@mkdir ./$(OBJDIR) -p

mkShadersDir:
	@mkdir ./$(shadersDir)/SPIR-V -p


$(OBJDIR)/%.o: %.$(fileExtension)
	@/bin/echo -e "$(stepColor)Compiling: ./$^$(reset)"
	$(compiler) $(dialectFlag) $(subst .o,.$(fileExtension),$(subst $(OBJDIR)/, ./, $@)) -c -o ./$@ $(optionsFlags) 
	@/bin/echo -e "$(successColor)[OK]$(reset)\n"


./$(shadersDir)/SPIR-V/%.$(compiledShaderExt): ./$(shadersDir)/%.*
	@/bin/echo -e "$(stepColor)Compiling: $^$(reset)"
	$(VULKAN_SDK)/bin/glslangValidator -V ./$^ -o ./$@




clean:
	@rm -rf ./$(OBJDIR)
	@rm -f $(progName)
	@rm -rf ./$(shadersDir)/SPIR-V
	@rm -f ./*~
	@rm -rf ./$(shadersDir)/*~


# $@ nom de la cible
# $^ nom de chaque dependance
#Compilation -c -> tous les flags sauf l edition de liens
#Compilation des .o -> seulement l edition de liens

#$(compiler) $(dialectFlag) $(SRC) -o $(progName)  $(optionsFlags) $(linkFlags)
#Dunno how to use dis shiet
#-fdiagnostics-color=always -fmessage-length=50

#0=none, 1=bold, 4=underscore, 5=blink, 7=reverse, 8=concealed
#30=black, 31=red, 32=green, 33=yellow, 34=blue, 35=magenta, 36=cyan, 37=light gray, 39 default
#90=dark gray, 91=light red, 92=light green, 93=light yellow, 94=light blue, 95=light magenta, 96=light cyan, 97=white, 

#echo -e "\E[1;32mHello World"

#<Esc>[38;5;ColorNumberm foreground -- ColorNumber [1;256]
#<Esc>[48;5;ColorNumberm background
