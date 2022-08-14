# Define TCC support audio codec
#	 note : AC3 decoder and DDP decoder can not be applied simultaneously. 
#	 So both 'tcc.audio.support.ac3' and 'tcc.audio.support.ddp' should not be active at the same time.
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.audio.support.ac3 = 0 \
	tcc.audio.support.ddp = 0 \
	tcc.audio.support.dts = 0 \
	tcc.audio.support.wma = 0 \
	tcc.audio.support.dra = 0 \
	tcc.audio.support.ra = 0 \
	tcc.audio.support.aac = 1 \
	tcc.audio.support.bsac = 0 \
	tcc.audio.support.mp3 = 1 \
	tcc.audio.support.mp2 = 1 \
	tcc.audio.support.mp1 = 1 \
	tcc.audio.support.vorbis = 1 \
	tcc.audio.support.flac = 1 \
	tcc.audio.support.ape = 1 \
	tcc.audio.support.wav = 1 \
	tcc.audio.support.amr = 1 \
	tcc.audio.support.amrwbp = 1 \
	tcc.audio.support.evrc = 0 \
	tcc.audio.support.qcelp = 0
	
PRODUCT_PACKAGES += \
	libTCC.aacdec \
	libTCC.latmdmx \
	libTCC.ac3dec \
	libTCC.apedec \
	libTCC.dtsdec \
	libTCC.flacdec \
	libTCC.mp2dec \
	libTCC.mp3dec \
	libTCC.mp3enc \
	libTCC.pcmdec \
	libTCC.vorbisdec \
	libTCC.wmadec

