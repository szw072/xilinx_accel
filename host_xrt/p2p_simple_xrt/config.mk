ifeq ($(HOST_ARCH), x86)
CXXFLAGS += -D_GLIBCXX_USE_CXX11_ABI=0
endif
DRIVE_PATH:=
ifneq ($(DRIVE_PATH), )
	CMD_ARGS += -p $(DRIVE_PATH)
else ifeq ($(findstring _u2_, $(DEVICE)), _u2_)
ifeq ($(TARGET),$(filter $(TARGET),hw))	
	CMD_ARGS += -p /dev/nvme0n1
endif
endif
