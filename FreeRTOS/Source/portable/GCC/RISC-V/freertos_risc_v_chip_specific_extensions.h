#ifndef FREERTOS_RISC_V_CHIP_SPECIFIC_EXTENSIONS_H
#define FREERTOS_RISC_V_CHIP_SPECIFIC_EXTENSIONS_H

/* Minimal configuration for QEMU virt machine (standard RISC-V) */
#define portasmHAS_SIFIVE_CLINT 1
#define portasmHAS_MTIME        1

/* No additional registers to save/restore (basic rv64) */
#define portasmADDITIONAL_CONTEXT_SIZE 0

#define portasmSAVE_ADDITIONAL_REGISTERS
#define portasmRESTORE_ADDITIONAL_REGISTERS

#endif /* FREERTOS_RISC_V_CHIP_SPECIFIC_EXTENSIONS_H */
