section .text
extern c_irq_handler

%macro generate_isr_stub 2
isr_stub_%+%1:
    ; save context
%if %2 = 0
    push 0          ; dummy push to be stack aligned with error code
%endif
    push rdi
    push rsi
    
    mov rdi, %1
%if %2 = 1
    mov  rsi, [rsp + 8 * 2]   ; error code
%else
    mov  rsi, 0   
%endif
    jmp isr_common
%endmacro

isr_common:
    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    call c_irq_handler

    ; restore context
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdx
    pop rcx
    pop rbx

    ; add rsp, 8          ; restore alignment
    pop rax
    pop rsi
    pop rdi
    add rsp, 8          ; pop error code 
    iretq


generate_isr_stub 0,  0
generate_isr_stub 1,  0
generate_isr_stub 2,  0
generate_isr_stub 3,  0
generate_isr_stub 4,  0
generate_isr_stub 5,  0
generate_isr_stub 6,  0
generate_isr_stub 7,  0
generate_isr_stub 8,  1 
generate_isr_stub 9,  0
generate_isr_stub 10, 0
generate_isr_stub 11, 0
generate_isr_stub 12, 0
generate_isr_stub 13, 1
generate_isr_stub 14, 1
generate_isr_stub 15, 0
generate_isr_stub 16, 0
generate_isr_stub 17, 0
generate_isr_stub 18, 0
generate_isr_stub 19, 0
generate_isr_stub 20, 0
generate_isr_stub 21, 0
generate_isr_stub 22, 0
generate_isr_stub 23, 0
generate_isr_stub 24, 0
generate_isr_stub 25, 0
generate_isr_stub 26, 0
generate_isr_stub 27, 0
generate_isr_stub 28, 0
generate_isr_stub 29, 0
generate_isr_stub 30, 0
generate_isr_stub 31, 0

%assign i 32
%rep 224
    generate_isr_stub i, 0
%assign i i+1
%endrep

global isr_stub_table
isr_stub_table:
%assign i 0 
%rep 256
    dq isr_stub_%+i
%assign i i+1 
%endrep
