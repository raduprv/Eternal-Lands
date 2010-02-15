#include <stdio.h>
#include <string.h>
#include "calstripper.h"

#define SQ(a) ((a)*(a))
#define EPSILON 0.0001

int is_frame_equal(cafkeyframe *a, cafkeyframe *b){
	float norm;

	norm= SQ(a->tx-b->tx)+SQ(a->ty-b->ty)+SQ(a->tz-b->tz)+
	      SQ(a->rx-b->rx)+SQ(a->ry-b->ry)+SQ(a->rz-b->rz)+SQ(a->rw-b->rw);
	
	return (norm<EPSILON);
	
}

int is_still(caftrack *tr){
	int i,b=1;
	for(i=1;i<tr->keyframes;i++)
		b&=is_frame_equal(&tr->frames[0],&tr->frames[i]);
	return b;
}


int blend(caftrack *tr, caftrack *fr, cafanim *bl){
	int kf;
	int bltr=-1;
	

	if (!bl) return 0;
	//search bone in bl
	for(kf=0;kf<bl->header.tracks;kf++)
		if (bl->tracks[kf].bone==tr->bone) {bltr=kf; break;}
	
	if(bltr==-1) return 0;

	for(kf=1;kf<tr->keyframes;kf++){
		tr->frames[kf].rx=bl->tracks[bltr].frames[0].rx+(fr->frames[kf].rx-fr->frames[kf-1].rx);
		tr->frames[kf].ry=bl->tracks[bltr].frames[0].ry+(fr->frames[kf].ry-fr->frames[kf-1].ry);
		tr->frames[kf].rz=bl->tracks[bltr].frames[0].rz+(fr->frames[kf].rz-fr->frames[kf-1].rz);
		tr->frames[kf].rw=bl->tracks[bltr].frames[0].rw+(fr->frames[kf].rw-fr->frames[kf-1].rw);	
		tr->frames[kf].tx=bl->tracks[bltr].frames[0].tx+(fr->frames[kf].tx-fr->frames[kf-1].tx);
		tr->frames[kf].ty=bl->tracks[bltr].frames[0].ty+(fr->frames[kf].ty-fr->frames[kf-1].ty);
		tr->frames[kf].tz=bl->tracks[bltr].frames[0].tz+(fr->frames[kf].tz-fr->frames[kf-1].tz);
	}
	//assuming frame[0].time is 0
	memcpy(&tr->frames[0],&bl->tracks[bltr].frames[0],sizeof(cafkeyframe));
	return 1;
}

int hardblend(caftrack *tr, caftrack *fr, cafanim *bl){
	int kf;
	int bltr=-1;
	float time;	

	if (!bl) return 0;
	//search bone in bl
	for(kf=0;kf<bl->header.tracks;kf++)
		if (bl->tracks[kf].bone==tr->bone) {bltr=kf; break;}
	
	if(bltr==-1) return 0;

	for(kf=0;kf<tr->keyframes;kf++){
		time=fr->frames[kf].time;
		if(kf==0||kf==tr->keyframes-1) tr->frames[kf]=bl->tracks[bltr].frames[0];
		else tr->frames[kf]=fr->frames[kf];
		tr->frames[kf].time=time;
	}
	//assuming frame[0].time is 0
	return 1;
}

void striptracks(cafanim *from, cafanim *to, csfdata *csf, cafanim *bl){
	int i,ctr=0,trs,tbl=0;

	trs=from->header.tracks;
	printf("Choose bone processing        [enter or kbfs+enter]\n");
	printf("---------------------------------------------------\n");
	printf("BONE            MOVING                       ACTION\n");
	printf("---------------------------------------------------\n");
	for(i=0;i<from->header.tracks;i++){
		char cmd[32];
		int st = is_still(&from->tracks[i]);
		printf("%-15s %4s   %sKeep/Blend/Force/%sStrip -->",csf->bones[from->tracks[i].bone].name,
		(st) ? ("N"):("Y"),(st) ? (""):("*"),(!st) ? (""):("*"));
		fflush(stdout);
		fgets(cmd,32,stdin);
		if(cmd[0]=='\n') cmd[0] = (st) ? ('s'):('k');
		switch(cmd[0]){
			case 'K':
			case 'k':
				memcpy(&to->tracks[ctr],&from->tracks[i],sizeof(caftrack));
				ctr++;
				break;
			case 'B':
			case 'b':
				if(!bl) {i--; continue;}
				memcpy(&to->tracks[ctr],&from->tracks[i],sizeof(caftrack));
				if(!blend(&to->tracks[ctr],&from->tracks[i],bl)) {
					printf("\nNot Blended, wrong bones, kept\n");
					ctr++;
					break;						
				}
				ctr++;
				tbl++;
				break;
			case 'F':
			case 'f':
				if(!bl) {i--; continue;}
				memcpy(&to->tracks[ctr],&from->tracks[i],sizeof(caftrack));
				if(!hardblend(&to->tracks[ctr],&from->tracks[i],bl)) {
					printf("\nNot Blended, wrong bones, kept\n");
					ctr++;
					break;						
				}
				ctr++;
				tbl++;
				break;
			case 'S':
			case 's':
				trs--;
				break;
			default:
				i--;
				continue;
		}		

	}
	memcpy(&to->header,&from->header,sizeof(cafheader));
	to->header.tracks=trs;
	printf("\n\nTracks Stripped: %i\n",from->header.tracks-to->header.tracks);
	printf("Tracks Blended: %2i\n",tbl);
	printf("Tracks Kept: %4i\n",to->header.tracks);
}


//CSF FUNCS
int read_csf(FILE *fc, csfdata *csf){
	int cb,ck;

	//read header	
	fread(&csf->header,sizeof(csfheader),1,fc);
	if(csf->header.magic[0]!='C'
	  ||csf->header.magic[1]!='S'||csf->header.magic[2]!='F'
	  ||csf->header.magic[3]!='\0') {
		printf("\nError in csf header %i %c %c %c %i\n",csf->header.version,csf->header.magic[0],csf->header.magic[1],csf->header.magic[2],csf->header.magic[3]); 
		return 0;
	} else printf("Csf Version: %i\n",csf->header.version);
	
	//read bones
	for(cb=0;cb<csf->header.bones;cb++){
		fread(&csf->bones[cb].name_len,sizeof(csf->bones[cb].name_len),1,fc);
		fread(&csf->bones[cb].name,1,csf->bones[cb].name_len,fc);
		fread(&csf->bones[cb].data,sizeof(csf->bones[cb].data),1,fc);
		fread(&csf->bones[cb].children,sizeof(int32_t),csf->bones[cb].data.num_children,fc);
	}
	return 1;
}

void print_bone_table(csfdata *csf){
	int cb;

	printf("\nBONES\n------------------------------\n");
        printf("  NAME                 ID     \n");
	printf("------------------------------\n");
	for(cb=0;cb<csf->header.bones;cb++){
		printf("%-22s %2i\n",csf->bones[cb].name, cb);
	}
	printf("------------------------------\n");
}


//CAF FUNCS
int read_caf(FILE *fc, cafanim *caf){
	int ct,ck;

	//read header	
	fread(&caf->header,sizeof(cafheader),1,fc);
	if(caf->header.magic[0]!='C'
	  ||caf->header.magic[1]!='A'||caf->header.magic[2]!='F'
	  ||caf->header.magic[3]!='\0') {
		printf("\nError in caf header %i %c %c %c %i\n",caf->header.version,caf->header.magic[0],caf->header.magic[1],caf->header.magic[2],caf->header.magic[3]); 
		return 0;
	} else printf("Caf Version: %i\n",caf->header.version);
	
	//read tracks
	for(ct=0;ct<caf->header.tracks;ct++){
		fread(&caf->tracks[ct].bone,sizeof(caf->tracks[ct].bone),1,fc);
		fread(&caf->tracks[ct].keyframes,sizeof(caf->tracks[ct].keyframes),1,fc);
		for(ck=0;ck<caf->tracks[ct].keyframes;ck++){
			fread(&caf->tracks[ct].frames[ck],sizeof(cafkeyframe),1,fc);
		}
	}
	return 1;
}

int write_caf(FILE *fo, cafanim *caf){
	
	int ct,ck;	

	//write header
	fwrite(&caf->header,sizeof(cafheader),1,fo);
	//write tracks
	for(ct=0;ct<caf->header.tracks;ct++){
		fwrite(&caf->tracks[ct].bone,sizeof(caf->tracks[ct].bone),1,fo);
		fwrite(&caf->tracks[ct].keyframes,sizeof(caf->tracks[ct].keyframes),1,fo);
		for(ck=0;ck<caf->tracks[ct].keyframes;ck++){
			fwrite(&caf->tracks[ct].frames[ck],sizeof(cafkeyframe),1,fo);
		}
	}
	return 1;
}

void helptext(){
	printf("\nEternal Lands cal3d animation tool\n");
	printf("\nUsage: calstripper source.caf dest.caf source.csf [blend.caf]\n");
        printf("------------------------------------------------------------\n");
	printf("For each bone in source.caf you can choose to:\n");
	printf(" - (K)eep the source keyframes (default if bone is moving)\n");
	printf(" - (S)trip the source keyframes (default if bone is not moving)\n");
	printf(" - (B)lend the source keyframes  with blend.caf (if specified).\n");
	printf("   By blending you keep the bone in blend.caf and add the rotations\n");
	printf("   and traslations of the corresponding bone in source.caf\n");
	printf(" - (F)orce blending. The first and last source keyframes are\n");
	printf("   substituted with the corresponding ones in blend.caf\n");
	printf("\nOutput is written in dest.caf\n");
	printf("source.csf is the skeleton file, needed to get bone names\n\n");
}

int main (int argc, char *argv[]) {

	FILE *fc,*fo,*fs,*fb;
	//static here is to prevent stack overflow in windows (-_-')
	static cafanim caf_in,caf_to,caf_blend;
	static csfdata csf;
	

	if (argc<4) {
		helptext();
		return 0;
	}

	printf("Reading Skeleton...");
	fs = fopen(argv[3],"rb");
	if(!fs) { printf("\nCan't open %s\n",argv[3]); return 0;}
	if(!read_csf(fs,&csf)) return 0;
	fclose(fs);

	if(argc>=5){
		printf("Reading Blending Animation...");
		fb = fopen(argv[4],"rb");
		if(!fb) { printf("\nCan't open %s\n",argv[4]); return 0;}
		if(!read_caf(fb,&caf_blend)) return 0;
		fclose(fb);
	} else printf("No Blending Animation given...\n");

	printf("Reading Source Animation...");
	fc = fopen(argv[1],"rb");
	if(!fc) { printf("\nCan't open %s\n",argv[1]); return 0;}
	if(!read_caf(fc,&caf_in)) return 0;
	fclose(fc);

	print_bone_table(&csf);	
	striptracks(&caf_in,&caf_to,&csf,(argc>=5) ? (&caf_blend):(NULL));	

	fo = fopen(argv[2],"wb");
	if(!fo) { printf("\nCan't open %s\n",argv[2]); return 0;}
	if(!write_caf(fo,&caf_to)) return 0;
	fclose(fo);

	return 0;
}
