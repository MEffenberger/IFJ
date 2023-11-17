.IFJcode23

DEFVAR GF@a



# a = 6+8*10
PUSHS int@6
PUSHS int@8
PUSHS int@10
MULS    
ADDS    


POPS GF@a
write GF@a

# s = "cau" + "ahoj"

DEFVAR GF@s
DEFVAR GF@s1
MOVE GF@s1 string@cau
DEFVAR GF@s2
MOVE GF@s2 string@6
CONCAT GF@s GF@s1 GF@s2

write GF@s

# a = 6+"cau"+2


#DEFVAR GF@cislo1
#MOVE GF@cislo1 int@6
#DEFVAR GF@cislo2
#MOVE GF@cislo2 float@0x1.8cccccccccccdp+2

DEFVAR GF@vysledek

#LT GF@vysledek GF@cislo1 GF@cislo2
#write GF@vysledek


DEFVAR GF@kokot
MOVE GF@kokot int@6
PUSHS float@0x1.8cccccccccccdp+2
PUSHS GF@kokot
MULS
DEFVAR GF@kokotdouble
POPS GF@kokotdouble
write GF@kokotdouble

