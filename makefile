CC=gcc
CCP=g++
FLAGS=-Wno-unused-function -g -I/usr/include/lua5.3/
DEASM_FLAGS=-g -Ddgconfig_DEASSEMBLER
DEASM_OBJ=deassembler_main.o deassembler.o
EMMU_FLAGS=-w -g 
EMMU_OBJ=main_file.o cpu_8080.o cpu_common.o cli_parser.c deassembler_emu.o simple_assembler.o
SPACE_INVADERS_OBJ=SpaceInvaders.o cpu_8080.o space_invaders_common.o deassembler.o

emulator: $(EMMU_OBJ)
	$(CC) -o emulator $(EMMU_OBJ)

deassembler: $(DEASM_OBJ)
	$(CC) -o deassembler $(DEASM_OBJ)

space: $(SPACE_INVADERS_OBJ)
	g++ -o space_invaders $(SPACE_INVADERS_OBJ) -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17


SpaceInvaders.o:
	$(CC) -c $(EMMU_FLAGS) SpaceInvaders.cpp -std=c++17

space_invaders_common.o:
	$(CC) -c $(EMMU_FLAGS) space_invaders_common.c

main_file.o: main_file.c
	$(CC) -c $(EMMU_FLAGS) main_file.c

cpu_common.o: cpu_common.c
	$(CC) -c $(EMMU_FLAGS) cpu_common.c

cpu_8080.o: cpu.c
	$(CC) -c $(EMMU_FLAGS) cpu_8080.c

cli_parser.o: cli_parser.c
	$(CC) -c $(EMU_FLAGS) cli_parser.c

simple_assembler.o: simple_assembler.c
	$(CC) -c $(EMMU_FLAGS) simple_assembler.c

deassembler_emu.o: deassembler.c
	$(CC) -c $(EMMU_FLAGS) deassembler.c -o deassembler_emu.o

deassembler.o: deassembler.c
	$(CC) -c $(DEASM_FLAGS) deassembler.c

deassembler_main.o: deassembler_main.c
	$(CC) -c $(DEASM_FLAGS) deassembler_main.c

clean:
	rm -rfv *.o
	rm -rfv *.log
	rm -f deassembler
	rm -f emulator
	rm -f space_invaders

