section .text

; --- Prints a message on the terminal
; %1: String
; %2: Length of the string
%macro print 2
    mov rax, 1
    mov rdi, 1
    mov rsi, %1
    mov rdx, %2
    syscall
%endmacro


; --- Gets user input
; %1 : Reserved memory
; %2 : Length in bytes
%macro getInput 2
    mov rax, 0
    mov rdi, 0
    mov rsi, %1
    mov rdx, %2
    syscall
%endmacro


; --- Stops the program from running
%macro end 0
    mov rax, 60
    mov rdi, 0
    syscall
%endmacro


; --- Clears a chunk of reserved memory
; %1 : Reserved memory
; %2 : Length in bytes
%macro clear 2
    mov rdi, %1
    mov rcx, %2
    xor rax, rax
    rep stosb
%endmacro


; --- Saves the expression to be evaluated
; %1 : String (expression)
; %2 : Length in bytes
%macro getExpression 2
    
    getInput tempExpression, %2

    ; Clears the counter
    xor rcx, rcx ; - String counter
    xor r15, r15 ; - Overall character counter

    %%_loop:
        ; If r15 surpasses %2, an error is shown
        cmp r15, %2
        jg _expressionError

        mov bl, BYTE[tempExpression + rcx]

        ; Finds the end of the expression
        cmp bl, ','
        je %%_stop
        
        ; ---- VALIDATIONS -----
        cmp bl, ' '
        je %%_valid

        cmp bl, '.'
        je _expressionError

        cmp bl, 10
        je %%_enter

        cmp bl, '%'
        je %%_valid

        cmp bl, '*'
        jl _expressionError

        cmp bl, '9'
        jle %%_valid

        cmp bl, 'A'
        jl _expressionError

        cmp bl, 'Z'
        jle %%_valid

        cmp bl, 'a'
        jl _expressionError

        cmp bl, 'z'
        jg _expressionError 

    %%_valid:
        ; Copies information
        mov BYTE[%1 + r15], bl  
    
        inc rcx
        inc r15

    %%_enter:
        ; Checks for enter character (no need to save it)
        cmp BYTE[tempExpression + rcx], 10
        je %%_redo

        jmp %%_loop

    ; Restarts the loop
    %%_redo:
        clear tempExpression, %2
        getInput tempExpression, %2

        ; - Clear the string counter
        xor rcx, rcx
        jmp %%_loop

    %%_stop:

%endmacro


; --- Validates the expression and gets rid of spaces
; %1 : String (expression)
; %2 : String (Empty memory)
%macro verifyExpression 2
    ; Clears counter
    xor rcx, rcx ; Old string counter
    xor r15, r15 ; New string counter
    xor r8, r8
    xor r9, r9 
    xor r10, r10 ; Flag regarding only 0's
    ; First loop to get rid of spaces
    %%_sloop:
        ; Copies the information
        mov al, BYTE[%1 + rcx]
        
        cmp al, 0
        je %%_verify
        
        ; Checks space characters to detect errors
        cmp al, ' '
        je %%_incrementS

        cmp al, 'A'
        jge %%_foundVar

        cmp al, '0'
        je %%_removeLeading

        cmp al, '0'
        jl %%_foundOperator

        mov r10, 1
        mov BYTE[%2 + r15], al
        inc r15

        jmp %%_incrementS

    %%_foundOperator:
        mov r10, 0
        mov BYTE[%2 + r15], al
        inc r15
        jmp %%_incrementS

    %%_foundVar:
        mov r10, 1
        mov BYTE[%2 + r15], al
        inc r15
        jmp %%_incrementS

    %%_removeLeading:

        
        cmp r10, 1
        je %%_gotNonLeading
        jmp %%_gotLeading

        %%_gotNonLeading:

            mov dl, BYTE[%1 + rcx + 1]
            cmp dl, '^'
            je %%_addNumEndLead

            cmp dl, 'A'
            jge %%_addZero

            cmp dl, '0'
            jne %%_addNumEndLead

            cmp dl, '0'
            je %%_addZero



        %%_gotLeading:
            mov dl, BYTE[%1 + rcx + 1]

            cmp dl, 'A'
            jge %%_addZero

            cmp dl, '0'
            je %%_skip

            cmp dl, '^'
            je %%_addNumEndLead

            cmp dl, '0'
            jg %%_skip2

            cmp dl, '0'
            jl %%_addNumEndLead
            

            %%_skip:
                jmp %%_incrementS

            %%_skip2:
                mov r10, 1
                jmp %%_incrementS


        %%_addZero:
            mov BYTE[%2 + r15], al
            inc r15
            jmp %%_incrementS

        %%_addNumEndLead:
            mov r10, 0
            mov BYTE[%2 + r15], al
            inc r15

    %%_incrementS:
        inc rcx
        jmp %%_sloop

    %%_verify:
        xor rcx, rcx ; Character counter
        xor r15, r15 ; Special counter

        ; Checks if the first character is a valid number
        ; or variable
        cmp BYTE[%2 + rcx], '-'
        je %%_continueFirst

        cmp BYTE[%2 + rcx], '0'
        jl _expressionError

        %%_continueFirst:
            inc rcx

    ; Second loop to verify the expression
    ; saved in %2
    %%_vloop:
        mov bl, BYTE[%2 + rcx]
        mov al, BYTE[%2 + rcx + 1]

        ; Reaches the end of the string
        cmp bl, 0
        je %%_stop

        ; Validations
        cmp bl, '+'
        je %%_sumVerify

        cmp bl, '-'
        je %%_sumVerify

        cmp bl, '/'
        je %%_divVerify

        cmp bl, '%'
        je %%_divVerify

        cmp bl, '*'
        je %%_mulVerify

        ; cmp bl, '.'
        ; je %%_dotVerify

        mov r10, 1
        jmp %%_incrementV




        ; Verification of special characters
        ; Addition and subtraction
        %%_sumVerify: 

            cmp al, '-'
            je %%_incrementV

            cmp al, ' '
            je %%_incrementV

            cmp al, '0'
            jl _expressionError

            jmp %%_incrementV
    
        ; Division and modulo
        %%_divVerify: 
            
            cmp al, '-'
            je %%_incrementV

            cmp al, '0'
            je %%_divOnlyZeros

            cmp al, ' '
            je %%_incrementV

            cmp al, '0'
            jl _expressionError


            jmp %%_incrementV

            %%_divOnlyZeros:
                mov r9, rcx
                %%_divloop:
                    cmp BYTE[%2 + r9 + 1], 44
                    je _zeroDivError

                    cmp BYTE[%2 + r9 + 1], 48
                    jne %%_incrementV

                    inc r9
                    jmp %%_divloop


        ; Multiplication and power
        %%_mulVerify:      


            cmp al, '*'
            je %%_checkStars
            
            ; Validations
            cmp al, '-'
            je %%_incrementV

            cmp al, ' '
            je %%_incrementV

            cmp al, '0'
            jl _expressionError

            cmp al, 'z'
            jle %%_incrementV

            ; Check amount of '*' (More than 2 -> Error)
            %%_checkStars:
                mov al, BYTE[%2 + rcx + 2]
                
                ; Goes into the last character
                cmp al, 0
                inc rcx
                je %%_incrementV 

                cmp al, '-'
                je %%_incrementV

                ; Checks if what follows isnt a number or variable
                cmp al, '0'
                jl _expressionError

                cmp al, 'z'
                jl %%_incrementV

        ; ; Decimal
        ; %%_dotVerify:
        ;     cmp BYTE[%2 + rcx - 1], '9'
        ;     jg _fractionError

        ;     cmp al, '0'
        ;     jl _expressionError

        ;     cmp al, '9'
        ;     jg _fractionError
        ;     jle %%_dotDigits

        ;     jmp %%_incrementV

        ;     ; Loop to count amount of digits after decimal
        ;     %%_dotDigits:
        ;         cmp r15, 3
        ;         jge _fractionError

        ;         cmp BYTE[%2 + rcx + r15], 0
        ;         je %%_break

        ;         cmp BYTE[%2 + rcx + r15 + 1], '0'
        ;         jl %%_break

        ;         cmp BYTE[%2 + rcx + r15 + 1], '9'
        ;         jg %%_break

        ;         inc r15
        ;         jmp %%_dotDigits

        ;     %%_break:
        ;     xor r15, r15 ; Clear special counter
        ;     jmp %%_incrementV

        %%_incrementV:
            inc rcx
            jmp %%_vloop

    %%_stop:
        mov BYTE[%2 + rcx], 10
%endmacro


; --- Corrects the expression and puts it in a better format
; --- (Changes '**' to '^' )
; --- (Verifies implicit multiplication)
; --- (Changes signs of operations if several '-' are found)
; %1 : String (expression to correct)
; %2 : String (Empty memory)
%macro correctExpression 2
    xor rcx, rcx ; %1 Counter
    xor r15, r15 ; %2 Counter
    xor r14, r14 ; Special counter

    ; Copies the information and corrects it on the way
    %%_loop:
        mov bl, BYTE[%1 + rcx]
        mov al, BYTE[%1 + rcx + 1]

        ; Checks for '**'
        cmp bl, '*'
        je %%_checkStar

        ; Checks if the next character is '-'
        cmp al, '-'
        je %%_beforeMin

        ; End of string
        cmp bl, 10
        je %%_end

        ; Checks for implicit multiplication
        cmp bl, '9'
        jle %%_checkMul1

        cmp bl, 'A'
        jge %%_checkMul2

        jmp %%_copyBL

    ; Adds a star to turn implicit multiplication to explicit
    ; 2a -> 2*a
    %%_checkMul1:
        cmp bl, '0'
        jl %%_copyBL

        cmp al, 'A'
        jge %%_addStar1

        jmp %%_copyBL

            %%_addStar1:
                mov BYTE[%2 + r15], bl
                inc r15
                mov BYTE[%2 + r15], '*'
                jmp %%_increment

    ; Adds a star to turn implicit multiplication to explicit
    ; a2 -> a*2
    %%_checkMul2:
        cmp al, '9'
        jle %%_addStar2

        jmp %%_copyBL

            %%_addStar2:
                cmp al, '0'
                jl %%_copyBL

                mov BYTE[%2 + r15], bl
                inc r15
                mov BYTE[%2 + r15], '*'
                jmp %%_increment

    ; Corrects amount of minus symbols    
    %%_beforeMin:
        mov BYTE[%2 + r15], bl

        cmp bl, '0'
        jge %%_insertPlus

        cmp bl, '-'
        je %%_countMinus

        jmp %%_minloop

        %%_insertPlus:
            inc r15
            mov BYTE[%2 + r15], '+'
            jmp %%_minloop

        %%_countMinus: ; Sometimes a minus symbol gets lost, this one counts it
            inc r14
            mov BYTE[%2 + r15], 0

    %%_minloop:

        inc r14 ; Found one '-'
        inc rcx ; Moves the counter

        mov al, [%1 + rcx + 1]
        
        cmp al, '-'
        je %%_minloop

        ; Checks if theres an even or odd number of '-'
        or r14, 0xFFFFFFFFFFFFFFFE
        not r14

        cmp r14, 1 ; Result is even
        je %%_increment 

        inc r15
        ; Amount of '-' is odd
        mov BYTE[%2 + r15], '-'
        jmp %%_increment

    ; Checks if theres two '*' to change it to '^'
    %%_checkStar:
        cmp al, '*'
        je %%_insertSymbol 
        jmp %%_copyBL

        %%_insertSymbol:
            inc rcx
            mov BYTE[%2 + r15], '^'

            jmp %%_increment
            
    ; Copies the information in bl to the
    ; string
    %%_copyBL:
        mov BYTE[%2 + r15], bl
    
    ; Increments the counters
    %%_increment:
        inc rcx
        inc r15
        jmp %%_loop

    %%_end:
        mov BYTE[%2 + r15], 10

%endmacro


; --- Gets all the variables in the expression
; %1 : String (expression to correct)
; %2 : String (Empty memory)
%macro getVariables 2
    xor rcx, rcx ; %1 counter
    xor r15, r15 ; %2 counter

    xor r14, r14 ; Overall variable counter
    xor r12, r12 ; Auxiliary counter
    xor r8, r8   ; Repetition counter
    xor r10, r10   ; Auxiliary counter
    xor r13, r13 ; Counter to check if variable exists
    xor r11, r11 ; Variable character counter

    %%_loop:
        mov bl, BYTE[%1 + rcx]
        
        ; Finishes reading the string
        cmp bl, 10
        je %%_end

        cmp bl, 'A'
        jge %%_checkVars        

        %%_incLoop:
            inc rcx
            jmp %%_loop

        ; Checks if any variable already saved matches
        ; the one being read
        %%_checkVars:
            cmp bl, '^'
            je %%_incLoop

            mov al, BYTE[%2 + r11]
            
            ; Checks if the variable needs to be added
            cmp al, 0
            je %%_addVar

            ; Checks if the variables match
            cmp al, bl
            je %%_parityCheck
            jne %%_nextVar

        ; Checks if its the same variable
        %%_parityCheck:
            xor r12, r12
            xor r10, r10
            xor r13, r13
            ; Gets length of variable saved
            %%_savedLen:
                mov al, BYTE[%2 + r11 + r12]

                cmp al, 0
                je %%_0saved

                cmp al, ','
                je %%_continueReading

                %%_incSaved:
                    inc r12
                    jmp %%_savedLen

                ; Finds any accidental 0's
                %%_0saved:
                    inc r13
                    jmp %%_incSaved

            ; Gets length of variable being read
            %%_continueReading:
                sub r12, r13
                xor r13, r13

            %%_readingLen:
                mov bl, BYTE[%1 + rcx + r10]

                cmp bl, 'A'
                jl %%_comparison

                %%_incReading:
                    inc r10
                    jmp %%_readingLen

                ; Finds any accidental 0's
                %%_0read:
                    inc r13
                    jmp %%_incReading

            %%_comparison:
                sub r10, r13
                
                ; Variables dont coincide
                cmp r12, r10
                jne %%_nextVar
                
                xor r8, r8
                xor r10, r10
            ; Checks each one of the characters
            %%_parityChar:
                mov al, BYTE[%2 + r11]
                mov bl, BYTE[%1 + rcx + r10]

                ; All characters match
                cmp al, ','
                je %%_newLoop

                ; Found accidental 0
                cmp al, 0
                je %%_incAL

                cmp bl, 0
                je %%_incBL

                ; Different character found
                cmp al, bl
                jne %%_nextVar

                ; Reloops
                inc r11
                inc r10
                jmp %%_parityChar

                %%_incAL:
                    inc r11
                    jmp %%_parityChar

                %%_incBL:
                    inc r10
                    jmp %%_parityChar

        ; Reloops into the next character
        %%_newLoop:
            inc rcx
            xor r11, r11

            jmp %%_loop

        ; Goes to the next variable in the list
        %%_nextVar:
            mov al, BYTE[%2 + r11]

            ; Reaches delimiter
            cmp al, ','
            je %%_newVar

            ; Loops again
            inc r11
            jmp %%_nextVar

            ; Puts counter on the first character of the next
            ; variable
            %%_newVar:
                inc r11
                jmp %%_checkVars    

        ; Adds the variable to %2
        %%_addVar:

            mov bl, BYTE[%1 + rcx]
            mov BYTE[%2 + r15], bl ; Saves the first character

            inc rcx
            inc r15

            mov bl, BYTE[%1 + rcx] ; Loads the next character

            ; Checks if the new byte isn't a
            ; letter
            cmp bl, 'A'
            jl %%_endVar

            cmp bl, '^'
            je %%_endVar

            jmp %%_addVar

        %%_endVar:
            inc r14 ; Increments the variable counter

            cmp r14, 10 ; More than 10 variables detected
            jg _variableError

            inc r15
            mov BYTE[%2 + r15], ','
            inc r15    

            xor r11, r11 ; Clears variable pointer
            xor r10, r10
            jmp %%_loop
    %%_end:

%endmacro


; --- Gets the value of a certain variable
; --- %1 : Pivot memory address
; --- %2 : Memory address to save the values
; --- %3 : End of input delimiter
%macro getValue 3
    xor r9, r9 ; Clears counter

    getInput %1, MAX_VAL_LEN

    ; Checks for any spaces
    %%_verifyLoop:
        mov r12b, BYTE[%1 + r9]

        ; Valid character
        cmp r12b, '-'
        je %%_inc 

        cmp r12b, %3
        je %%_addVal

        cmp r12b, 'A'
        jge _valueError

        cmp r12b, '0'
        jl _valueError

        cmp r12b, ' '
        je _valueError

        %%_inc: 
            inc r9
            jmp %%_verifyLoop

    ; Corrects user input
    %%_addVal:
        xor r9, r9 ; Clears counter to use it again
        xor rcx, rcx
        xor r13, r13 ; Keeps count of the amount of '-'s
        %%_saveVal:            
            mov r12b, BYTE[%1 + r9]
            mov BYTE[%2 + r15], r12b ; Saves the first character

            inc r9
            inc r15

            cmp r12b, '0'
            jge %%_incNums

            cmp r12b, '-'
            je %%_incMinus

            %%_incNums:
                inc rcx
                jmp %%_continueSaving

            %%_incMinus:
                inc r13
                
                ; More than one negative sign as value input
                cmp r13, 1
                jg _valueError

            
            %%_continueSaving:
                mov r12b, BYTE[%1 + r9] ; Loads the next character

                cmp r12b, %3
                je %%_endVal

                jmp %%_saveVal

            %%_endVal:
                ; No numbers detected
                cmp rcx, 0
                je _valueError

                inc r15
                mov BYTE[%2 + r15], ','
                inc r15
%endmacro


; Shows the variables on screen and gets their value
%macro getVarData 0
    xor r8, r8 ; Points to a specific variable
    xor r15, r15 ; Keeps track of where to add the value
    xor r12, r12 ; Looks for the message to show
    xor r13, r13 ; Keeps track of the start of the next variable
    ; Checks for each variable saved
    %%_initial:
        cmp r14, 0 ; Theres no variables left
        je %%_noVals

        print msgEnterValues, msgEnterValuesLen
        
        %%_varsLoop:
            clear pivot, MAX_VAL_LEN
            clear varPivot, MAX_EXP_LEN
            xor r12, r12

            cmp r14, 1
            je %%_printLast

            jmp %%_printNextVar

            %%_printLast:
                print msgLastVar, msgLastVarLen


        %%_printNextVar:
            mov bl, BYTE[vars + r8 + r12]    
            
            cmp bl, ','
            je %%_readyPrint

            ; Saves the memory
            mov BYTE[varPivot + r12], bl

            ; Keeps looping
            inc r12
            jmp %%_printNextVar

        %%_readyPrint:
            add r8, r12
            inc r8   ; Puts the pointer onto the next variable
            xor r12, r12

            %%_printNormal:
                print varPivot, MAX_EXP_LEN
                print msgEqual, msgEqualLen


        cmp r14, 1 ; Finishes checking the variables
        je %%_getValueFinal

        %%_getValueComma:
            mov bl, ','
            getValue pivot, values, bl 
            dec r14

            jmp %%_varsLoop

        %%_getValueFinal:

            mov bl, 10
            getValue pivot, values, bl 
            dec r14

    %%_noVals: ; Exits the loop

%endmacro


; --- Converts string to postfix
; %1 : String (infix Expression)
; %2 : String (Empty memory)
%macro infix2postfix 2
    xor rcx, rcx ; %1 counter
    xor r15, r15 ; %2 counter
    xor r8, r8 ; validate pop register
    push 4; end of stack flag
    %%_loop:
        mov bl, BYTE[%1 + rcx]   


        ; Finds if the string finishes
        cmp bl, 10
        je %%_almostEnd

        cmp bl, '^'
        je %%_expVerify

        ; Validates that it's indeed a number
        cmp bl, '0'
        jge %%_addVar
        ;;

        cmp bl, '-'
        je %%_addVar

            ; Adds ' ' at the end of the number
            mov BYTE[%2 + r15], 32
            inc r15

       ; Symbol Validations
        %%_symbolVerify:


	        cmp bl, '+'
	        je %%_sumVerify

	        cmp bl, '/'
	        je %%_divVerify

	        cmp bl, '*'
	        je %%_mulVerify

	        cmp bl, '%'
	        je %%_modVerify





        %%_continueLoop:
	        ; Keeps looping
	        inc rcx
	        jmp %%_loop



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	    %%_sumVerify:


            pop r8

            cmp r8, 4
            je %%_endOfStack4

            cmp r8, '^'
            je %%_biggerPrecedence 

            cmp r8, '/'
            je %%_biggerPrecedence
            
            cmp r8, '*'
            je %%_biggerPrecedence

            cmp r8, '%'
            je %%_biggerPrecedence


            push rbx
            mov rbx, r8
            mov BYTE[%2 + r15], bl
            inc r15 


            jmp %%_continueLoop



	    %%_divVerify:

            pop r8

            cmp r8, 4
            je %%_endOfStack4

            cmp r8, '^'
            je %%_biggerPrecedence 

            cmp r8, '/'
            je %%_biggerPrecedence
            
            cmp r8, '*'
            je %%_biggerPrecedence

            cmp r8, '%'
            je %%_biggerPrecedence


            jmp %%_lowerPrecedence

        ; *    
	    %%_mulVerify:

            pop r8

            cmp r8, 4
            je %%_endOfStack4

            cmp r8, '^'
            je %%_biggerPrecedence 

            cmp r8, '/'
            je %%_biggerPrecedence
            
            cmp r8, '*'
            je %%_biggerPrecedence

            cmp r8, '%'
            je %%_biggerPrecedence


            jmp %%_lowerPrecedence

	    ; %
	    %%_modVerify:

            pop r8

            cmp r8, 4
            je %%_endOfStack4

            cmp r8, '^'
            je %%_biggerPrecedence 

            cmp r8, '/'
            je %%_biggerPrecedence
            
            cmp r8, '*'
            je %%_biggerPrecedence

            cmp r8, '%'
            je %%_biggerPrecedence


            jmp %%_lowerPrecedence
	    ; ^
	    %%_expVerify:
            ; Adds ' ' at the end of the number
            mov BYTE[%2 + r15], 32
            inc r15
            pop r8

            cmp r8, 4
            je %%_endOfStack4

            push r8
            push rbx
            jmp %%_continueLoop

            jmp %%_biggerPrecedence



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;




        ; Adds the variable to %2
        %%_addVar:

            mov BYTE[%2 + r15], bl

            inc rcx
            inc r15

            mov bl, BYTE[%1 + rcx]

            jmp %%_loop


        %%_biggerPrecedence:
            mov rdx, r8
            mov BYTE[%2 + r15], dl
            inc r15
            jmp %%_symbolVerify

        %%_lowerPrecedence:
            push r8
            push rbx
            jmp %%_continueLoop


        %%_endOfStack4:
            push r8
        	push rbx
        	jmp %%_continueLoop


    %%_almostEnd:

            ; Adds ' ' at the end of the number
            mov BYTE[%2 + r15], 32
            inc r15
            pop r8
            
            push r8
        %%_emptyStackLoop:
            pop r8
            cmp r8, 4
            je %%_end

            mov rbx, r8
            mov BYTE[%2 + r15], bl
            inc r15

            jmp %%_emptyStackLoop


    %%_end:
        mov bl, 10
        mov BYTE[%2 + r15], bl


%endmacro

; Macro obtained from previous short homework (Operations).
; Edited to fit in with this program
; %1 : Reserved memory 
; %2 : Storage (Register)
%macro saveNumber 2
    xor rcx, rcx  ; Cleans counter register
    xor %2, %2    ; Cleans storage register
    xor r12, r12  ; Amount of digits in the number
    xor r13, r13

    xor r10, r10  
    mov r10, 1    ; Cleans and sets decimal position

    mov rbx, 10

    %%_vloop:
        mov al, BYTE[%1 + rcx] ; Moves one character to AL
        inc rcx

        cmp al, '-'
        je %%_incMinCount

        cmp al, 10
        je %%_sloop   ; Validates end of line

        %%_increment:
            inc r12       ; Increments amount of digits
            jmp %%_vloop

        %%_incMinCount:
            inc r13      ; Increments amount of '-'s found
            jmp %%_increment

    %%_sloop: ; Loop to convert char to num 
        xor rax, rax

        dec r12
        mov al, BYTE[%1 + r12]

        cmp al, '-'
        je %%_end

        sub rax, 48   ; Converts from char to num

        mul r10       ; Multiplies for position

        add %2, rax   ; Adds to storage register
        jo _overflowError

        cmp r12, 0
        je %%_end      ; No more digits

        xor rax, rax 
        mov rax, r10
        mul rbx
        xor r10, r10
        mov r10, rax ; Increments position
        
        jmp %%_sloop
    
    %%_end:
        cmp r13, 0 ; Checks for amount of '-'s found
        je %%_realEnd

        ; Checks if there needs to be a negation
        ; Checks if theres an even or odd number of '-'
        or r13, 0xFFFFFFFFFFFFFFFE
        not r13

        cmp r13, 1 ; Even number of '-'s
        je %%_realEnd

        ; complement
        neg %2
    %%_realEnd:        
        xor r10, r10 
        xor r12, r12  ; Amount of digits in the number
        xor r13, r13
        xor rcx, rcx  ; Cleans counter register

%endmacro

; Gets the corresponding value of a variable
; %1 : Variable to look for
; %2 : Space to store value
%macro findValue 2 
    xor rcx, rcx ; Counter
    xor r9, r9 ; Delimiter counter
    xor rax, rax ; Counter

    %%_checkParity:
        mov r14b, BYTE[%1 + rcx]
        mov r15b, BYTE[vars + rax]

        cmp r14b, 0 ; Accidental 0
        je %%_incRCX

        cmp r15b, 0 ; Accidental 0
        je %%_incRAX

        cmp r14b, r15b ; Same data
        je %%_checkEnd

        jne %%_checkNextVar ; Data doesnt match

            %%_incRCX:
                inc rcx
                jmp %%_checkParity

            %%_incRAX:
                inc rax
                jmp %%_checkParity

        %%_checkEnd:
            cmp r14b, ','
            je %%_varFound

            ; Not the end
            inc rcx
            inc rax
            jmp %%_checkParity

        %%_checkNextVar:
            mov r15b, BYTE[vars + rax]

            cmp r15b, ','
            je %%_newFound

            inc rax
            jmp %%_checkNextVar

            %%_newFound:
                xor rcx, rcx ; Start looping %1 again
                inc r9 ; Delimiter counter up
                inc rax ; Check next var
                jmp %%_checkParity

    %%_varFound: ; Checks the values
        xor rcx, rcx

        %%_checkVal:
            cmp r9, 0
            je %%_valueFound
            jmp %%_nextVal

        %%_valueFound:
            mov r14b, BYTE[values + rcx]

            cmp r14b, 0
            je %%_accidental0

            cmp r14b, ','
            je %%_endVal

            mov BYTE[%2 + rdx], r14b ; Saves each digit

            inc rdx
            inc rcx
            jmp %%_valueFound

            %%_accidental0:
                inc rcx
                jmp %%_valueFound

        %%_nextVal:
            mov r14b, BYTE[values + rcx]
            
            cmp r14b, ','
            je %%_delimiterFound

            inc rcx
            jmp %%_nextVal

            %%_delimiterFound:
                dec r9
                inc rcx
                jmp %%_checkVal

    %%_endVal:
        mov BYTE[%2 + rdx], 10
%endmacro


; Evaluates the post fix expression
; %1 : Postfix expression
; %2 : Destiny register
%macro evaluatePostfix 2
    xor rax, rax ; Operations will be done here
    xor r8, r8 ; Second number will be saved here
    xor r11, r11 ; %1 Counter
    xor r15, r15 ; Saves byte
    xor rcx, rcx ; Power loop counter

    %%_loop:
        mov r15b, BYTE[%1 + r11]

        ; End of postfix string
        cmp r15b, 10
        je %%_finishEval

        cmp r15b, '-'
        je %%_varOrNum

        cmp r15b, '0'
        jl %%_operation

        cmp r15b, '^'
        je %%_operation

        cmp r15b, 'A'
        jge %%_findVar

        cmp r15b, '0'
        jge %%_saveNum

        inc r11
        jmp %%_loop

    %%_varOrNum:
        mov r14b, BYTE[%1 + r11 + 1]

        cmp r14b, 'A'
        jge %%_findVar

        cmp r14b, '0'
        jge %%_saveNum

    %%_operation:
        xor rax, rax

        pop r8 ; Gets second num
        pop rax ; Gets first num

        ; Checks which operation
        cmp r15b, '+'
        je %%_add

        cmp r15b, '^'
        je %%_power

        cmp r15b, '*'
        je %%_multiply

        cmp r15b, '/'
        je %%_div

        cmp r15b, '%'
        je %%_mod

    %%_add:
        add rax, r8
        jo _overflowError
        jmp %%_saveRes

    %%_multiply:
        ; FIX NEGATIVE MULTIPLICATION
        imul r8
        jo _overflowError
        jmp %%_saveRes

    %%_mod:
        cmp r8, 0
        je _zeroDivError

        cqo 
        idiv r8

        mov rax, rdx

        ; Saves result
        jo _overflowError

        jmp %%_saveRes

    %%_power:
        cmp r8, 0
        je %%_expEquals0

        cmp r8, 1
        je %%_expEquals1


        bsr r9, r8  ; Checks for highest significant bit 
        cmp r9, 63 ; Most significant bit in the highest position
        je %%_expEqualsNeg

        bsr r9, rax
        cmp r9, 63
        je %%_negate

        jmp %%_continue

        %%_negate:
            neg rax
            mov bl, 1

        %%_continue:
            mov rcx, r8
            mov r8, rax
            dec rcx
        %%_multiplyPow:
        ; rax * r8
        	cmp rcx, 0
        	je %%_checkSign
        
        	imul r8
            jo _overflowError

        	dec rcx
        	jmp %%_multiplyPow

        %%_expEquals0:
        	mov rax, 1
        	jmp %%_saveRes
        
        %%_expEquals1:
        	jmp %%_saveRes

        %%_expEqualsNeg:
        	mov rax, 0
        	jmp %%_saveRes

        %%_checkSign:
            cmp bl, 1
            je %%_negRes
            jmp %%_saveRes

            %%_negRes:
                neg rax
                jmp %%_saveRes


    %%_div:
        ; FIX NEGATIVE DIVISION
        cmp r8, 0
        je _zeroDivError

        div r8
        jo _overflowError
        jmp %%_saveRes

    %%_saveRes:
        push rax ; Saves the value again

        inc r11
        jmp %%_loop

    %%_saveNum:
        mov r15b, BYTE[%1 + r11]
        inc r11

        ; Finishes with the number
        cmp r15b, ' '
        je %%_stop

        cmp r15b, 0
        je %%_incEmpty

        mov BYTE[calcPivot + r10], r15b

        jmp %%_incrementNum

        ; Reloops
        %%_incEmpty:
            inc r11
            jmp %%_saveNum

        %%_incrementNum:
            inc r10
            jmp %%_saveNum

        ; Saves the number
        %%_stop:
            mov BYTE[calcPivot + r10], 10
            saveNumber calcPivot, r8
            push r8

            xor r10, r10

            jmp %%_loop

    %%_findVar:

        xor rdx, rdx ; Writing counter for corresponding number    

        cmp r15b, '-'
        je %%_saveMin

        xor r10, r10
        jmp %%_saveVariable

        %%_saveMin: ; Saves negative symbol
            mov BYTE[calcPivot + rdx], r15b
            inc rdx
            inc r11
            mov r15b, BYTE[%1 + r11]

        %%_saveVariable:
            mov r15b, BYTE[%1 + r11]

            cmp r15b, ' '
            je %%_saved

            cmp r15b, 0
            je %%_ignore

            mov BYTE[correspondingVar + r10], r15b
                
            %%_incSave:
                inc r11
                inc r10
                jmp %%_saveVariable

            %%_ignore:
                inc r11
                jmp %%_saveVariable

            %%_saved:
                mov BYTE[correspondingVar + r10], ','

            ; Gets the value according to the variable
            findValue correspondingVar, calcPivot

            xor r8, r8
            saveNumber calcPivot, r8
            push r8

            inc r11

            jmp %%_loop
    %%_finishEval:
        xor %2, %2
        pop %2
%endmacro


; printDigit macro obtained from previous short homework (Operations).
; Edited to show a signed number
; Prints the number saved in a register (Binary to Decimal)
; %1 Register holding a number 
; %2 Reserved memory
%macro printDigit 2
    mov rax, %1

    mov r8, 10
    xor rcx, rcx ; Counter
    xor r9, r9   ; Temp holder
    push 10      ; End flag pushed 

    %%checkSign:
        bsr r9, rax   ; Checks for highest significant bit 

        cmp r9, 63 ; Most significant bit in the highest position
        je %%_complement

        jmp %%rloop
    
        %%_complement:
            mov BYTE[%2 + rcx], '-'
            inc rcx
            
            neg rax
    %%rloop:
        xor rdx, rdx
        div r8
        add rdx, 48 ; Convertion to character
        push rdx    ; 

        cmp rax, 0
        je %%sloop ; RAX empty
    
        jmp %%rloop

    %%sloop:
        xor r9, r9 ; Temp holder
        pop r9

        mov [%2 + rcx], r9
        inc rcx

        cmp r9, 10
        je %%_end

        jmp %%sloop
    %%_end:

    print %2, MAX_VAL_LEN
%endmacro
