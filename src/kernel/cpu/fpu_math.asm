[global fsinus]
; Return sinus value of esp + 4 degrees
fsinus:
    fclex                       ;clear all previous exceptions
    mov eax, dword [esp + 4]    ;take the degree to take sinus of
    push eax 
    fidiv dword [esp]           ;divide the angle by 180 degrees
    fldpi                       ;load the hard-coded value of PI
    fmul                        ;ST(0) = angle in degrees * PI / 180 -> angle in radians
    fsin                        ;compute the sinus of the angle

    ;mov eax, [esp + 8]
    fstsw [esp]           ;store the value of sin(esp + 4) at [esp]
    fwait                       ;to insure that the operation was complete
    
    pop eax
    jmp .done
    shr al, 1                   ;check if the Invalid OP Flag set
    jnc .done                   ;Nope, we're done

    ;angle has been trashed ..
    mov eax, -1
    ret
    
    .done:
        mov [esp + 8], eax
        ret

[global fpu_init]
fpu_init:
    finit

    ret


