fileExtension = cpp
compiler = g++
progName = glSuchEngine

dialectFlag = -std=c++11
optionsFlags = -Wall -O3
linkFlags = -lGLEW -lGL -lSDL2 -lpthread -lpng -lz -lm


OBJDIR = obj

SRC = $(wildcard  $(addprefix ./,*.$(fileExtension)))

OBJ = $(SRC:.$(fileExtension)=.o)
OBJ := $(subst ./, ./$(OBJDIR)/, $(OBJ))


all: mkOBJDIR $(OBJ)
	@/bin/echo -e "\e[38;5;202mLinking process\t\e[0m"
	$(compiler) $(OBJ) -o $(progName) $(linkFlags)
	@/bin/echo -e "\e[38;5;190m[DONE]\n\e[0m"


run: all
	./$(progName)

cleanRun: run clean


mkOBJDIR:
	@mkdir ./$(OBJDIR) -p

$(OBJDIR)/%.o: %.$(fileExtension)
	@/bin/echo -e "\e[38;5;202m$@\t\e[0m" 
	$(compiler) $(dialectFlag) $(subst .o,.$(fileExtension),$(subst $(OBJDIR)/, ./, $@)) -c -o ./$@ $(optionsFlags) 
	@/bin/echo -e "\e[38;5;46m[OK]\n\e[0m"

clean:
	@rm -rf ./$(OBJDIR)
	@rm -f $(progName)
	@rm -f ./*~
	@rm -f ./shaders/*~


# $@ nom de la cible
# $^ nom de chaque dependance
#Compilation -c -> tous les flags sauf l edition de liens
#Compilation des .o -> seulement l edition de liens

#$(compiler) $(dialectFlag) $(SRC) -o $(progName)  $(optionsFlags) $(linkFlags)
#Dunno how to use dis shiet
#-fdiagnostics-color=always -fmessage-length=50

#0=none, 1=bold, 4=underscore, 5=blink, 7=reverse, 8=concealed
#30=black, 31=red, 32=green, 33=yellow, 34=blue, 35=magenta, 36=cyan, 37=white
#echo -e "\E[1;32mHello World"
#<Esc>[38;5;ColorNumberm foreground -- ColorNumber [1;256]
#<Esc>[48;5;ColorNumberm background
