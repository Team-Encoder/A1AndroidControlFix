#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>
#include <time.h>
#include <syslog.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#include "uinput_gamepad.h"

#include "android/log.h"
static const char *TAG="TEFIX";
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO,  TAG, fmt, ##args)
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, TAG, fmt, ##args)

static speed_t getBaudrate(int baudrate)
{
	switch(baudrate) {
	case 0: return B0;
	case 50: return B50;
	case 75: return B75;
	case 110: return B110;
	case 134: return B134;
	case 150: return B150;
	case 200: return B200;
	case 300: return B300;
	case 600: return B600;
	case 1200: return B1200;
	case 1800: return B1800;
	case 2400: return B2400;
	case 4800: return B4800;
	case 9600: return B9600;
	case 19200: return B19200;
	case 38400: return B38400;
	case 57600: return B57600;
	case 115200: return B115200;
	case 230400: return B230400;
	case 460800: return B460800;
	case 500000: return B500000;
	case 576000: return B576000;
	case 921600: return B921600;
	case 1000000: return B1000000;
	case 1152000: return B1152000;
	case 1500000: return B1500000;
	case 2000000: return B2000000;
	case 2500000: return B2500000;
	case 3000000: return B3000000;
	case 3500000: return B3500000;
	case 4000000: return B4000000;
	default: return -1;
	}
}
#define TRACK_BALL
//#define DEBUG_CONTROL_BUFFER
//#define LOG_CONTROL_INPUT
#define TYPE_4PLAYER
#define TYPE_4PLAYER_4BUTTON

//#define TYPE_2PLAYER

#ifdef TYPE_2PLAYER
    #define UMICOUNT 14
    #define SERIAL_INPUT_COUNT 18
    #define TTYPATH "/dev/ttyMT2"
    #define CAB_PLAYERCOUNT 2
    #define WRITE_MICROSECONDS 17
    #define READ_MICROSECONDS 14
#endif

#ifdef TRACK_BALL
	#define TRACKBALL_TTY "/dev/ttyS2"
	#define TRACKBALL_SERIAL_INPUT_COUNT 128
	#define TRACKBALL_UMICOUNT 4
    #define TRACKBALL_READ_MS 16
    #define TRACKBALL_UMIDEV "/dev/umidokey3"
    typedef unsigned char u8;
    typedef unsigned short u16;
    typedef unsigned int u32;

    typedef struct {
        u8 partial_buff[4];
        u8 recv_buff[128];
    } pkt_buffs_t;

    typedef struct {
        u16 flags;
        u16 stopFlag;
        int fd;
        pthread_t threadHandle;
        int upCnt;
        int downCnt;
        int leftCnt;
        int rightCnt;
    } TrackBallData_t;


    #define TRACKBALL_TIMEOUT	16
    #define TRACKBALL_UP		0x0001
    #define TRACKBALL_DOWN		0x00FE
    #define TRACKBALL_LEFT		0x0100
    #define TRACKBALL_RIGHT		0xFE00

    volatile TrackBallData_t g_trackball_data = { 0 };

#endif

#ifdef TYPE_4PLAYER
    #define UMICOUNT 26
    #define SERIAL_INPUT_COUNT 18
    #define TTYPATH "/dev/ttyS1"
    #define CAB_PLAYERCOUNT 4
    #define WRITE_MICROSECONDS 17
    #define READ_MICROSECONDS 14
#endif

int fd = 0;
int umifd = 0;
int trackball_fd = 0;
int trackball_umi_fd = 0;

char queryKey[4] = {static_cast<char>(-90),1,0};
signed char keyBuffer[0x40] = {0};
signed char umiBuffer[UMICOUNT] = {0x30};

#ifdef TRACK_BALL
signed char TrackBallUMIBuffer[TRACKBALL_UMICOUNT] = {0x30};
#endif


#ifdef TYPE_4PLAYER
char umiKeys[12] = {4,5,6,8,9,0x0a,2,0xc,0xd,0xe,0x10,0x11};
#endif

#ifdef TYPE_2PLAYER
char umiKeys[7] = {4,5,6,8,9,0x0a,2};
#endif

#ifdef TRACK_BALL

typedef struct {
	unsigned char partial_buff[4];
	unsigned char full_buff[128];
} buffs_t;


#endif

pthread_mutex_t keyMutex;
pthread_cond_t keyMsg;

extern int errno;

void write_log_thread1(const char* Msg)
{
    FILE* fh = fopen("/sdcard/ControlFix.log","a+");
    fwrite(Msg,strlen(Msg),1,fh);
    fflush(fh);
    fclose(fh);
}

void write_log_thread2(const char* Msg)
{
    FILE* fh = fopen("/sdcard/ControlFix2.log","a+");
    fwrite(Msg,strlen(Msg),1,fh);
    fflush(fh);
    fclose(fh);
}

void write_log_thread3(const char* Msg)
{
    FILE* fh = fopen("/sdcard/ControlFix3.log","a+");
    fwrite(Msg,strlen(Msg),1,fh);
    fflush(fh);
    fclose(fh);
}


#ifdef TYPE_2PLAYER
	#define PLAYER_BUTTON_A_BIT 4
	#define PLAYER_BUTTON_B_BIT 1
	#define PLAYER_BUTTON_C_BIT 2
	#define PLAYER_BUTTON_X_BIT 3
	#define PLAYER_BUTTON_Y_BIT 0
	#define PLAYER_BUTTON_Z_BIT 5
#endif

#ifdef TYPE_4PLAYER
	#define PLAYER_BUTTON_A_BIT 4
	#define PLAYER_BUTTON_B_BIT 1
	#define PLAYER_BUTTON_C_BIT 2
	#define PLAYER_BUTTON_X_BIT 3
	#define PLAYER_BUTTON_Y_BIT 0
	#define PLAYER_BUTTON_Z_BIT 5
#endif

#define PLAYER_START_BIT 0x05
#define SYSTEM_VOLUME_UP_BIT 1
#define SYSTEM_VOLUME_DOWN_BIT 0
#define SYSTEM_LIVE_BIT 4

//char umiKeys[7] = {4,5,6,8,9,0x0a,2};
//char umiKeys[12] = {4,5,6,8,9,0x0a,2,0xc,0xd,0xe,0x10,0x11};

#define SYSTEM_KEY 0x02


#define PLAYER_UP_BIT 3
#define PLAYER_DOWN_BIT 1
#define PLAYER_RIGHT_BIT 0
#define PLAYER_LEFT_BIT 2

#define PLAYER1_DIRECTION 0x04
#define PLAYER2_DIRECTION 0x08
#define PLAYER3_DIRECTION 0x0C
#define PLAYER4_DIRECTION 0x10

#define PLAYER1_BUTTON 0x05
#define PLAYER2_BUTTON 0x09
#define PLAYER3_BUTTON 0x0D
#define PLAYER4_BUTTON 0x11

char playerDirectionBits[5] = { PLAYER_LEFT_BIT, PLAYER_RIGHT_BIT, PLAYER_DOWN_BIT, PLAYER_UP_BIT, PLAYER_START_BIT };
char playerButtonBits[6] = { PLAYER_BUTTON_A_BIT, PLAYER_BUTTON_B_BIT, PLAYER_BUTTON_C_BIT, PLAYER_BUTTON_X_BIT, PLAYER_BUTTON_Y_BIT, PLAYER_BUTTON_Z_BIT };
char playerDirection[4] = { PLAYER1_DIRECTION, PLAYER2_DIRECTION, PLAYER3_DIRECTION, PLAYER4_DIRECTION };
char playerButton[4] = { PLAYER1_BUTTON, PLAYER2_BUTTON, PLAYER3_BUTTON, PLAYER4_BUTTON};

#define GPADSNUM 4
UINP_GPAD_DEV uinp_gpads[GPADSNUM];
UINP_GPAD_DEV uinp_mouse[1];

void outputKeyPress(short keyPad, int keyCode, int state) {
    uinput_gpad_write(&uinp_gpads[ keyPad ], keyCode, state, EV_KEY);
}

void outputAxisChange(short keyPad, int axisCode, int value) {
    uinput_gpad_write(&uinp_gpads[ keyPad ], axisCode, value, EV_ABS);
}

bool isStockAppRunning = false;

void OpenUMIDoKey()
{
	umifd = open("/dev/umidokey", O_WRONLY);
	if(umifd == NULL)
	{
		LOGE("ERROR: Could not open umidokey!");
		write_log_thread2("ERROR: Could not open umidokey for writing!\n");
		exit(0);
	}
	else{
		LOGD("umidokey opened successfully!");
        write_log_thread2("umidokey opened successfully!\n");
    }
}

using namespace std;
bool IsForeGroundStockApp() {
	char buffer[128];

	memset(buffer,0,128);

	FILE* pipe = popen("dumpsys activity recents | grep 'Recent #0' | cut -d= -f2 | sed 's| .*||' | cut -d '/' -f1", "r");
	if (!pipe) {
		LOGE("IsStockApp Check - popen failed!\n");
		return false;
	}

	while (!feof(pipe)) {
		fgets(buffer, 128, pipe);
	}

	pclose(pipe);

#ifdef LOG_CONTROL_INPUT
	LOGI("IsForeGroundStockApp() Buffer: %s\n",buffer);
#endif

	if(strcmp(buffer,"com.arcade\n") == 0 || strcmp(buffer,"umido.androidemu.mame\n") == 0 || strcmp(buffer,"com.arcade.tmnt") == 0) {
#ifdef LOG_CONTROL_INPUT
	    LOGI("IsForeGroundStockApp() com.arcade true");
#endif
        return true;
    }
	else {
#ifdef LOG_CONTROL_INPUT
	    LOGI("IsForeGroundStockApp() com.arcade false");
#endif
        return false;
    }
}

void WriteUmiDoKey()
{

	int writeData = write(umifd,umiBuffer,UMICOUNT);

    if(umifd == NULL)
    {
        LOGE("ERROR: umihf == NULL when calling WriteUmiDoKey()!\n");
    }

	if(writeData == -1)
	{
		LOGE("ERROR: Encountered error writing to umidokey!\n");
	}

	memset(umiBuffer,0,UMICOUNT);
}

_Noreturn void *CheckStockAppRunning(void* arg)
{
	while(1)
	{
		bool StockRunningStatus = false;

		/* run logic to check if StockApp is running*/

        StockRunningStatus = IsForeGroundStockApp();
		//LOGI("CheckStockAppRunning() - %d",StockRunningStatus);

		if(StockRunningStatus != isStockAppRunning) {
			pthread_mutex_lock(&keyMutex);
                isStockAppRunning = StockRunningStatus;
			pthread_mutex_unlock(&keyMutex);
		}

		sleep(5);
	}
}

#ifdef TRACK_BALL
void trackball_get_xy(int* pX, int* pY)
{
    int u = g_trackball_data.upCnt;
    int d = g_trackball_data.downCnt;
    int l = g_trackball_data.leftCnt;
    int r = g_trackball_data.rightCnt;
    *pX = (l != 0) ? -l : r;
    *pY = (u != 0) ? -u : d;
}
#endif

_Noreturn void *TrackBallMousePush(void* arg)
{
	//int16_t uinput_mouse_open(UINP_GPAD_DEV* const gpad, unsigned char number);
	int16_t mouse_ret = uinput_mouse_open(&uinp_mouse[0]);
	if( mouse_ret == -1)
	{
		LOGE("uinput_mouse_open FAILED -1\n");
		write_log_thread2("uinput_mouse_open FAILED!\n");
		printf("uinput_mouse_open FAILED!\n");
	}

	if( mouse_ret == -2 )
	{
		LOGE("uinput_mouse_open FAILED -2\n");
		write_log_thread2("uinput_mouse_open CREATE DEVICE FAILED!\n");
		printf("uinput_mouse_open CREATE DEVICE FAILED!\n");
	}

	while(1)
    {
        int x = 0;
        int y = 0;
        trackball_get_xy(&x,&y);
        if(x != 0 || y != 0) {
			uinput_mouse_write(&uinp_mouse[0], x, y);
			LOGI("Writing new mouse input x: %i, y: %i\n",x,y);
		}
        usleep(1000);
    }
}

_Noreturn void *SerialKeyHandle(void* arg) {
	LOGI("KeyHandler Thread Started...");
    write_log_thread2("[Team-Encoder_FIX] - Key Handler Thread Started...\n");
	printf("SerialKeyHandle Started..\n");

	int16_t ginput1_ret = uinput_gpad_open(&uinp_gpads[0],1);
    if( ginput1_ret == -1)
	{
        LOGE("uinput_gpad_open FAILED -1\n");
    	write_log_thread2("uinput_gpad_open FAILED!\n");
    	printf("uinput_gpad_open FAILED!\n");
	}

    if( ginput1_ret == -2 )
	{
        LOGE("uinput_gpad_open FAILED -2\n");
        write_log_thread2("uinput_gpad_open CREATE DEVICE FAILED!\n");
    	printf("uinput_gpad_open CREATE DEVICE FAILED!\n");
	}

    uinput_gpad_open(&uinp_gpads[1],2);

#ifdef TYPE_4PLAYER
    uinput_gpad_open(&uinp_gpads[2],3);
    uinput_gpad_open(&uinp_gpads[3],4);
#endif

    OpenUMIDoKey();

	while (1) {
		pthread_mutex_lock(&keyMutex);
		pthread_cond_wait(&keyMsg, &keyMutex);

#ifdef DEBUG_CONTROL_BUFFER
		LOGI("Keys: \n");
		for(int i =0; i<sizeof(keyBuffer);i++)
		{
			LOGI("%02X,",keyBuffer[i]);
		}
		LOGI("\n");
#endif
		//UMIDOKEY
		if(isStockAppRunning) {

			for(int i=0;i<UMICOUNT;i++)
			{
				umiBuffer[i] = '0';
			}

			for(int i2=0;i2<sizeof(umiKeys);i2++)
			{
				int idx = umiKeys[i2];

				if(keyBuffer[idx] > 0x09)
					umiBuffer[ ( i2 * 2 ) ] = ((keyBuffer[idx] >> 4) + '0');
				if(keyBuffer[idx] != 0)
					umiBuffer[ ( i2 * 2 ) +1 ] = ((keyBuffer[idx] & 0xF) + '0');

				if((keyBuffer[idx] & 0xF) >= 0x0A)
                {
				    switch ( (keyBuffer[idx] & 0xF) )
                    {
                        case 0x0A:
                            umiBuffer[ (i2*2 ) +1 ] = 'A';
                            break;
                        case 0x0B:
                            umiBuffer[ (i2*2) +1 ] = 'B';
                            break;
                        case 0x0C:
                            umiBuffer[ (i2*2) +1 ] = 'C';
                            break;
                        case 0x0D:
                            umiBuffer[ (i2*2) +1] = 'D';
                            break;
                        case 0x0E:
                            umiBuffer[ (i2*2) +1] = 'E';
                            break;
                        case 0x0F:
                            umiBuffer[ (i2*2)+1] = 'F';
                            break;
                    }
                }

			}

			//LOGI("umiBuffer Output: %s\n",umiBuffer);
			WriteUmiDoKey();
		}
		else{

            for(int i =0; i<CAB_PLAYERCOUNT;i++)
			{
				if( ( (keyBuffer[playerDirection[i]] >> PLAYER_UP_BIT) & 1 ) == 1)
				{
#ifdef LOG_CONTROL_INPUT
                    LOGI("Player %i pressing up!\n",i);
#endif
					outputAxisChange(i,ABS_HAT0Y, -1);
				}
				else if( ( (keyBuffer[playerDirection[i]] >> PLAYER_DOWN_BIT) & 1 ) == 1)
				{
#ifdef LOG_CONTROL_INPUT
                    LOGI("Player %i pressing down!\n",i);
#endif
					outputAxisChange(i,ABS_HAT0Y,1);
				}else{
					outputAxisChange(i,ABS_HAT0Y,0);
				}


				if( ( (keyBuffer[playerDirection[i]] >> PLAYER_LEFT_BIT) & 1 ) == 1)
				{
#ifdef LOG_CONTROL_INPUT
                    LOGI("Player %i pressing left!\n",i);
#endif
					outputAxisChange(i,ABS_HAT0X,-1);
				}
				else if( ( (keyBuffer[playerDirection[i]] >> PLAYER_RIGHT_BIT) & 1 ) == 1)
				{
#ifdef LOG_CONTROL_INPUT
                    LOGI("Player %i pressing right!\n",i);
#endif
					outputAxisChange(i,ABS_HAT0X,1);
				}else{
					outputAxisChange(i,ABS_HAT0X,0);
				}



				if( ( (keyBuffer[playerDirection[i]] >> PLAYER_START_BIT) & 1 ) == 1)
				{
#ifdef LOG_CONTROL_INPUT
                    LOGI("Player %i pressing start!\n",i);
#endif

					outputKeyPress(i,BTN_START,1);
				}else{
					outputKeyPress(i,BTN_START,0);
				}

#ifdef TYPE_2PLAYER
				if( ( (keyBuffer[playerButton[i]] >> PLAYER_BUTTON_A_BIT) & 1 ) == 1)
				{
					LOGI("Player %i pressing A!\n",i);
					outputKeyPress(i,BTN_A,1);
				}else{
					outputKeyPress(i,BTN_A,0);
				}

				if( ( (keyBuffer[playerButton[i]] >> PLAYER_BUTTON_B_BIT) & 1 ) == 1)
				{
					LOGI("Player %i pressing B!\n",i);
					outputKeyPress(i,BTN_B,1);
				}else{
					outputKeyPress(i,BTN_B,0);
				}


				if( ( (keyBuffer[playerButton[i]] >> PLAYER_BUTTON_C_BIT) & 1 ) == 1)
				{
					LOGI("Player %i pressing C!\n",i);
					outputKeyPress(i,BTN_C,1);
				}else{
					outputKeyPress(i,BTN_C,0);
				}

				if( ( (keyBuffer[playerButton[i]] >> PLAYER_BUTTON_X_BIT) & 1 ) == 1)
				{
					LOGI("Player %i pressing X!\n",i);
					outputKeyPress(i,BTN_X,1);
				}else{
					outputKeyPress(i,BTN_X,0);
				}

				if( ( (keyBuffer[playerButton[i]] >> PLAYER_BUTTON_Y_BIT) & 1 ) == 1)
				{
					LOGI("Player %i pressing Y!\n",i);
					outputKeyPress(i,BTN_Y,1);
				}else{
					outputKeyPress(i,BTN_Y,0);
				}

				if( ( (keyBuffer[playerButton[i]] >> PLAYER_BUTTON_Z_BIT) & 1 ) == 1)
				{
					LOGI("Player %i pressing Z!\n",i);
					outputKeyPress(i,BTN_Z,1);
				}else {
                    outputKeyPress(i, BTN_Z, 0);
                }
#endif

#ifdef TYPE_4PLAYER_4BUTTON
                if( ( (keyBuffer[playerButton[i]] >> PLAYER_BUTTON_A_BIT) & 1 ) == 1)
				{
#ifdef LOG_CONTROL_INPUT
                    LOGI("Player %i pressing X ( A bit )!\n",i);
#endif
					outputKeyPress(i,BTN_X,1);
				}else{
					outputKeyPress(i,BTN_X,0);
				}

				if( ( (keyBuffer[playerButton[i]] >> PLAYER_BUTTON_C_BIT) & 1 ) == 1)
				{
#ifdef LOG_CONTROL_INPUT
                    LOGI("Player %i pressing Y (C bit)!\n",i);
#endif

                    outputKeyPress(i,BTN_Y,1);
				}else{
					outputKeyPress(i,BTN_Y,0);
				}
#endif

#ifdef TYPE_4PLAYER
                if( ( (keyBuffer[playerButton[i]] >> PLAYER_BUTTON_X_BIT) & 1 ) == 1)
                {
#ifdef LOG_CONTROL_INPUT
                    LOGI("Player %i pressing A (X bit)!\n",i);
#endif
                    outputKeyPress(i,BTN_A,1);
                }else{
                    outputKeyPress(i,BTN_A,0);
                }

                if( ( (keyBuffer[playerButton[i]] >> PLAYER_BUTTON_Y_BIT) & 1 ) == 1)
                {
#ifdef LOG_CONTROL_INPUT
                    LOGI("Player %i pressing B (Y bit)!\n",i);
#endif
                    outputKeyPress(i,BTN_B,1);
                }else{
                    outputKeyPress(i,BTN_B,0);
                }
#endif
			} // End of per player keys


			// Global System Keys
			if( ( (keyBuffer[SYSTEM_KEY] >> SYSTEM_LIVE_BIT) & 1 ) == 1)
			{
#ifdef LOG_CONTROL_INPUT
                LOGI("Player pressing live!\n");
#endif

                outputKeyPress(0, BTN_SELECT, 1);
			} else{
                outputKeyPress(0, BTN_SELECT, 0);
            }

			if( ( (keyBuffer[SYSTEM_KEY] >> SYSTEM_VOLUME_UP_BIT) & 1 ) == 1)
			{
#ifdef LOG_CONTROL_INPUT
                LOGI("Player pressing volume up!\n");
#endif
				outputKeyPress(0, KEY_VOLUMEUP, 1);
			} else {
				outputKeyPress( 0, KEY_VOLUMEUP, 0);
			}

			if( (keyBuffer[SYSTEM_KEY] >> SYSTEM_VOLUME_DOWN_BIT) == 0)
			{
#ifdef LOG_CONTROL_INPUT
				LOGI("Player pressing volume down!\n");
#endif
				outputKeyPress(0, KEY_VOLUMEDOWN, 1);
			} else {
				outputKeyPress( 0, KEY_VOLUMEDOWN, 0);
			}


		}

        pthread_mutex_unlock(&keyMutex);
    }
}


#ifdef TRACK_BALL


int trackball_read(int fddev, u8* pBuff, int buffSize, int timeoutMS)
{
	struct timeval timeout;
	timeout.tv_usec = 1000 * timeoutMS;
	timeout.tv_sec = timeoutMS / 1000;

	fd_set readfds;
	FD_ZERO(&readfds);
	//__FD_SET_chk(fddev, &readfds, 128);
	FD_SET(fd, &readfds);

	int sel_ret = select(fddev + 1, &readfds, NULL, NULL, &timeout);
	if (sel_ret >= 1)
		return read(fddev, pBuff, buffSize);
	if ( sel_ret )
	{
		return -1;
	}
	return 0;
}



_Noreturn void* trackball_read_and_parse_pkts_thread(void* param)
{
	pkt_buffs_t pkt_buffs = { 0 };
	int partial_size = 0;
	while (!g_trackball_data.stopFlag)
	{
		// read raw data from trackball
		int recv_size = trackball_read(g_trackball_data.fd, pkt_buffs.recv_buff,
			sizeof(pkt_buffs.recv_buff), TRACKBALL_TIMEOUT);
		if (recv_size <= 0)
		{
			// this is what the libarcade code does
			partial_size = 0;
			g_trackball_data.flags = 0;
			g_trackball_data.upCnt = 0;
			g_trackball_data.downCnt = 0;
			g_trackball_data.leftCnt = 0;
			g_trackball_data.rightCnt = 0;
			continue;
		}

		// parse all full pkts
		int total_size = partial_size + recv_size;
		int partial_offset = 4 - partial_size;
		u8* p_pkt = pkt_buffs.partial_buff + partial_offset;
		int pkt_off = 0;
		if (*p_pkt == 0xFF)
		{
			p_pkt++;
			pkt_off++;
		}
		while ( pkt_off < total_size )
		{
			// make sure there is 4 bytes left for a full packet
			if (total_size - pkt_off < 4)
				break;
			// get packet type and handle the type
			int pkt_type = (p_pkt[0] << 8) | p_pkt[1];
			if (pkt_type == TRACKBALL_UP)
			{
				g_trackball_data.upCnt++;
				g_trackball_data.downCnt = 0;
			}
			else if (pkt_type == TRACKBALL_DOWN)
			{
				g_trackball_data.downCnt++;
				g_trackball_data.upCnt = 0;
			}
			else if (pkt_type == TRACKBALL_LEFT)
			{
				g_trackball_data.leftCnt++;
				g_trackball_data.rightCnt = 0;
			}
			else if (pkt_type == TRACKBALL_RIGHT)
			{
				g_trackball_data.rightCnt++;
				g_trackball_data.leftCnt = 0;
			}
			p_pkt += 4;
			pkt_off += 4;
		}

		// handle partial packet data
		// this will handle data at the end of raw data
		// as well as if some partial data still exists in the partial buffer,
		// such as if there was 1 byte in it and 1 byte more data was read in.
		if (pkt_off < total_size)
		{
			partial_size = total_size - pkt_off;
			partial_offset = 4 - partial_size;
			memmove(pkt_buffs.partial_buff + partial_offset, p_pkt, partial_size);
		}
	}
}
#endif

_Noreturn void *SerialReadKey(void* arg)
{
    LOGI("ReadKey Thread started...");
    write_log_thread3("[Team-Encoder_FIX] - ReadKey Thread Started...\n");
	printf("SerialReadKey Started..\n");

	while(1)
    {
	    memset(keyBuffer,0x00,sizeof(keyBuffer));

#ifdef DEBUG_INPUT
        LOGI("ReadKeyThread before read\n");
#endif
        int readbytes = 0;
        readbytes = read(fd,keyBuffer,SERIAL_INPUT_COUNT);

#ifdef DEBUG_INPUT
        LOGI("ReadKeyThread after read (readbytes: %i)\n",readbytes);

        for(int i =0;i<readbytes;i++)
        {
            LOGI("ReadKeyThread after read keyBuffer[%i]: %02X\n",keyBuffer[i]);
        }
#endif

        if(keyBuffer[0] == 0xFFFFFFA7 && keyBuffer[1] == 0x10)
        {
            pthread_mutex_lock(&keyMutex);
            pthread_cond_signal(&keyMsg);
            pthread_mutex_unlock(&keyMutex);
        }
        usleep(15*1000);
        //usleep(14*1000);
    }
}

_Noreturn void *SerialQueryKey(void* arg) {
	LOGI("SerialQueryKey Thread started...");
    write_log_thread1("[Team-Encoder_FIX] - SerialQueryKey Thread Started...\n");
	printf("SerialQueryKeyThread Started..\n");

	speed_t speed;
	speed = getBaudrate(115200);
	if(speed == -1)
	{
		LOGE("Invalid baudrate");
		exit(0);
	}

	LOGD("Opening serial port %s with flags 0x%x",TTYPATH,O_RDWR | O_NOCTTY);
	fd = open(TTYPATH, O_RDWR | O_NOCTTY);

	LOGD("open() fd = %d",fd);
	if(fd == -1)
	{
		LOGE("Cannot open %s",TTYPATH);
        write_log_thread1("[Team-Encoder_FIX] - Could not open device!\n");
        exit(0);
	}

	struct termios cfg;
	LOGD("Configuring serial port");

	if(tcgetattr(fd,&cfg))
	{
		LOGE("tcgetattr() failed");
        write_log_thread1("[Team-Encoder_FIX] - Could not configure device!\n");

		close(fd);

        exit(0);
	}

	cfmakeraw(&cfg);
	cfsetispeed(&cfg,speed);
	cfsetospeed(&cfg,speed);

	if(tcsetattr(fd, TCSANOW, &cfg))
	{
		LOGE("tcsetattr() failed!\n");
        write_log_thread1("[Team-Encoder_FIX] - Could not configure device! #2\n");

		close(fd);

        exit(0);
	}

#ifdef TRACK_BALL
	speed_t speed_trackball;
	speed_trackball = getBaudrate(115200);

	if(speed_trackball == -1)
	{
		LOGE("Invalid baudrate");
		exit(0);
	}

	LOGD("Opening serial port %s with flags 0x%x",TRACKBALL_TTY,( O_RDONLY | O_NONBLOCK | O_NOCTTY));
	trackball_fd = open(TRACKBALL_TTY, O_RDONLY | O_NONBLOCK | O_NOCTTY); // 0x904 in bowling...
	// 00004404
	// 00000002 - O_RDWR
	// 00000400	- O_NO_CTTY
    // 00004000 - O_NONBLOCK
	// 00000000 - O_RDONLY

    LOGD("open() trackball_fd = %d",fd);

	if(trackball_fd == -1)
	{
		LOGE("Cannot open %s",TRACKBALL_TTY);
		write_log_thread1("[Team-Encoder_FIX] - Could not open device (trackball)!\n");
		exit(0);
	}

	if (fcntl(trackball_fd, F_SETFL, 0) < 0)
	{
		close(trackball_fd);
		write_log_thread1("[Team-Encoder_FIX] - Could not open device (trackball) fcntl(trackball_fd, F_SETFL, 0)!\n");
		exit(0);
	}

	struct termios trackball_cfg;
	LOGD("Configuring serial port (Trackball)");

	if(tcgetattr(trackball_fd,&trackball_cfg))
	{
		LOGE("tcgetattr() failed (Trackball)");
		write_log_thread1("[Team-Encoder_FIX] - Could not configure device (TrackBall)!\n");

		close(trackball_fd);

		exit(0);
	}

	cfmakeraw(&trackball_cfg);
	cfsetispeed(&trackball_cfg,speed_trackball);
	cfsetospeed(&trackball_cfg,speed_trackball);

	if(tcsetattr(trackball_fd, TCSANOW, &trackball_cfg))
	{
		LOGE("tcsetattr() failed!  (TrackBall)\n");
		write_log_thread1("[Team-Encoder_FIX] - Could not configure device! #2  (TrackBall)\n");

		close(trackball_fd);

		exit(0);
	}

	memset((void*)&g_trackball_data, 0, sizeof(g_trackball_data));
	g_trackball_data.fd = trackball_fd;


    //SerialReadTrackBall
    pthread_t trackBallReadThread;
	if(!pthread_create(&trackBallReadThread, NULL, &trackball_read_and_parse_pkts_thread, NULL))
    {
	    if(!pthread_detach(trackBallReadThread))
            LOGD("Created Trackball Read Thread Successfully!\n");
    }

	pthread_t trackBallPushThread;
    if(!pthread_create(&trackBallPushThread, NULL, &TrackBallMousePush, NULL))
    {
        if(!pthread_detach(trackBallPushThread))
        {
            LOGD("Created Trackball Push thread Successfully!\n");
        }
    }
#endif


    // Create Read Thread...
    pthread_cond_init(&keyMsg,NULL);
    pthread_mutex_init(&keyMutex,NULL);

    pthread_t keyReadThread;
    if(!pthread_create(&keyReadThread, NULL, &SerialReadKey, NULL))
    {
        if(!pthread_detach(keyReadThread))
            LOGD("Created ReadKey thread successfully.");
    }

	pthread_t keyHandlerThread;
	if(!pthread_create(&keyHandlerThread, NULL, &SerialKeyHandle, NULL))
	{
		if(!pthread_detach(keyHandlerThread))
			LOGD("Created KeyHandler thread successfully.");
	}

	pthread_t StockAppRunningThread;
	if(!pthread_create(&StockAppRunningThread, NULL, &CheckStockAppRunning, NULL))
	{
		if(!pthread_detach(StockAppRunningThread))
			LOGD("StockApp Running Thread Created Successfully.");
	}

    write_log_thread1("[Team-Encoder_FIX] - Finished creating threads and opening key device...\n");
    while(1)
    {
        if(write(fd,queryKey,3) == -1)
        {
            LOGD("Caught error while writing to the UART  - ERROR: %08X - fd:%08X",errno,fd);
        }
        usleep(17*1000);

    }

}


int main()
{
    printf("Starting Control Fix\n");

	pthread_t threadId;
	printf("Creating first thread\n");
	int err = pthread_create(&threadId, NULL, &SerialQueryKey, NULL);
	if(!err)
	{
	    printf("Detaching first thread\n");
		if(!pthread_detach(threadId))
		{
			LOGD("Created serial reading thread successfully!\n");
			printf("Created serial reading thread successfully!\n");
            write_log_thread1("Created serial reading thread successfully\n");
		}else{
		    printf("Failed to create serial reading thread!\n");
            write_log_thread1("Failed to create serial reading thread\n");
		}
	}else{
        printf("Failed to create serial reading thread!\n");
        write_log_thread1("Failed to create serial reading thread\n");
    }

    while(true)
    {
    	printf("Running...\n");
        sleep(60);
    }
}