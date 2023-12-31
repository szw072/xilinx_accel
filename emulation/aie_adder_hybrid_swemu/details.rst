AIE Adder Hybrid SW-Emulation using x86 PS 
==========================================

This example demonstrates how a user can run SW Emulation with cycle accurate AIE intended to provide cycle accuracy for AIE with fast, functional PL and PS running in SW emulation.  

The entire system design having AIE interfaced with PL kernels needs to be controlled through software using XRT APIs and such designs can be verified only through SW Emulation or HW Emulation flow.The cycle accuracy is very important aspect in AIE kernel development. SW_EMU does not provide cycle accuracy while HW_EMU does provides cycle accuracy but it is too slow for the users focused only on AIE. The hybrid flow intends to run SW Emulation with cycle accurate AIE keeping the PL as functional.

On the top, the PS on x86 flow in hybrid software emulation helps to run SW_EMU faster unlike the QEMU flow that requires overhead of creating sd_card.img, booting petalinux on full QEMU machine thereby taking additional time.  
 
Input from the User
-------------------

1. Compile the ADF graph for hw target 

        `aiecompiler --target=hw -Xpreproc=-D_HYBRID_EMU_ -include="$(XILINX_VITIS)/aietools/include" -include="./src/aie" -include="./data" -include="./" --pl-freq=100 -workdir=./_aie`

2. The vitis compile and link flow (v++ -c and v++ -l) must use the sw_emu target.  

3. Set the following environment variables on the terminal:
 
         `setenv XILINX_VITIS_EMU_HYBRID 1`: sets the Hybrid flow (required for link and runtime) 

4. Host compilation must be similar to ps_on_x86. 
 
          *Note:* This example Makefile already supports ps on x86 host compilation flow for hybrid software emulation. 

5. Use ``make run TARGET=sw_emu`` to run for the software emulation target. 
