// these defines have no effect on the release source

// to enable decode only mode after seek
#define ENABLE_DECODE_ONLY_MODE

// to seek to requested position correctly
//#define ENABLE_ACCURATE_SEEK

// to use set play rate API
#define USE_PLAY_RATE_FUNCTION

// to deliver audio and video output to remote player
//#define ENABLE_REMOTE_PLAYER

// to deliver audio and video output to transcoder
//#define ENABLE_TRANSCODER

// to enable retrying to seek
#define USE_MEDIA_SEEK_RETRY

#define PLAYRATE_DEFAULT 	100000 // x1
#define PLAYRATE_BOOST		150000 // x1.5
#define PLAYRATE_SKIP		180000 // x1.8
#define PLAYRATE_SKIMMING 	200000 // x2
#define MIN_PLAYRATE		50000 // x0.5
#define MAX_PLAYRATE 		3200000 // x32

#define SCREENMODE_DEFAULT  2      // FULL SCREEN

#define PLAYMODE_NORMAL	0x00000000
#define PLAYMODE_SKIPPING	0x80000000
#define PLAYMODE_SKIMMING	0x40000000
#define PLAYDIR_FORWARD	0x00000000
#define PLAYDIR_BACKWARD	0x08000000
#define INSUF_PERF_FLAG	0x01000000
#define STOP_PLAYING_AUDIO  0x02000000

#define PLAYMODE_CHANGE_THRESHOLD	10
#define PLAYMODE_LATENESS_THRESHOLD	10000
