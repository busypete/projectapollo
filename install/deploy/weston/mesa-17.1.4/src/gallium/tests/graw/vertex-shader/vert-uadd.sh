VERT
DCL IN[0]
DCL IN[1]
DCL OUT[0], GENERIC[0]
DCL OUT[1], GENERIC[1]
IMM[0] INT32 {1, 0, 0, 0}
MOV OUT[0], IN[0]
UADD OUT[1].x, IN[1].xxxx, IMM[0].xxxx
END
