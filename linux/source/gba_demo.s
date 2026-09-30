
 mov r10, #0x04000000
 mov r0, #0x0400
 orr r0, r0, #3
 strh r0, [r10]
 mov r8, #0x02000000
 mov r7, #0
 str r7, [r8]
 ldr r0, =0x08001000
 mov r1, #0x06000000
 ldr r2, =38400
 copy:
 ldrh r3, [r0], #2
 strh r3, [r1], #2
 subs r2, r2, #1
 bne copy
 mov r0, #0x80
 strh r0, [r10, #0x84]
 ldr r0, =0x1177
 strh r0, [r10, #0x80]
 mov r0, #2
 strh r0, [r10, #0x82]
 wait_active:
 ldrh r0, [r10, #6]
 cmp r0, #160
 bhs wait_active
 wait_blank:
 ldrh r0, [r10, #6]
 cmp r0, #160
 blo wait_blank
 ldr r1, =0x04000130
 ldrh r2, [r1]
 mvn r2, r2
 ldr r0, =0x3FF
 and r2, r2, r0
 str r2, [r8]
 add r7, r7, #1
 str r7, [r8, #4]
 ldr r1, =0x0E000000
 ldrb r3, [r1]
 tst r2, #1
 beq no_save
 ldr r4, [r8, #8]
 tst r4, #1
 bne no_save
 add r3, r3, #1
 strb r3, [r1]
 no_save:
 str r2, [r8, #8]
 str r3, [r8, #12]
 tst r2, #1
 beq quiet
 ldr r0, =0xF080
 strh r0, [r10, #0x62]
 ldr r0, =0x8700
 strh r0, [r10, #0x64]
 b sound_done
 quiet:
 mov r0, #0
 strh r0, [r10, #0x62]
 sound_done:
 ldr r1, =0x060106D0
 ldr r0, =0x7FE0
 cmp r2, #0
 ldrne r0, =0x03FF
 mov r4, #6
 barrow:
 mov r5, #160
 barpixel:
 strh r0, [r1], #2
 subs r5, r5, #1
 bne barpixel
 add r1, r1, #160
 subs r4, r4, #1
 bne barrow
 b wait_active
 