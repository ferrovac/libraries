/*
struct ContextData {
        // General-purpose registers
        uint32_t r0;
        uint32_t r1;
        uint32_t r2;
        uint32_t r3;
        uint32_t r4;
        uint32_t r5;
        uint32_t r6;
        uint32_t r7;
        uint32_t r8;
        uint32_t r9;
        uint32_t r10;
        uint32_t r11;
        uint32_t r12;
        
        // Stack pointers
        uint32_t msp;  // Main Stack Pointer
        uint32_t psp;  // Process Stack Pointer

        // Link Register and Program Counter
        uint32_t lr;   // Link Register
        uint32_t pc;   // Program Counter
        
    };
    
    
    ContextData savedContext;
    //This saves the context on the PSP, the Cortex-M3 pushes the other registers using hardware
    void save_context(){
      asm volatile (
            // Save general-purpose registers
            "str r0, [%0, #0]\n"
            "str r1, [%0, #4]\n"
            "str r2, [%0, #8]\n"
            "str r3, [%0, #12]\n"
            "str r4, [%0, #16]\n"
            "str r5, [%0, #20]\n"
            "str r6, [%0, #24]\n"
            "str r7, [%0, #28]\n"
            "str r8, [%0, #32]\n"
            "str r9, [%0, #36]\n"
            "str r10, [%0, #40]\n"
            "str r11, [%0, #44]\n"
            "str r12, [%0, #48]\n"

            // Save the Main Stack Pointer (MSP) and Process Stack Pointer (PSP)
            "mrs r0, MSP\n"
            "str r0, [%0, #52]\n"  // Offset 52 for MSP
            "mrs r0, PSP\n"
            "str r0, [%0, #56]\n"  // Offset 56 for PSP

            // Save LR and PC
            "mov r0, lr\n"
            "str r0, [%0, #60]\n"  // Offset 60 for LR
            "mov r0, pc\n"
            "str r0, [%0, #64]\n"  // Offset 64 for PC
            :
            : "r" (&savedContext)
            : "memory", "r0"
        );

        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
    }

    //This loads the context from the PSP, the Cortex-M3 loads the other registers using hardware
    void load_context(){
              asm volatile (
            // Load general-purpose registers
            "ldr r0, [%0, #0]\n"
            "ldr r1, [%0, #4]\n"
            "ldr r2, [%0, #8]\n"
            "ldr r3, [%0, #12]\n"
            "ldr r4, [%0, #16]\n"
            "ldr r5, [%0, #20]\n"
            "ldr r6, [%0, #24]\n"
            "ldr r7, [%0, #28]\n"
            "ldr r8, [%0, #32]\n"
            "ldr r9, [%0, #36]\n"
            "ldr r10, [%0, #40]\n"
            "ldr r11, [%0, #44]\n"
            "ldr r12, [%0, #48]\n"

            // Load the Main Stack Pointer (MSP) and Process Stack Pointer (PSP)
            "ldr r0, [%0, #52]\n"  // Offset 52 for MSP
            "msr MSP, r0\n"
            "ldr r0, [%0, #56]\n"  // Offset 56 for PSP
            "msr PSP, r0\n"

            // Load LR and PC
            "ldr r0, [%0, #60]\n"  // Offset 60 for LR
            "mov lr, r0\n"
            "ldr r0, [%0, #64]\n"  // Offset 64 for PC
            "mov pc, r0\n"
            :
            : "r" (&savedContext)
            : "memory", "r0"
        );     
}

*/