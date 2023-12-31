#
# Copyright 2022 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# makefile-generator v1.0.3
#

############################## Help Section ##############################
.PHONY: help

help::
	$(ECHO) "Makefile Usage:"
	$(ECHO) "  make all TARGET=<sw_emu/hw_emu/hw> PLATFORM=<FPGA platform> EDGE_COMMON_SW=<rootfs and kernel image path>."
	$(ECHO) "      Command to generate the design for specified Target and Shell."
	$(ECHO) ""
	$(ECHO) "  make clean "
	$(ECHO) "      Command to remove the generated non-hardware files."
	$(ECHO) ""
	$(ECHO) "  make cleanall"
	$(ECHO) "      Command to remove all the generated files."
	$(ECHO) ""
	$(ECHO) "  make test PLATFORM=<FPGA platform>"
	$(ECHO) "     Command to run the application. This is same as 'run' target but does not have any makefile dependency."
	$(ECHO) ""
	$(ECHO) "  make run TARGET=<sw_emu/hw_emu/hw> PLATFORM=<FPGA platform> EDGE_COMMON_SW=<rootfs and kernel image path>"
	$(ECHO) "      Command to run application in emulation."
	$(ECHO) ""
	$(ECHO) "  make build TARGET=<sw_emu/hw_emu/hw> PLATFORM=<FPGA platform> EDGE_COMMON_SW=<rootfs and kernel image path>"
	$(ECHO) "      Command to build xclbin application."
	$(ECHO) ""
	$(ECHO) "  make host"
	$(ECHO) "      Command to build host application."
	$(ECHO) "      EDGE_COMMON_SW is required for SoC shells. Please download and use the pre-built image from - "
	$(ECHO) "      https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/embedded-platforms.html"
	$(ECHO) ""

############################## Setting up Project Variables ##############################
# Points to top directory of Git repository
MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR := $(patsubst %/,%,$(dir $(MK_PATH)))
COMMON_REPO ?= $(shell bash -c 'export MK_PATH=$(MK_PATH); echo $${MK_PATH%validate/ps_aie_gmio_test/*}')
PWD = $(shell readlink -f .)
XF_PROJ_ROOT = $(shell readlink -f $(COMMON_REPO))

TARGET := hw
HOST_ARCH := x86
SYSROOT := $(EDGE_COMMON_SW)/sysroots/cortexa72-cortexa53-xilinx-linux

include ./utils.mk

XSA := 
ifneq ($(PLATFORM), )
XSA := $(call device2xsa, $(PLATFORM))
endif
TEMP_DIR := ./_x.$(TARGET).$(XSA)
BUILD_DIR := ./build_dir.$(TARGET).$(XSA)

LINK_OUTPUT := $(BUILD_DIR)/ps_aie.link.xclbin
ifeq ($(DEV_ARCH), versal)
LINK_OUTPUT := $(BUILD_DIR)/ps_aie.xsa
endif

# SoC variables
RUN_APP_SCRIPT = ./run_app.sh
PACKAGE_OUT = ./package.$(TARGET)

LAUNCH_EMULATOR = $(PACKAGE_OUT)/launch_$(TARGET).sh
RESULT_STRING = TEST PASSED

VPP := v++
VPP_PFLAGS := 
CMD_ARGS = -p $(GEN_DIR) 

include config.mk

CXXFLAGS += $(opencl_CXXFLAGS) -Wall -O0 -g -std=c++1y
LDFLAGS += $(opencl_LDFLAGS)

########################## Checking if PLATFORM in allowlist #######################
PLATFORM_ALLOWLIST +=  vck5000
PLATFORM_BLOCKLIST +=  u25_ u30 u200 zc vck190 u250 aws-vu9p-f1 samsung u2_ x3522pv nodma

############################## Setting up Host Variables ##############################
#Include Required Host Source Files
HOST_SRCS += ./src/host.cpp 
# Host compiler global settings
CXXFLAGS += -fmessage-length=0 -I$(XILINX_XRT)/include
LDFLAGS += -lrt -lstdc++ -lxrt_core -lxrt_coreutil -lpthread -luuid -L$(XILINX_XRT)/lib
LDFLAGS += -lboost_filesystem -lboost_system

# aie template
XILINX_VITIS_AIETOOLS ?=
XILINX_VITIS_AIELIBS ?=
ifneq ($(wildcard $(XILINX_VITIS)/aietools/.*),)
XILINX_VITIS_AIETOOLS := $(XILINX_VITIS)/aietools
else
XILINX_VITIS_AIETOOLS := $(XILINX_VITIS)/cardano
endif
CXXFLAGS += -I$(XILINX_VITIS_AIETOOLS)/include
# Setting customized_params in aiecompiler

############################ setting AIE Compiler ###########################
ifneq ($(filter aiesim hw_emu, $(TARGET)),)
AIETARGET := hw
else
AIETARGET := $(TARGET)
endif
AIE_CXXFLAGS += --target=$(AIETARGET) --platform=$(PLATFORM) -v
AIE_CXXFLAGS += -I $(XILINX_VITIS_AIETOOLS)/include
AIE_CXXFLAGS += -I $(CUR_DIR)/src
AIE_CONTAINER = $(TEMP_DIR)/libadf.a

EXE_FILE_DEPS += $(AIE_CONTAINER)
BINARY_CONTAINERS_DEPS += $(AIE_CONTAINER)

DMA_LOCK_JSON += $(CUR_DIR)/Work/reports/dma_lock_report.json
AIE_CONFIG_JSON += $(CUR_DIR)/Work/ps/c_rts/aie_control_config.json

############################## Setting up Kernel Variables ##############################
# Kernel compiler global settings
VPP_FLAGS += -t $(TARGET) --platform $(PLATFORM) --save-temps 

EXECUTABLE = ./ps_aie_gmio_test.exe
EMCONFIG_DIR = $(TEMP_DIR)

############################## Setting up PS Kernel Variables ##############################
CROSS_COMPILE := $(XILINX_VITIS)/gnu/aarch64/lin/aarch64-linux/bin/aarch64-linux-gnu-
CROSSCXX := $(CROSS_COMPILE)g++
ps_target = aie_kernel.so
ps_OBJS = aie_kernel.o
CROSSCFLAGS = -I$(XILINX_XRT)/include -I$(SYSROOT)/usr/include/xrt -I$(SYSROOT)/ -fPIC -fvisibility=hidden -lrt --sysroot=$(SYSROOT) -Ilibs -I$(XILINX_VITIS_AIETOOLS)/include
CROSSCFLAGS += -Wall -O2 -g -D__PS_ENABLE_AIE__

CROSSLDFLAGS = -shared -lxrt_core -L$(SYSROOT)/usr/lib -lxilinxopencl -lpthread -lrt -ldl -lcrypt -lstdc++ --sysroot=$(SYSROOT)

############################## Declaring Binary Containers ##############################
BINARY_CONTAINERS += $(BUILD_DIR)/ps_aie.xsa
BINARY_CONTAINERS_TEMP += $(BUILD_DIR)/ps_aie_temp.xclbin
BINARY_CONTAINERS_PKG += $(BUILD_DIR)/ps_aie.xclbin

############################## Setting Targets ##############################
CP = cp -rf

.PHONY: all clean cleanall docs emconfig
all: check-platform check-device check-vitis check_edge_sw $(EXECUTABLE) $(ps_target) $(BINARY_CONTAINERS) $(BINARY_CONTAINERS_TEMP) $(BINARY_CONTAINERS_PKG) emconfig

.PHONY: host
host: $(EXECUTABLE)

.PHONY: build
build: check-vitis check-device check_edge_sw $(ps_target) $(BINARY_CONTAINERS) $(BINARY_CONTAINERS_TEMP) $(BINARY_CONTAINERS_PKG)

.PHONY: xclbin
xclbin: build

.PHONY: ps_kernel
ps_kernel: check_edge_sw $(ps_target)

############################## Setting Rules for PS Kernel objects (Building PS Kernels) ##############################
$(ps_OBJS) : ./src/aie_kernel.cpp
	$(CROSSCXX) -c $(CROSSCFLAGS) -o $@ $<

$(ps_target): $(ps_OBJS)
	$(CROSSCXX) $< $(CROSSLDFLAGS) -o $@

############################## Setting Rules for AIE (Building Kernels) ##################
$(AIE_CONTAINER): $(CUR_DIR)/src/graph.cpp $(CUR_DIR)/src/kernels/mm.cc
	$(ECHO) "Compiling: libadf.a"
	mkdir -p $(dir $@)
	$(AIECXX) $(AIE_CXXFLAGS) $^
	cp ./libadf.a $(AIE_CONTAINER)

############################## Setting Rules for Binary Containers (Building Kernels) ##############################
$(BINARY_CONTAINERS): $(BINARY_CONTAINERS_DEPS)
	mkdir -p $(BUILD_DIR)
	$(VPP) -l $(VPP_FLAGS) $(VPP_LDFLAGS) -o $@ $^

$(BINARY_CONTAINERS_TEMP): $(AIE_CONTAINER) $(BINARY_CONTAINERS)
	${VPP} -p -t ${TARGET} -f ${PLATFORM} $(AIE_CONTAINER) ${BINARY_CONTAINERS} -o ./${BINARY_CONTAINERS_TEMP}
$(BINARY_CONTAINERS_PKG): $(AIE_CONTAINER) $(BINARY_CONTAINERS) $(ps_target)
	xclbinutil --add-pskernel $(ps_target) --input ./${BINARY_CONTAINERS_TEMP} --output ./${BINARY_CONTAINERS_PKG} --force
	mkdir -p $(GEN_DIR) && $(CP) $(BINARY_CONTAINERS_PKG) $(GEN_DIR)
	$(CP) $(DMA_LOCK_JSON) $(GEN_DIR)
	$(CP) $(AIE_CONFIG_JSON) $(GEN_DIR)

#$(BINARY_CONTAINERS_PKG): $(AIE_CONTAINER) $(BINARY_CONTAINERS) $(ps_target)
#	${VPP} -p -t ${TARGET} -f ${PLATFORM} ${BINARY_CONTAINERS} --package.ps_kernel $(ps_target) -o ./${BINARY_CONTAINERS_PKG}

############################## Setting Rules for Host (Building Host Executable) ##############################
$(EXECUTABLE): $(HOST_SRCS) | check-xrt
		$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

emconfig:$(EMCONFIG_DIR)/emconfig.json
$(EMCONFIG_DIR)/emconfig.json:
	emconfigutil --platform $(PLATFORM) --od $(EMCONFIG_DIR)

############################## Setting Essential Checks and Running Rules ##############################
run: all
ifeq ($(TARGET),$(filter $(TARGET),sw_emu hw_emu))
	$(CP) $(EMCONFIG_DIR)/emconfig.json .
	XCL_EMULATION_MODE=$(TARGET) $(EXECUTABLE) $(CMD_ARGS)
else
	$(EXECUTABLE) $(CMD_ARGS)
endif


.PHONY: test
test: $(EXECUTABLE)
ifeq ($(TARGET),$(filter $(TARGET),sw_emu hw_emu))
	XCL_EMULATION_MODE=$(TARGET) $(EXECUTABLE) $(CMD_ARGS)
else
	$(EXECUTABLE) $(CMD_ARGS)
endif

check_edge_sw:
ifndef EDGE_COMMON_SW
	$(error EDGE_COMMON_SW variable is not set, please download and use the pre-built image from https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/embedded-platforms.html)
endif

############################## Cleaning Rules ##############################
# Cleaning stuff
clean:
	-$(RMDIR) $(EXECUTABLE) $(XCLBIN)/{*sw_emu*,*hw_emu*} 
	-$(RMDIR) profile_* TempConfig system_estimate.xtxt *.rpt *.csv 
	-$(RMDIR) src/*.ll *v++* .Xil emconfig.json dltmp* xmltmp* *.log *.jou *.wcfg *.wdb

cleanall: clean
	-$(RMDIR) build_dir*
	-$(RMDIR) package.*
	-$(RMDIR) _x* *xclbin.run_summary qemu-memory-_* emulation _vimage pl* start_simulation.sh *.xclbin

