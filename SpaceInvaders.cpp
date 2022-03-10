#define OLC_PGE_APPLICATION
#define FORCE_EXPERIMENTAL_FS
#include "olcPixelGameEngine.h"
#include "space_invaders_common.h"

#define RAM_SIZE	(16*1024)

memory_t *mem = NULL;
cpu_model_t cpu = {0};

static uint8_t shift0 = 0;
static uint8_t shift1 = 0;
static uint8_t shift_offset = 0;
	int   R = 0;
	int   S = 0;

extern FILE* pLogFile;

class Example : public olc::PixelGameEngine
{
public:

	static uint8_t InputHandler(uint8_t port) {

       		uint8_t a = 0;    
       		switch(port)    
       		{    
			case 1:
				a = (S) |(R << 1);
				break;
           		case 3:    
           		{    
               			//uint16_t v = (shift1<<8) | shift0;
				uint16_t v = shift1;
				v = v << 8;
				v |= shift0;

			        uint16_t mask = 0xF0 >> shift_offset;	
               			a = (uint8_t) ((v >> (8-shift_offset)) & 0xff);    
          
//				fprintf(pLogFile, "InputHandler: v: 0x%x a:0x%x\n", v, a);
			}    
        		break;    
       		}    
	       	
		return a;
	}

	static void OutputHandler(uint8_t port, uint8_t value) {
       		switch(port)    
       		{    
           		case 2:    
               			shift_offset = value & 0x7;    
//				fprintf(pLogFile, "OutputHandler: offset: %d 2\n", shift_offset);
               			break;    
           		case 4:    
               			shift0 = shift1;    
               			shift1 = value;    
//				fprintf(pLogFile, "OutputHandler shift_offset:0x%x shift0:0x%x shift1:0x%x\n", shift_offset, shift0, shift1);
               			break;    
       		}
	}


	Example()
		: fTime(0)
	{
		fScreenRes = 1/60;
		sAppName = "SpaceInvaders Emulator";
	}


public:
	bool OnUserDestroy() override
	{
		if(log_file)
			fclose(log_file);	
	}

	bool OnUserCreate() override
	{
		if(!(mem = create_ram_memory()))
			return false;
		
		load_rom_memory_from_file("bin_8080/invaders.full", mem, 0);
       
	        log_file = fopen("cpu1_8080.log", "w");

		if(log_file == NULL)
			return false;

		cpu.is_running = true;
		cpu.cb_in = &Example::InputHandler;
		cpu.cb_out = &Example::OutputHandler;	
	
		return true;
	}

	void DrawScreen()	{
		int x_offset = 0x2400;
		int counter = 0;
		int x_max = ScreenWidth();
		int y_max = ScreenHeight();


		for (int x = 0 ; x < 224 ; x++) {
			for(int y = 0 ; y < 255 ; y++) {
				Draw(x, y, olc::Pixel(0,0,0));
			}
		}

		// called once per frame
		for (int x = 0; x < 224 ; x++)	{
			for(int y = 255 ; y >= 0 ; y-=8) {
				for(int i = 7 ; i >= 0 ; i--) {
					int offset = 0x2400 + counter;
					uint8_t pixel = mem->memory[offset];
					if(pixel & (0x1 << i))
						Draw(x, (y-i), olc::Pixel(125, 125, 125));
				}

				counter++;

			}

		}
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		static int counter=  0;
		static bool lower_half = false;

		for(int i = 0 ; i < 1000 ; i++) {
			single_cpu_8080_cycle(log_file, mem, &cpu);
		}


		fTime += fElapsedTime;

		if(fTime >= fScreenRes)	{
			fTime = fTime - fScreenRes;
	
			if(counter >= 100) {

			if(lower_half) {
				execute_interrupt(log_file, 1, mem, &cpu);
				lower_half = false;
			}

			else {
				execute_interrupt(log_file, 2, mem, &cpu);
				lower_half = true;
			}

			} else
				counter++;

			DrawScreen();
		}

		if (GetKey(olc::Key::R).bPressed)	{
			R = 1;
		} else {
			R = 0;
		}

		if (GetKey(olc::Key::SPACE).bPressed)	{
			S = 1;
		} else {
			S = 0;
		}

		return true;
	}

	FILE* log_file;
	float fScreenRes;
	float fTime;
};


int main()
{
	Example demo;
	if (demo.Construct(224, 256, 4, 4))
		demo.Start();

	return 0;
}
