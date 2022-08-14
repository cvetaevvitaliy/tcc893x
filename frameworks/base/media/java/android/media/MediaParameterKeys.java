package android.media;

// Written by Telechips Inc. 

/**
 * @hide
 */
public class MediaParameterKeys 
{
	// List of KEY values which will be used by getParameter/setParameter
	// These values must be coincide with 'media_parameter_keys' of mediaplayer.h
	
	public static final int	KEY_PARAMETER_TIMED_TEXT_TRACK_INDEX = 1000;                // set only
	public static final int	KEY_PARAMETER_TIMED_TEXT_ADD_OUT_OF_BAND_SOURCE = 1001;     // set only

    // Streaming/buffering parameters
	public static final int	KEY_PARAMETER_CACHE_STAT_COLLECT_FREQ_MS = 1100;            // set only

    // Return a Parcel containing a single int, which is the channel count of the
    // audio track, or zero for error (e.g. no audio track) or unknown.
	public static final int	KEY_PARAMETER_AUDIO_CHANNEL_COUNT = 1200;                   // get only
	
	// Multi-audio selection
    public static final int KEY_PARAMETER_TOTAL_AUDIO_TRACK_COUNT   = 2000;             // get only
    public static final int KEY_PARAMETER_GET_CURRENT_AUDDIO_TRACK_IDX = 2001;			// get only
    public static final int KEY_PARAMETER_PREPARE_TO_GET_AUDIO_TRACK_INFO = 2002;		// set only
    public static final int KEY_PARAMETER_GET_AUDIO_LANGUAGE_INFO = 2003;				// get only
    public static final int KEY_PARAMETER_GET_AUDIO_CODEC_INFO = 2004;					// get only
	public static final int KEY_PARAMETER_DO_CHANGE_AUDIO_TRACK  = 2005; 				// set only

	// Screen Mode Change
	public static final int KEY_PARAMETER_CHANGE_SCREENMODE                 = 4000;
	public static final int KEY_PARAMETER_CHANGE_SCREENMODE_CROP_LEFT       = 4001;
	public static final int KEY_PARAMETER_CHANGE_SCREENMODE_CROP_TOP        = 4002;
	public static final int KEY_PARAMETER_CHANGE_SCREENMODE_CROP_RIGHT      = 4003;
	public static final int KEY_PARAMETER_CHANGE_SCREENMODE_CROP_BOTTOM     = 4004;

	// For Media Info
	public static final int	KEY_PARAMETER_GET_AUDIO_CODECTYPE  = 8000;
	public static final int	KEY_PARAMETER_GET_VIDEO_CODECTYPE  = 8001;
	public static final int	KEY_PARAMETER_GET_VIDEO_FRAMERATE  = 8002;
	public static final int	KEY_PARAMETER_GET_AUDIO_SAMPLERATE = 8003;
	public static final int	KEY_PARAMETER_GET_BITRATE          = 8004;
}
