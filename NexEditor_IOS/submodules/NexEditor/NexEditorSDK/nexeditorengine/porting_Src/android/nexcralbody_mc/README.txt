Some android makefile options you may need to add when compiling the shared library:

LOCAL_SHARED_LIBRARIES += \
	libandroid \
	liblog \
	libGLESv2 \
	libEGL \
	libdl

LOCAL_STATIC_LIBRARIES += \
	libstlport_static