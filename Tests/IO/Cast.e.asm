.intel_syntax noprefix
.file 1 "Tests/IO/Cast.e"
.file 2 "../../IO/cstd.e"
.file 3 "sys.e"
.file 4 "win32.asm.obj"
.file 5 "asm.h"
.file 6 "win32.asm"
.section .text
Code_Start:
.global main
.global Start_Test
Base_START:
Base:
.cfi_startproc 
.cfi_def_cfa_offset 16
.loc 1 3 1
mov dword ptr [rcx + 0 ], 0
.loc 1 4 11
mov rax, rcx
.loc 1 3 1
ret 
ret 
Base_END:


.cfi_endproc 
Mid_START:
Mid:
.cfi_startproc 
.cfi_def_cfa_offset 16
.loc 1 7 1
mov rbx, rcx
push rbx
mov rcx, rbx
call Base
mov dword ptr [rbx + 0 ], 1
.loc 1 8 7
mov ecx, 1074261268
mov dword ptr [rbx + 4 ], ecx
mov rax, rbx
.loc 1 7 1
pop rbx
ret 
pop rbx
ret 
Mid_END:


.cfi_endproc 
test_all_format_casts_START:
test_all_format_casts:
.cfi_startproc 
.cfi_def_cfa_offset 16
.loc 1 17 1
mov ecx, 1
.loc 1 18 8
cvtsi2ss xmm0, ecx
movss xmm0, xmm0
.loc 1 20 10
cvtsi2sd xmm1, ecx
.loc 1 18 2
movsd xmm1, xmm1
.loc 1 21 11
cvttss2si r8d, xmm0
.loc 1 20 2
mov ecx, r8d
.loc 1 23 4
cvttsd2si r8d, xmm1
.loc 1 21 2
mov ecx, r8d
.loc 1 24 4
cvtsd2ss xmm1, xmm1
movss xmm0, xmm1
cvtss2sd xmm0, xmm0
movsd xmm1, xmm0
ret 
.loc 1 17 1
test_all_format_casts_END:


.cfi_endproc 
Start_Test_START:
Start_Test:
.cfi_startproc 
.cfi_def_cfa_offset 16
.loc 1 30 1
sub rsp, 8
call test_all_format_casts
.loc 1 31 2
lea rcx, qword ptr [rsp ]
mov rcx, rcx
.loc 1 32 8
call Mid
movss xmm0, dword ptr [rsp + 4 ]
.loc 1 9 2
cvttss2si ecx, xmm0
mov eax, ecx
.loc 1 33 2
add rsp, 8
ret 
add rsp, 8
ret 
.loc 1 30 1
Start_Test_END:


.cfi_endproc 
main_START:
main:
.cfi_startproc 
.cfi_def_cfa_offset 16
.loc 1 36 1
mov eax, 1
.loc 1 38 2
ret 
ret 
.loc 1 36 1
main_END:


.cfi_endproc 
Code_End:
.section .debug_abbrev
debug_abbrev:
.byte 1
.byte 17
.byte 1
.byte 37
.byte 14
.byte 19
.byte 5
.byte 3
.byte 14
.byte 16
.byte 23
.byte 27
.byte 14
.byte 17
.byte 1
.byte 85
.byte 23
.byte 0
.byte 0
.byte 2
.byte 36
.byte 0
.byte 3
.byte 8
.byte 62
.byte 11
.byte 11
.byte 11
.byte 58
.byte 11
.byte 59
.byte 11
.byte 0
.byte 0
.byte 3
.byte 52
.byte 0
.byte 56
.byte 5
.byte 3
.byte 8
.byte 58
.byte 11
.byte 59
.byte 11
.byte 73
.byte 19
.byte 0
.byte 0
.byte 4
.byte 46
.byte 1
.byte 17
.byte 1
.byte 18
.byte 6
.byte 64
.byte 24
.byte 3
.byte 8
.byte 58
.byte 11
.byte 59
.byte 11
.byte 0
.byte 0
.byte 5
.byte 5
.byte 0
.byte 2
.byte 24
.byte 3
.byte 8
.byte 58
.byte 11
.byte 59
.byte 11
.byte 73
.byte 19
.byte 0
.byte 0
.byte 6
.byte 2
.byte 1
.byte 54
.byte 11
.byte 3
.byte 8
.byte 11
.byte 11
.byte 58
.byte 11
.byte 59
.byte 11
.byte 0
.byte 0
.byte 7
.byte 2
.byte 1
.byte 54
.byte 11
.byte 3
.byte 8
.byte 11
.byte 11
.byte 58
.byte 11
.byte 59
.byte 11
.byte 73
.byte 19
.byte 0
.byte 0
.byte 8
.byte 52
.byte 0
.byte 2
.byte 24
.byte 3
.byte 8
.byte 58
.byte 11
.byte 59
.byte 11
.byte 73
.byte 19
.byte 0
.byte 0
.byte 9
.byte 46
.byte 1
.byte 17
.byte 1
.byte 18
.byte 6
.byte 64
.byte 24
.byte 110
.byte 8
.byte 3
.byte 8
.byte 58
.byte 11
.byte 59
.byte 11
.byte 63
.byte 25
.byte 0
.byte 0
.byte 10
.byte 46
.byte 0
.byte 17
.byte 1
.byte 18
.byte 6
.byte 64
.byte 24
.byte 110
.byte 8
.byte 3
.byte 8
.byte 58
.byte 11
.byte 59
.byte 11
.byte 63
.byte 25
.byte 0
.byte 0
.byte 11
.byte 46
.byte 1
.byte 17
.byte 1
.byte 18
.byte 6
.byte 64
.byte 24
.byte 3
.byte 8
.byte 58
.byte 11
.byte 59
.byte 11
.byte 73
.byte 19
.byte 0
.byte 0
.byte 0
.section .debug_info
Debug_Info_Start:
.long Debug_Info_End-Debug_Info
Debug_Info:
.word 4
.secrel32 debug_abbrev
.byte 8
.byte 1
.secrel32 .COMPILER_NAME
.word 0x29A
.secrel32 .FILE_NAME
.secrel32 .LINE_TABLE
.secrel32 .DIRECTORY
.quad Code_Start
.long Code_End-Code_Start
_int_START:
.byte 2
.asciz "int"
.byte 5
.byte 4
.byte 2
.byte 3
_short_START:
.byte 2
.asciz "short"
.byte 5
.byte 2
.byte 2
.byte 7
_char_START:
.byte 2
.asciz "char"
.byte 6
.byte 1
.byte 2
.byte 11
_float_START:
.byte 2
.asciz "float"
.byte 4
.byte 4
.byte 2
.byte 15
_double_START:
.byte 2
.asciz "double"
.byte 4
.byte 8
.byte 2
.byte 20
_long_START:
.byte 2
.asciz "long"
.byte 5
.byte 8
.byte 2
.byte 25
_string_START:
.byte 2
.asciz "string"
.byte 6
.byte 1
.byte 2
.byte 29
_Base_START:
.byte 6
.byte 1
.asciz "Base"
.byte 4
.byte 1
.byte 3
.byte 3
.byte 2
.byte 145
.byte 0
.asciz "Type"
.byte 1
.byte 4
.long _int_START-Debug_Info_Start
.byte 0
_Mid_START:
.byte 7
.byte 1
.asciz "Mid"
.byte 8
.byte 1
.byte 7
.long _Base_START-Debug_Info_Start
.byte 3
.byte 2
.byte 145
.byte 4
.asciz "feature"
.byte 1
.byte 9
.long _float_START-Debug_Info_Start
.byte 0
_Top_START:
.byte 7
.byte 1
.asciz "Top"
.byte 12
.byte 1
.byte 12
.long _Mid_START-Debug_Info_Start
.byte 0
.byte 4
.quad test_all_format_casts_START
.long test_all_format_casts_END-test_all_format_casts_START
.byte 1
.byte 87
.asciz "test_all_format_casts"
.byte 1
.byte 17
.byte 8
.byte 0
.asciz "a"
.byte 1
.byte 20
.long _float_START-Debug_Info_Start
.byte 8
.byte 0
.asciz "b"
.byte 1
.byte 21
.long _double_START-Debug_Info_Start
.byte 0
.byte 9
.quad Start_Test_START
.long Start_Test_END-Start_Test_START
.byte 1
.byte 87
.asciz "Start_Test"
.asciz "Start_Test"
.byte 1
.byte 30
.byte 8
.byte 0
.asciz "m"
.byte 1
.byte 32
.long _Mid_START-Debug_Info_Start
.byte 0
.byte 10
.quad main_START
.long main_END-main_START
.byte 1
.byte 87
.asciz "main"
.asciz "main"
.byte 1
.byte 36
.byte 5
.byte 0
.asciz "this"
.byte 1
.byte 3
.long _Base_START-Debug_Info_Start
Debug_Info_End:
.section .debug_str
.COMPILER_NAME:
.asciz "Evie engine 3.0.0 https://github.com/Gabidal/Evie"
.FILE_NAME:
.asciz "Tests/IO/Cast.e"
.DIRECTORY:
.asciz "Tests/IO/"
.section .LINE_TABLE
.LINE_TABLE:
