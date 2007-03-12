/*!
 * \file
 * \ingroup misc_utils
 * \brief common functions to handle the MD5 sum of data. This comes from RSA Data Security Inc.
 */

/* md5.h - header file for md5.c */
/* RSA Data Security, Inc., MD5 Message-Digest Algorithm */

/* NOTE: Numerous changes have been made; the following notice is
included to satisfy legal requirements.

Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
*/

#ifndef H__MD5
#define H__MD5

#ifdef __cplusplus
extern "C" {
#endif

// sizeof(unsigned long) == 8 on 64 bit system and MD5Close will segfault.
//typedef unsigned long UINT4;
typedef unsigned int UINT4;
typedef unsigned char MD5_DIGEST[16];

/*!
 * The MD5 structure used to generate MD5 hash values
 */
typedef struct
{
  UINT4 state[4];
  UINT4 count[2];
  unsigned char buffer[64];
} MD5;

/*!
 * \ingroup misc_utils
 * \brief
 *
 *      Detail
 *
 * \param md5
 */
void MD5Open(MD5 * md5);

/*!
 * \ingroup misc_utils
 * \brief
 *
 *      Detail
 *
 * \param md5
 * \param input
 * \param input_len
 */
void MD5Digest(MD5 *md5, const void *input, unsigned int input_len);

/*!
 * \ingroup misc_utils
 * \brief
 *
 *      Detail
 *
 * \param md5
 * \param digest
 *
 * \sa MD5Digest
 */
void MD5Close(MD5 *md5, MD5_DIGEST digest);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
