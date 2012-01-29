!!ARBvp1.0
PARAM constant = { 1, 3, 0, 0 };
TEMP R0, R1, R2;
ADDRESS A0;
ATTRIB texure_coord = vertex.attrib[8];
ATTRIB index = vertex.attrib[3];
ATTRIB weight = vertex.attrib[1];
ATTRIB position = vertex.attrib[0];
PARAM mvp[4] = { state.matrix.mvp };
PARAM matrix[%d] = { program.local[0..%d] };

MUL R0, index, constant.y;
FLR R2, R0;

ARL A0.x, R2.x;
DPH R0.x, position.xyzx, matrix[A0.x];
DPH R0.y, position.xyzx, matrix[A0.x + 1];
DPH R0.z, position.xyzx, matrix[A0.x + 2];
MUL R1.xyz, R0.xyzx, weight.x;

ARL A0.x, R2.y;
DPH R0.x, position.xyzx, matrix[A0.x];
DPH R0.y, position.xyzx, matrix[A0.x + 1];
DPH R0.z, position.xyzx, matrix[A0.x + 2];
MAD R1.xyz, R0.xyzx, weight.y, R1.xyzx;

ARL A0.x, R2.z;
DPH R0.x, position.xyzx, matrix[A0.x];
DPH R0.y, position.xyzx, matrix[A0.x + 1];
DPH R0.z, position.xyzx, matrix[A0.x + 2];
MAD R1.xyz, R0.xyzx, weight.z, R1.xyzx;

ARL A0.x, R2.w;
DPH R0.x, position.xyzx, matrix[A0.x];
DPH R0.y, position.xyzx, matrix[A0.x + 1];
DPH R0.z, position.xyzx, matrix[A0.x + 2];
MAD R1.xyz, R0.xyzx, weight.w, R1.xyzx;

DPH result.position.x, R1.xyzx, mvp[0];
DPH result.position.y, R1.xyzx, mvp[1];
DPH result.position.z, R1.xyzx, mvp[2];
DPH result.position.w, R1.xyzx, mvp[3];

MOV result.color, constant.xxxx;

MOV result.texcoord[0].xy, texure_coord.xyxx;	

END
