#ifndef CONFIG
#define CONFIG
#define ENABLE_IRLIB
#ifdef ENABLE_IRLIB
    #define ENABLE_IRCAPTURE
    #define SOUND_RECORD_ENABLE
    //#define ENABLE_COM_LIB
#endif
#define IR_IMAGE_WIDTH 640
#define IR_IMAGE_HEIGHT 480
#define RECORD_VIDEO_SIZE 640 * 480 * 40 * 4
#define DATA_PACKET_SIZE 640*480*3
#define IR_STREAM
//#define FILE_STREAM
#define TEST_MODEL
//#define MODEL
#endif // CONFIG

