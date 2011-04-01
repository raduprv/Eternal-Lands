!!ARBvp1.0
PARAM constant = { 1, 3, 0, 0 };
TEMP R0, R1, R2, R3, R4;
ADDRESS A0;
ATTRIB texure_coord = vertex.attrib[8];
ATTRIB normal = vertex.attrib[2];
ATTRIB index = vertex.attrib[3];
ATTRIB weight = vertex.attrib[1];
ATTRIB position = vertex.attrib[0];
ATTRIB color = vertex.attrib[4];
PARAM mvp[4] = { state.matrix.mvp };
PARAM nm[4] = { state.matrix.modelview.invtrans };
PARAM modelview[4] = { state.matrix.modelview };
PARAM diffuse_0 = state.lightprod[0].diffuse;
PARAM diffuse_1 = state.lightprod[1].diffuse;
PARAM diffuse_2 = state.lightprod[2].diffuse;
PARAM diffuse_3 = state.lightprod[3].diffuse;
PARAM diffuse_4 = state.lightprod[4].diffuse;
PARAM diffuse_5 = state.lightprod[5].diffuse;
PARAM diffuse_6 = state.lightprod[6].diffuse;
PARAM diffuse_7 = state.lightprod[7].diffuse;
PARAM attenuation_0 = state.light[0].attenuation;
PARAM attenuation_1 = state.light[1].attenuation;
PARAM attenuation_2 = state.light[2].attenuation;
PARAM attenuation_3 = state.light[3].attenuation;
PARAM attenuation_4 = state.light[4].attenuation;
PARAM attenuation_5 = state.light[5].attenuation;
PARAM attenuation_6 = state.light[6].attenuation;
PARAM attenuation_7 = state.light[7].attenuation;
PARAM light_position_0 = state.light[0].position;
PARAM light_position_1 = state.light[1].position;
PARAM light_position_2 = state.light[2].position;
PARAM light_position_3 = state.light[3].position;
PARAM light_position_4 = state.light[4].position;
PARAM light_position_5 = state.light[5].position;
PARAM light_position_6 = state.light[6].position;
PARAM light_position_7 = state.light[7].position;
PARAM ambient = state.lightprod[7].ambient;
PARAM scene_color = state.lightmodel.scenecolor;
PARAM texgen_s = state.texgen[0].eye.s;
PARAM texgen_t = state.texgen[0].eye.t;
PARAM texgen_r = state.texgen[0].eye.r;
PARAM texgen_q = state.texgen[0].eye.q;
PARAM matrix[%d] = { program.local[0..%d] };

MOV result.texcoord[1].xy, texure_coord.xyxx;	

MUL R0, index, constant.y;
FLR R4, R0;

ARL A0.x, R4.x;
DP3 R0.x, matrix[A0.x].xyzx, normal.xyzx;
DP3 R0.y, matrix[A0.x + 1].xyzx, normal.xyzx;
DP3 R0.z, matrix[A0.x + 2].xyzx, normal.xyzx;
MUL R1.xyz, R0.xyzx, weight.x;

ARL A0.x, R4.y;
DP3 R0.x, matrix[A0.x].xyzx, normal.xyzx;
DP3 R0.y, matrix[A0.x + 1].xyzx, normal.xyzx;
DP3 R0.z, matrix[A0.x + 2].xyzx, normal.xyzx;
MAD R1.xyz, R0.xyzx, weight.y, R1.xyzx;

ARL A0.x, R4.z;
DP3 R0.x, matrix[A0.x].xyzx, normal.xyzx;
DP3 R0.y, matrix[A0.x + 1].xyzx, normal.xyzx;
DP3 R0.z, matrix[A0.x + 2].xyzx, normal.xyzx;
MAD R1.xyz, R0.xyzx, weight.z, R1.xyzx;

ARL A0.x, R4.w;
DP3 R0.x, matrix[A0.x].xyzx, normal.xyzx;
DP3 R0.y, matrix[A0.x + 1].xyzx, normal.xyzx;
DP3 R0.z, matrix[A0.x + 2].xyzx, normal.xyzx;
MAD R1.xyz, R0.xyzx, weight.w, R1.xyzx;

DP3 R0.x, nm[0], R1.xyzx;
DP3 R0.y, nm[1], R1.xyzx;
DP3 R0.z, nm[2], R1.xyzx;

DP3 R1.x, R0.xyzx, R0.xyzx;
RSQ R1.x, R1.x;
MUL R3.xyz, R1.x, R0.xyzx;

ARL A0.x, R4.x;
DPH R0.x, position.xyzx, matrix[A0.x];
DPH R0.y, position.xyzx, matrix[A0.x + 1];
DPH R0.z, position.xyzx, matrix[A0.x + 2];
MUL R1.xyz, R0.xyzx, weight.x;

ARL A0.x, R4.y;
DPH R0.x, position.xyzx, matrix[A0.x];
DPH R0.y, position.xyzx, matrix[A0.x + 1];
DPH R0.z, position.xyzx, matrix[A0.x + 2];
MAD R1.xyz, R0.xyzx, weight.y, R1.xyzx;

ARL A0.x, R4.z;
DPH R0.x, position.xyzx, matrix[A0.x];
DPH R0.y, position.xyzx, matrix[A0.x + 1];
DPH R0.z, position.xyzx, matrix[A0.x + 2];
MAD R1.xyz, R0.xyzx, weight.z, R1.xyzx;

ARL A0.x, R4.w;
DPH R0.x, position.xyzx, matrix[A0.x];
DPH R0.y, position.xyzx, matrix[A0.x + 1];
DPH R0.z, position.xyzx, matrix[A0.x + 2];
MAD R1.xyz, R0.xyzx, weight.w, R1.xyzx;

DPH result.position.x, R1.xyzx, mvp[0];
DPH result.position.y, R1.xyzx, mvp[1];
DPH result.position.z, R1.xyzx, mvp[2];
DPH result.position.w, R1.xyzx, mvp[3];

DPH R0.x, R1.xyzx, modelview[0];
DPH R0.y, R1.xyzx, modelview[1];
DPH R0.z, R1.xyzx, modelview[2];
DPH R0.w, R1.xyzx, modelview[3];

ABS result.fogcoord, R0.z;

MAD R1.xyz, R0.xyzx, -light_position_0.w, light_position_0.xyzx;
DP3 R2.x, R1.xyzx, R1.xyzx;
RSQ R2.y, R2.x;
MUL R1.xyz, R1.xyzx, R2.y;
DST R2, R2.xxxx, R2.yyyy;
MUL R2.yz, R2.yzxx, light_position_0.wwww;
DP3 R2.w, R2, attenuation_0;
DP3 R1.x, R1.xyzx, R3.xyzx;
MAX R1.x, R1.x, constant.z;
RCP R2.w, R2.w;
MUL R1.x, R1.x, R2.w;
ADD R4, scene_color, ambient;
MAD R4, R1.xxxx, diffuse_0, R4;

MAD R1.xyz, R0.xyzx, -light_position_5.w, light_position_5.xyzx;
DP3 R2.x, R1.xyzx, R1.xyzx;
RSQ R2.y, R2.x;
MUL R1.xyz, R1.xyzx, R2.y;
DST R2, R2.xxxx, R2.yyyy;
MUL R2.yz, R2.yzxx, light_position_5.wwww;
DP3 R2.w, R2, attenuation_5;
DP3 R1.x, R1.xyzx, R3.xyzx;
MAX R1.x, R1.x, constant.z;
RCP R2.w, R2.w;
MUL R1.x, R1.x, R2.w;
MAD R4, R1.xxxx, diffuse_5, R4;

MAD R1.xyz, R0.xyzx, -light_position_6.w, light_position_6.xyzx;
DP3 R2.x, R1.xyzx, R1.xyzx;
RSQ R2.y, R2.x;
MUL R1.xyz, R1.xyzx, R2.y;
DST R2, R2.xxxx, R2.yyyy;
MUL R2.yz, R2.yzxx, light_position_6.wwww;
DP3 R2.w, R2, attenuation_6;
DP3 R1.x, R1.xyzx, R3.xyzx;
MAX R1.x, R1.x, constant.z;
RCP R2.w, R2.w;
MUL R1.x, R1.x, R2.w;
MAD R4, R1.xxxx, diffuse_6, R4;

MAD R1.xyz, R0.xyzx, -light_position_7.w, light_position_7.xyzx;
DP3 R2.x, R1.xyzx, R1.xyzx;
RSQ R2.y, R2.x;
MUL R1.xyz, R1.xyzx, R2.y;
DST R2, R2.xxxx, R2.yyyy;
MUL R2.yz, R2.yzxx, light_position_7.wwww;
DP3 R2.w, R2, attenuation_7;
DP3 R1.x, R1.xyzx, R3.xyzx;
MAX R1.x, R1.x, constant.z;
RCP R2.w, R2.w;
MUL R1.x, R1.x, R2.w;
MAD R4, R1.xxxx, diffuse_7, R4;

SLT R1, color, constant.zzzz;
ADD R2, -R1, constant.xxxx;
MUL R3, R2, color;

MAD result.color.primary, R4, R1, R3;

DP4 R4.x, texgen_s, R0;
DP4 R4.y, texgen_t, R0;
DP4 R4.z, texgen_r, R0;
DP4 R4.w, texgen_q, R0;

MUL R3, R2, constant.zzxx;

MAD result.texcoord[0], R4, R1, R3;

END
