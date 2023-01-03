@micropython.asm_thumb
def gpio_output(r0):
    mov(r1, r0)
    asr(r1, 4)