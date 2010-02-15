#ifndef __CALSTRIPPER__
#define __CALSTRIPPER__

#include <stdint.h>

//too lazy to dinamically alloc :P
#define MAX_TRACKS 100
#define MAX_FRAMES 500

//CAF
typedef struct _cafheader {
	int8_t magic[4]; //CAF\0
	int32_t version;
	float duration;
	int32_t tracks;
} cafheader;

typedef struct _cafkeyframe {
	float time;
	float tx,ty,tz;
	float rx,ry,rz,rw;
} cafkeyframe;

typedef struct _caftrack {
	int32_t bone;
	int32_t keyframes;
	cafkeyframe frames[MAX_FRAMES];
} caftrack;



typedef struct _caffile {
	cafheader header;
	caftrack tracks[MAX_TRACKS];
} cafanim;



//CSF
#define MAX_BONES 300
typedef struct _csfheader {
	int8_t magic[4]; //CAF\0
	int32_t version;
	int32_t bones;
} csfheader;


typedef struct _bonedata {
	float tx,ty,tz;
	float rx,ry,rz,rw;
	float tlx,tly,tlz;
	float rlx,rly,rlz,rlw;
	int32_t parent;
	int32_t num_children;
} bonedata;

typedef struct _csfbone {
	int32_t name_len;
	char name[100];
	bonedata data;
	int32_t children[MAX_BONES];	
} csfbone;


typedef struct _csffile {
	csfheader header;
	csfbone bones[MAX_BONES];
} csfdata;

/*
CSF FILE
Stored in this file is the hierarchy of bones that composes the skeleton.


description                length  type      comments
-------------------------  ------  --------  -----------------------------------
[header]
  magic token              4       const     "CSF\0"
  file version             4       integer   700
  number of bones          4       integer

[first bone]
  length of bone name      4       integer
  bone name                var     string
  translation x            4       float     relative translation to parent bone
  translation y            4       float
  translation z            4       float
  rotation x               4       float     relative rotation to parent bone
  rotation y               4       float     stored as a quaternion
  rotation z               4       float
  rotation w               4       float
  local translation x      4       float     translation to bring a vertex from
  local translation y      4       float     model space into bone space
  local translation z      4       float
  local rotation x         4       float     rotation to bring a vertex from
  local rotation y         4       float     model space into bone space
  local rotation z         4       float
  local rotation w         4       float
  parent bone id           4       integer   index to parent bone
  number of children       4       integer

  [first child]
    child bone id          4       integer   index to child bone

  [all other children]
    ...

[all other bones]
  ...

*/




/*
CAF FILE

All the keyframes of an animation are stored in this file. They are grouped by
tracks (one track per animated bone) and contain the time, the relative position
and the relative rotation to the parent bone.


description                length  type      comments
-------------------------  ------  --------  -----------------------------------
[header]
  magic token              4       const     "CAF\0"
  file version             4       integer   700
  duration                 4       float     length of animation in seconds
  number of tracks         4       integer

[first track]
  bone id                  4       integer   index to bone
  number of keyframes      4       integer

  [first keyframe]
    time                   4       float     time of keyframe in seconds
    translation x          4       float     relative translation to parent bone
    translation y          4       float
    translation z          4       float
    rotation x             4       float     relative rotation to parent bone
    rotation y             4       float     stored as a quaternion
    rotation z             4       float
    rotation w             4       float

  [all other keyframes]
    ...

[all other tracks]
  ...

*/
#endif
