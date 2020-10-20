#include <dirent.h>
#include <nds.h>
#include <slim.h>
#include <stdarg.h>
#include <stdio.h>

#include "bios_decompress_callback.h"
#include "fileOperations.h"
#include "gif.hpp"
#include "inifile.h"
#include "nds_loader_arm9.h"
#include "tonccpy.h"
#include "version.h"

#include "topLoad.h"
#include "subLoad.h"
#include "topError.h"
#include "subError.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

#define SETTINGS_INI_PATH "sd:/hiya/settings.ini"

#define TMD_SIZE 0x208

static char tmdBuffer[TMD_SIZE];

bool splash = true;
bool dsiSplash = false;
bool titleAutoboot = false;

bool splashFound[2] = {false};
bool splashBmp[2] = {false};

Gif gif[2];

bool loadBMP(bool top) {
	FILE* file = fopen((top ? "sd:/hiya/splashtop.bmp" : "sd:/hiya/splashbottom.bmp"), "rb");
	if (!file)
		return false;

	// Read width & height
	fseek(file, 0x12, SEEK_SET);
	u32 width, height;
	fread(&width, 1, sizeof(width), file);
	fread(&height, 1, sizeof(height), file);

	if (width > 256 || height > 192) {
		fclose(file);
		return false;
	}

	fseek(file, 0xE, SEEK_SET);
	u8 headerSize = fgetc(file);
	bool rgb565 = false;
	if(headerSize == 0x38) {
		// Check the upper byte green mask for if it's got 5 or 6 bits
		fseek(file, 0x2C, SEEK_CUR);
		rgb565 = fgetc(file) == 0x07;
		fseek(file, headerSize - 0x2E, SEEK_CUR);
	} else {
		fseek(file, headerSize - 1, SEEK_CUR);
	}
	u16 *bmpImageBuffer = new u16[width * height];
	fread(bmpImageBuffer, 2, width * height, file);
	u16 *dst = (top ? BG_GFX : BG_GFX_SUB) + ((191 - ((192 - height) / 2)) * 256) + (256 - width) / 2;
	u16 *src = bmpImageBuffer;
	for (uint y = 0; y < height; y++, dst -= 256) {
		for (uint x = 0; x < width; x++) {
			u16 val = *(src++);
			*(dst + x) = ((val >> (rgb565 ? 11 : 10)) & 0x1F) | ((val >> (rgb565 ? 1 : 0)) & (0x1F << 5)) | (val & 0x1F) << 10 | BIT(15);
		}
	}

	delete[] bmpImageBuffer;
	fclose(file);
	return true;
}

void bootSplashInit() {
	// Initialize bitmap background
	videoSetMode(MODE_3_2D);
	videoSetModeSub(MODE_3_2D);
	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);
	// Clear these to prevent messing up the main BG
	vramSetBankH(VRAM_H_LCD);
	vramSetBankG(VRAM_G_LCD);

	if (splashFound[true] && splashBmp[true])
		bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	else
		bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);

	if (splashFound[false] && splashBmp[false])
		bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	else
		bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);

	// Clear backgrounds
	BG_PALETTE[0xFF] = 0xFFFF;
	BG_PALETTE_SUB[0xFF] = 0xFFFF;
	toncset16(BG_GFX, 0xFFFF, 256 * 256 * 2);
	toncset16(BG_GFX_SUB, 0xFFFF, 256 * 256 * 2);
}

void loadScreen() {
	bootSplashInit();

	// Display Load Screen
	if (splashBmp[true]) {
		loadBMP(true);
	} else if (!splashFound[true]) {
		tonccpy(&BG_PALETTE[0], topLoadPal, topLoadPalLen);
		swiDecompressLZSSVram((void*)topLoadBitmap, BG_GFX, 0, &decompressBiosCallback);
	}
	if (splashBmp[false]) {
		loadBMP(false);
	} else if (!splashFound[false]) {
		tonccpy(&BG_PALETTE_SUB[0], subLoadPal, subLoadPalLen);
		swiDecompressLZSSVram((void*)subLoadBitmap, BG_GFX_SUB, 0, &decompressBiosCallback);
	}
}

int cursorPosition = 0;

void loadSettings(void) {
	// GUI
	CIniFile settingsini(SETTINGS_INI_PATH);

	splash = settingsini.GetInt("HIYA-CFW", "SPLASH", 0);
	dsiSplash = settingsini.GetInt("HIYA-CFW", "DSI_SPLASH", 0);
	titleAutoboot = settingsini.GetInt("HIYA-CFW", "TITLE_AUTOBOOT", 0);
}

void saveSettings(void) {
	// GUI
	CIniFile settingsini(SETTINGS_INI_PATH);

	settingsini.SetInt("HIYA-CFW", "SPLASH", splash);
	settingsini.SetInt("HIYA-CFW", "DSI_SPLASH", dsiSplash);
	settingsini.SetInt("HIYA-CFW", "TITLE_AUTOBOOT", titleAutoboot);
	settingsini.SaveIniFile(SETTINGS_INI_PATH);
}

void setupConsole() {
	// Subscreen as a console
	videoSetMode(MODE_0_2D);
	vramSetBankG(VRAM_G_MAIN_BG);
	videoSetModeSub(MODE_0_2D);
	vramSetBankH(VRAM_H_SUB_BG);
}

int main( int argc, char **argv) {
	// defaultExceptionHandler();

	if (!fatInitDefault()) {
		bootSplashInit();

		// Display Error Screen
		swiDecompressLZSSVram((void*)topErrorBitmap, BG_GFX, 0, &decompressBiosCallback);
		swiDecompressLZSSVram((void*)subErrorBitmap, BG_GFX_SUB, 0, &decompressBiosCallback);
		tonccpy(&BG_PALETTE[0], topErrorPal, topErrorPalLen);
		tonccpy(&BG_PALETTE_SUB[0], subErrorPal, subErrorPalLen);

		while (1)
			swiWaitForVBlank();
	}

	loadSettings();

	bool gotoSettings = (access("sd:/hiya/settings.ini", F_OK) != 0);

	scanKeys();

	if (keysHeld() & KEY_SELECT) gotoSettings = true;

	if (gotoSettings) {
		setupConsole();

		int pressed = 0;
		bool menuprinted = true;

		while (1) {
			if (menuprinted) {
				consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, true, true);
				consoleClear();

				printf ("\x1B[46m");

				printf("HiyaCFW %s%cconfiguration\n", VER_NUMBER, sizeof(VER_NUMBER) > 11 ? '\n' : ' ');
				printf("Press A to select, START to save");
				printf("\n");

				printf ("\x1B[47m");

				if (cursorPosition == 0) printf ("\x1B[41m");
				else printf ("\x1B[47m");

				printf(" Splash: ");
				if (splash)
					printf("Off( ), On(x)");
				else
					printf("Off(x), On( )");
				printf("\n\n");

				if (cursorPosition == 1) printf ("\x1B[41m");
				else printf ("\x1B[47m");

				if (dsiSplash)
					printf(" (x)");
				else
					printf(" ( )");
				printf(" DSi Splash/H&S screen\n");

				if (cursorPosition == 2) printf ("\x1B[41m");
				else printf ("\x1B[47m");

				if (titleAutoboot)
					printf(" (x)");
				else
					printf(" ( )");
				printf(" Autoboot title\n");

				consoleInit(NULL, 1, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);
				consoleClear();

				printf("\n");
				if (cursorPosition == 0) {
					printf(" Enable splash screen.");
				} else if (cursorPosition == 1) {
					printf(" Enable showing the DSi Splash/\n");
					printf(" Health & Safety screen.");
				} else if (cursorPosition == 2) {
					printf(" Load title contained in\n");
					printf(" sd:/hiya/autoboot.bin\n");
					printf(" instead of the DSi Menu.");
				}

				menuprinted = false;
			}

			do {
				scanKeys();
				pressed = keysDownRepeat();
				swiWaitForVBlank();
			} while (!pressed);

			if (pressed & KEY_L) {
				// Debug code
				FILE* ResetData = fopen("sd:/hiya/ResetData_extract.bin","wb");
				fwrite((void*)0x02000000,1,0x800,ResetData);
				fclose(ResetData);
				for (int i = 0; i < 30; i++) swiWaitForVBlank();
			}

			if (pressed & KEY_A) {
				switch (cursorPosition){
					case 0:
					default:
						splash = !splash;
						break;
					case 1:
						dsiSplash = !dsiSplash;
						break;
					case 2:
						titleAutoboot = !titleAutoboot;
						break;
				}
				menuprinted = true;
			}

			if (pressed & KEY_UP) {
				cursorPosition--;
				menuprinted = true;
			} else if (pressed & KEY_DOWN) {
				cursorPosition++;
				menuprinted = true;
			}

			if (cursorPosition < 0) cursorPosition = 2;
			if (cursorPosition > 2) cursorPosition = 0;

			if (pressed & KEY_START) {
				saveSettings();
				break;
			}
		}
	}

	// Create dummy file
	// Check the free space
	struct statvfs st;
	statvfs("sd:/", &st);
	u32 freeSpace = st.f_bsize * st.f_bfree;

	// If the free space is bigger than 2GiB (using a u32 so always 0 - 4GiB)
	if(freeSpace > (2u << 30)) {
		consoleDemoInit();
		size_t oldSize = 0;
		// Check old dummy file size to see if it can just be removed
		FILE *file = fopen("sd:/hiya/dummy.bin", "rb");
		if(file) {
			fseek(file, 0, SEEK_END);
			oldSize = ftell(file);
			fclose(file);
		}

		// Check that dummy file is still needed
		if((freeSpace + oldSize) > (2u << 30)) {
			// Make sure hiya directory exists and make the file
			mkdir("sd:/hiya", 0777);
			printf("Making new dummy file...   ");
			// Make sure the file exists
			file = fopen("sd:/hiya/dummy.bin", "wb");
			if(file)
				fclose(file);
			// Free space - 2GB + 10MB
			truncate("sd:/hiya/dummy.bin", (freeSpace - (2 << 30)) + 10000000);
			printf("Done!\n");
		} else {
			printf("Removing old dummy file... ");
			remove("sd:/hiya/dummy.bin");
			printf("Done!\n");
		}
	}

	if (!gotoSettings && (*(u32*)0x02000300 == 0x434E4C54)) {
		// if "CNLT" is found, then don't show splash
		splash = false;
	}

	if ((*(u32*)0x02000300 == 0x434E4C54) && (*(u32*)0x02000310 != 0x00000000)) {
		// if "CNLT" is found, and a title is set to launch, then don't autoboot title in "autoboot.bin"
		titleAutoboot = false;
	}

	if (titleAutoboot) {
		FILE* ResetData = fopen("sd:/hiya/autoboot.bin","rb");
		if (ResetData) {
			fread((void*)0x02000300,1,0x20,ResetData);
			dsiSplash = false;	// Disable DSi splash, so that DSi Menu doesn't appear
			fclose(ResetData);
		}
	}

	if (splash) {
		if (gif[true].load(true)) {
			splashFound[true] = true;
			splashBmp[true] = false;
		} else if (access("sd:/hiya/splashtop.bmp", F_OK) == 0) {
			splashFound[true] = true;
			splashBmp[true] = true;
		}

		if (gif[false].load(false)) {
			splashFound[false] = true;
			splashBmp[false] = false;
		} else if (access("sd:/hiya/splashtop.bmp", F_OK) == 0) {
			splashFound[false] = true;
			splashBmp[false] = true;
		}

		loadScreen();

		timerStart(0, ClockDivider_1024, TIMER_FREQ_1024(100), Gif::timerHandler);

		// If both GIFs will loop forever (or are not loaded)
		// then show for 3s
		if (gif[true].loopForever() && gif[false].loopForever()) {
			for (int i = 0; i < 60 * 3; i++)
				swiWaitForVBlank();
		} else {
			while (!(gif[true].finished() && gif[false].finished())) {
				swiWaitForVBlank();
				scanKeys();
				u16 down = keysDown();

				for (auto &g : gif) {
					if (g.waitingForInput() && down)
						g.resume();
				}
			}
		}
		timerStop(0);
	}

	if (!dsiSplash) {
		fifoSendValue32(FIFO_USER_03, 1);
		// Tell Arm7 to check FIFO_USER_03 code
		fifoSendValue32(FIFO_USER_04, 1);
		// Small delay to ensure arm7 has time to write i2c stuff
		for (int i = 0; i < 1*3; i++) { swiWaitForVBlank(); }
	} else {
		fifoSendValue32(FIFO_USER_04, 1);
	}

	char tmdpath[256];
	for (u8 i = 0x41; i <= 0x5A; i++) {
		snprintf (tmdpath, sizeof(tmdpath), "sd:/title/00030017/484e41%x/content/title.tmd", i);
		if (access(tmdpath, F_OK) == 0) {
			break;
		}
	}

	FILE* f_tmd = fopen(tmdpath, "rb");
	if (f_tmd) {
		if (getFileSize(tmdpath) > TMD_SIZE) {
			// Read big .tmd file at the correct size
			f_tmd = fopen(tmdpath, "rb");
			fread(tmdBuffer, 1, TMD_SIZE, f_tmd);
			fclose(f_tmd);

			// Write correct sized .tmd file
			f_tmd = fopen(tmdpath, "wb");
			fwrite(tmdBuffer, 1, TMD_SIZE, f_tmd);
			fclose(f_tmd);
		}
		int err = runNdsFile("sd:/hiya/BOOTLOADER.NDS", 0, NULL);
		setupConsole();
		consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, true, true);
		consoleClear();
		printf ("Start failed. Error %i\n", err);
		if (err == 1) printf ("bootloader.nds not found!");
		consoleInit(NULL, 1, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);
		consoleClear();
	} else {
		setupConsole();
		consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, true, true);
		consoleClear();
		printf("Error!\n");
		printf("\n");
		printf("Launcher's title.tmd was\n");
		printf("not found!\n");
		consoleInit(NULL, 1, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);
		consoleClear();
	}

	while (1)
		swiWaitForVBlank();
}

