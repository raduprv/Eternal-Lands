#ifdef EYE_CANDY

// I N C L U D E S ////////////////////////////////////////////////////////////

#include <iostream>
#include <math.h>
#include "eye_candy.h"

#include "math_cache.h"

const float PI = 3.141592654;

namespace ec
{

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

/*
MathCache_Hirange::MathCache_Hirange()
{
  // powf
  for (int i = 1; i < 10001; i++)
  {
    float base = (float)i / 10000;
    float power = 1.0;
    for (int j = 0; j < 63; j++)
      power /= 2.0;
    for (int j = 1; j < 129; j++)
    {
      powf_map[i][j] = powf(base, power);
      power *= 2.0;
    }
    base *= 2.0;
    powf_map[i][0] = 1.0;
  }
  for (int j = 1; j < 129; j++)
    powf_map[0][j] = powf(0.25 / 10000, j);
  powf_map[0][0] = 1;

  // sqrt
  float value = 1.0;
  for (int i = 0; i < 64; i++)
    value /= 2.0;
  for (int i = 0; i < 129; i++)
  {
    sqrt_map[i] = sqrt(value);
    value *= 2.0;
  }

  // Sin and cos
  for (int i = 1; i < 10002; i++)
  {
    sin_map[i] = sin((float)i / 10000 * (2 * PI));
    sin_map2[i] = sin((float)i / 10000 * (2 * PI) - PI);
    cos_map[i] = cos((float)i / 10000 * (2 * PI));
    cos_map2[i] = cos((float)i / 10000 * (2 * PI) - PI);
  }
}

// Sorry about the formatting on this one, but it's such a massive unrolled
// block that the brackets would make it unwieldy.
int MathCache_Hirange::get_lower_index(const float power)
{
  if (power < 1.0)
    if (power < 1.0 / 4294967296.0)
      if (power < 1.0 / 4294967296.0 / 65536.0)
        if (power < 1.0 / 4294967296.0 / 65536.0 / 256.0)
          if (power < 1.0 / 4294967296.0 / 65536.0 / 256.0 / 16.0)
            if (power < 1.0 / 4294967296.0 / 65536.0 / 256.0 / 16.0 / 4.0)
              if (power < 1.0 / 4294967296.0 / 65536.0 / 256.0 / 16.0 / 4.0 / 2.0)
                return 0;
              else
                return 1;
            else
              if (power < 1.0 / 4294967296.0 / 65536.0 / 256.0 / 16.0 / 2.0)
                return 2;
              else
                return 3;
          else
            if (power < 1.0 / 4294967296.0 / 65536.0 / 256.0 / 4.0)
              if (power < 1.0 / 4294967296.0 / 65536.0 / 256.0 / 4.0 / 2.0)
                return 4;
              else
                return 5;
            else
              if (power < 1.0 / 4294967296.0 / 65536.0 / 256.0 / 2.0)
                return 6;
              else
                return 7;
        else
          if (power < 1.0 / 4294967296.0 / 65536.0 / 16.0)
            if (power < 1.0 / 4294967296.0 / 65536.0 / 16.0 / 4.0)
              if (power < 1.0 / 4294967296.0 / 65536.0 / 16.0 / 4.0 / 2.0)
                return 8;
              else
                return 9;
            else
              if (power < 1.0 / 4294967296.0 / 65536.0 / 16.0 / 2.0)
                return 10;
              else
                return 11;
          else
            if (power < 1.0 / 4294967296.0 / 65536.0 / 4.0)
              if (power < 1.0 / 4294967296.0 / 65536.0 / 4.0 / 2.0)
                return 12;
              else
                return 13;
            else
              if (power < 1.0 / 4294967296.0 / 65536.0 / 2.0)
                return 14;
              else
                return 15;
      else
        if (power < 1.0 / 4294967296.0 / 256.0)
          if (power < 1.0 / 4294967296.0 / 256.0 / 16.0)
            if (power < 1.0 / 4294967296.0 / 256.0 / 16.0 / 4.0)
              if (power < 1.0 / 4294967296.0 / 256.0 / 16.0 / 4.0 / 2.0)
                return 16;
              else
                return 17;
            else
              if (power < 1.0 / 4294967296.0 / 256.0 / 16.0 / 2.0)
                return 18;
              else
                return 19;
          else
            if (power < 1.0 / 4294967296.0 / 256.0 / 4.0)
              if (power < 1.0 / 4294967296.0 / 256.0 / 4.0 / 2.0)
                return 20;
              else
                return 21;
            else
              if (power < 1.0 / 4294967296.0 / 256.0 / 2.0)
                return 22;
              else
                return 23;
        else
          if (power < 1.0 / 4294967296.0 / 16.0)
            if (power < 1.0 / 4294967296.0 / 16.0 / 4.0)
              if (power < 1.0 / 4294967296.0 / 16.0 / 4.0 / 2.0)
                return 24;
              else
                return 25;
            else
              if (power < 1.0 / 4294967296.0 / 16.0 / 2.0)
                return 26;
              else
                return 27;
          else
            if (power < 1.0 / 4294967296.0 / 4.0)
              if (power < 1.0 / 4294967296.0 / 4.0 / 2.0)
                return 28;
              else
                return 29;
            else
              if (power < 1.0 / 4294967296.0 / 2.0)
                return 30;
              else
                return 31;
    else
      if (power < 1.0 / 65536.0)
        if (power < 1.0 / 65536.0 / 256.0)
          if (power < 1.0 / 65536.0 / 256.0 / 16.0)
            if (power < 1.0 / 65536.0 / 256.0 / 16.0 / 4.0)
              if (power < 1.0 / 65536.0 / 256.0 / 16.0 / 4.0 / 2.0)
                return 32;
              else
                return 33;
            else
              if (power < 1.0 / 65536.0 / 256.0 / 16.0 / 2.0)
                return 34;
              else
                return 35;
          else
            if (power < 1.0 / 65536.0 / 256.0 / 4.0)
              if (power < 1.0 / 65536.0 / 256.0 / 4.0 / 2.0)
                return 36;
              else
                return 37;
            else
              if (power < 1.0 / 65536.0 / 256.0 / 2.0)
                return 38;
              else
                return 39;
        else
          if (power < 1.0 / 65536.0 / 16.0)
            if (power < 1.0 / 65536.0 / 16.0 / 4.0)
              if (power < 1.0 / 65536.0 / 16.0 / 4.0 / 2.0)
                return 40;
              else
                return 41;
            else
              if (power < 1.0 / 65536.0 / 16.0 / 2.0)
                return 42;
              else
                return 43;
          else
            if (power < 1.0 / 65536.0 / 4.0)
              if (power < 1.0 / 65536.0 / 4.0 / 2.0)
                return 44;
              else
                return 45;
            else
              if (power < 1.0 / 65536.0 / 2.0)
                return 46;
              else
                return 47;
      else
        if (power < 1.0 / 256.0)
          if (power < 1.0 / 256.0 / 16.0)
            if (power < 1.0 / 256.0 / 16.0 / 4.0)
              if (power < 1.0 / 256.0 / 16.0 / 4.0 / 2.0)
                return 48;
              else
                return 49;
            else
              if (power < 1.0 / 256.0 / 16.0 / 2.0)
                return 50;
              else
                return 51;
          else
            if (power < 1.0 / 256.0 / 4.0)
              if (power < 1.0 / 256.0 / 4.0 / 2.0)
                return 52;
              else
                return 53;
            else
              if (power < 1.0 / 256.0 / 2.0)
                return 54;
              else
                return 55;
        else
          if (power < 1.0 / 16.0)
            if (power < 1.0 / 16.0 / 4.0)
              if (power < 1.0 / 16.0 / 4.0 / 2.0)
                return 56;
              else
                return 57;
            else
              if (power < 1.0 / 16.0 / 2.0)
                return 58;
              else
                return 59;
          else
            if (power < 1.0 / 4.0)
              if (power < 1.0 / 4.0 / 2.0)
                return 60;
              else
                return 61;
            else
              if (power < 1.0 / 2.0)
                return 62;
              else
                return 63;
  else
    if (power > 4294967296.0)
      if (power > 4294967296.0 * 65536.0)
        if (power > 4294967296.0 * 65536.0 * 256.0)
          if (power > 4294967296.0 * 65536.0 * 256.0 * 16.0)
            if (power > 4294967296.0 * 65536.0 * 256.0 * 16.0 * 4.0)
              if (power > 4294967296.0 * 65536.0 * 256.0 * 16.0 * 4.0 * 2.0)
                return 127;
              else
                return 126;
            else
              if (power > 4294967296.0 * 65536.0 * 256.0 * 16.0 * 2.0)
                return 125;
              else
                return 124;
          else
            if (power > 4294967296.0 * 65536.0 * 256.0 * 4.0)
              if (power > 4294967296.0 * 65536.0 * 256.0 * 4.0 * 2.0)
                return 123;
              else
                return 122;
            else
              if (power > 4294967296.0 * 65536.0 * 256.0 * 2.0)
                return 121;
              else
                return 120;
        else
          if (power > 4294967296.0 * 65536.0 * 16.0)
            if (power > 4294967296.0 * 65536.0 * 16.0 * 4.0)
              if (power > 4294967296.0 * 65536.0 * 16.0 * 4.0 * 2.0)
                return 119;
              else
                return 118;
            else
              if (power > 4294967296.0 * 65536.0 * 16.0 * 2.0)
                return 117;
              else
                return 116;
          else
            if (power > 4294967296.0 * 65536.0 * 4.0)
              if (power > 4294967296.0 * 65536.0 * 4.0 * 2.0)
                return 115;
              else
                return 114;
            else
              if (power > 4294967296.0 * 65536.0 * 2.0)
                return 113;
              else
                return 112;
      else
        if (power > 4294967296.0 * 256.0)
          if (power > 4294967296.0 * 256.0 * 16.0)
            if (power > 4294967296.0 * 256.0 * 16.0 * 4.0)
              if (power > 4294967296.0 * 256.0 * 16.0 * 4.0 * 2.0)
                return 111;
              else
                return 110;
            else
              if (power > 4294967296.0 * 256.0 * 16.0 * 2.0)
                return 109;
              else
                return 108;
          else
            if (power > 4294967296.0 * 256.0 * 4.0)
              if (power > 4294967296.0 * 256.0 * 4.0 * 2.0)
                return 107;
              else
                return 106;
            else
              if (power > 4294967296.0 * 256.0 * 2.0)
                return 105;
              else
                return 104;
        else
          if (power > 4294967296.0 * 16.0)
            if (power > 4294967296.0 * 16.0 * 4.0)
              if (power > 4294967296.0 * 16.0 * 4.0 * 2.0)
                return 103;
              else
                return 102;
            else
              if (power > 4294967296.0 * 16.0 * 2.0)
                return 101;
              else
                return 100;
          else
            if (power > 4294967296.0 * 4.0)
              if (power > 4294967296.0 * 4.0 * 2.0)
                return 99;
              else
                return 98;
            else
              if (power > 4294967296.0 * 2.0)
                return 97;
              else
                return 96;
    else
      if (power > 65536.0)
        if (power > 65536.0 * 256.0)
          if (power > 65536.0 * 256.0 * 16.0)
            if (power > 65536.0 * 256.0 * 16.0 * 4.0)
              if (power > 65536.0 * 256.0 * 16.0 * 4.0 * 2.0)
                return 95;
              else
                return 94;
            else
              if (power > 65536.0 * 256.0 * 16.0 * 2.0)
                return 93;
              else
                return 92;
          else
            if (power > 65536.0 * 256.0 * 4.0)
              if (power > 65536.0 * 256.0 * 4.0 * 2.0)
                return 91;
              else
                return 90;
            else
              if (power > 65536.0 * 256.0 * 2.0)
                return 89;
              else
                return 88;
        else
          if (power > 65536.0 * 16.0)
            if (power > 65536.0 * 16.0 * 4.0)
              if (power > 65536.0 * 16.0 * 4.0 * 2.0)
                return 87;
              else
                return 86;
            else
              if (power > 65536.0 * 16.0 * 2.0)
                return 85;
              else
                return 84;
          else
            if (power > 65536.0 * 4.0)
              if (power > 65536.0 * 4.0 * 2.0)
                return 83;
              else
                return 82;
            else
              if (power > 65536.0 * 2.0)
                return 81;
              else
                return 80;
      else
        if (power > 256.0)
          if (power > 256.0 * 16.0)
            if (power > 256.0 * 16.0 * 4.0)
              if (power > 256.0 * 16.0 * 4.0 * 2.0)
                return 79;
              else
                return 78;
            else
              if (power > 256.0 * 16.0 * 2.0)
                return 77;
              else
                return 76;
          else
            if (power > 256.0 * 4.0)
              if (power > 256.0 * 4.0 * 2.0)
                return 75;
              else
                return 74;
            else
              if (power > 256.0 * 2.0)
                return 73;
              else
                return 72;
        else
          if (power > 16.0)
            if (power > 16.0 * 4.0)
              if (power > 16.0 * 4.0 * 2.0)
                return 71;
              else
                return 70;
            else
              if (power > 16.0 * 2.0)
                return 69;
              else
                return 68;
          else
            if (power > 4.0)
              if (power > 4.0 * 2.0)
                return 67;
              else
                return 66;
            else
              if (power > 2.0)
                return 65;
              else
                return 64;
}

void MathCache_Hirange::get_lower_index_and_percent(const float power, int& index, float& percent)
{
  if (power < 1.0)
    if (power < 1.0 / 4294967296.0)
      if (power < 1.0 / 4294967296.0 / 65536.0)
        if (power < 1.0 / 4294967296.0 / 65536.0 / 256.0)
          if (power < 1.0 / 4294967296.0 / 65536.0 / 256.0 / 16.0)
            if (power < 1.0 / 4294967296.0 / 65536.0 / 256.0 / 16.0 / 4.0)
              if (power < 1.0 / 4294967296.0 / 65536.0 / 256.0 / 16.0 / 4.0 / 2.0)
              {
                const float upper = 1.0 / 4294967296.0 / 65536.0 / 256.0 / 16.0 / 4.0 / 2.0;;
                percent = power / upper;
                index = 0;
              }
              else
              {
                const float lower = 1.0 / 4294967296.0 / 65536.0 / 256.0 / 16.0 / 4.0 / 2.0;
                percent = (power - lower) / lower;
                index = 1;
              }
            else
              if (power < 1.0 / 4294967296.0 / 65536.0 / 256.0 / 16.0 / 2.0)
              {
                const float lower = 1.0 / 4294967296.0 / 65536.0 / 256.0 / 16.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 2;
              }
              else
              {
                const float lower = 1.0 / 4294967296.0 / 65536.0 / 256.0 / 16.0 / 2.0;
                percent = (power - lower) / lower;
                index = 3;
              }
          else
            if (power < 1.0 / 4294967296.0 / 65536.0 / 256.0 / 4.0)
              if (power < 1.0 / 4294967296.0 / 65536.0 / 256.0 / 4.0 / 2.0)
              {
                const float lower = 1.0 / 4294967296.0 / 65536.0 / 256.0 / 4.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 4;
              }
              else
              {
                const float lower = 1.0 / 4294967296.0 / 65536.0 / 256.0 / 4.0 / 2.0;
                percent = (power - lower) / lower;
                index = 5;
              }
            else
              if (power < 1.0 / 4294967296.0 / 65536.0 / 256.0 / 2.0)
              {
                const float lower = 1.0 / 4294967296.0 / 65536.0 / 256.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 6;
              }
              else
              {
                const float lower = 1.0 / 4294967296.0 / 65536.0 / 256.0 / 2.0;
                percent = (power - lower) / lower;
                index = 7;
              }
        else
          if (power < 1.0 / 4294967296.0 / 65536.0 / 16.0)
            if (power < 1.0 / 4294967296.0 / 65536.0 / 16.0 / 4.0)
              if (power < 1.0 / 4294967296.0 / 65536.0 / 16.0 / 4.0 / 2.0)
              {
                const float lower = 1.0 / 4294967296.0 / 65536.0 / 16.0 / 4.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 8;
              }
              else
              {
                const float lower = 1.0 / 4294967296.0 / 65536.0 / 16.0 / 4.0 / 2.0;
                percent = (power - lower) / lower;
                index = 9;
              }
            else
              if (power < 1.0 / 4294967296.0 / 65536.0 / 16.0 / 2.0)
              {
                const float lower = 1.0 / 4294967296.0 / 65536.0 / 16.0 / 4.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 10;
              }
              else
              {
                const float lower = 1.0 / 4294967296.0 / 65536.0 / 16.0 / 2.0;
                percent = (power - lower) / lower;
                index = 11;
              }
          else
            if (power < 1.0 / 4294967296.0 / 65536.0 / 4.0)
              if (power < 1.0 / 4294967296.0 / 65536.0 / 4.0 / 2.0)
              {
                const float lower = 1.0 / 4294967296.0 / 65536.0 / 4.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 12;
              }
              else
              {
                const float lower = 1.0 / 4294967296.0 / 65536.0 / 4.0 / 2.0;
                percent = (power - lower) / lower;
                index = 13;
              }
            else
              if (power < 1.0 / 4294967296.0 / 65536.0 / 2.0)
              {
                const float lower = 1.0 / 4294967296.0 / 65536.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 14;
              }
              else
              {
                const float lower = 1.0 / 4294967296.0 / 65536.0 / 2.0;
                percent = (power - lower) / lower;
                index = 15;
              }
      else
        if (power < 1.0 / 4294967296.0 / 256.0)
          if (power < 1.0 / 4294967296.0 / 256.0 / 16.0)
            if (power < 1.0 / 4294967296.0 / 256.0 / 16.0 / 4.0)
              if (power < 1.0 / 4294967296.0 / 256.0 / 16.0 / 4.0 / 2.0)
              {
                const float lower = 1.0 / 4294967296.0 / 256.0 / 16.0 / 4.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 16;
              }
              else
              {
                const float lower = 1.0 / 4294967296.0 / 256.0 / 16.0 / 4.0 / 2.0;
                percent = (power - lower) / lower;
                index = 17;
              }
            else
              if (power < 1.0 / 4294967296.0 / 256.0 / 16.0 / 2.0)
              {
                const float lower = 1.0 / 4294967296.0 / 256.0 / 16.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 18;
              }
              else
              {
                const float lower = 1.0 / 4294967296.0 / 256.0 / 16.0 / 2.0;
                percent = (power - lower) / lower;
                index = 19;
              }
          else
            if (power < 1.0 / 4294967296.0 / 256.0 / 4.0)
              if (power < 1.0 / 4294967296.0 / 256.0 / 4.0 / 2.0)
              {
                const float lower = 1.0 / 4294967296.0 / 256.0 / 4.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 20;
              }
              else
              {
                const float lower = 1.0 / 4294967296.0 / 256.0 / 4.0 / 2.0;
                percent = (power - lower) / lower;
                index = 21;
              }
            else
              if (power < 1.0 / 4294967296.0 / 256.0 / 2.0)
              {
                const float lower = 1.0 / 4294967296.0 / 256.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 22;
              }
              else
              {
                const float lower = 1.0 / 4294967296.0 / 256.0 / 2.0;
                percent = (power - lower) / lower;
                index = 23;
              }
        else
          if (power < 1.0 / 4294967296.0 / 16.0)
            if (power < 1.0 / 4294967296.0 / 16.0 / 4.0)
              if (power < 1.0 / 4294967296.0 / 16.0 / 4.0 / 2.0)
              {
                const float lower = 1.0 / 4294967296.0 / 16.0 / 4.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 24;
              }
              else
              {
                const float lower = 1.0 / 4294967296.0 / 16.0 / 4.0 / 2.0;
                percent = (power - lower) / lower;
                index = 25;
              }
            else
              if (power < 1.0 / 4294967296.0 / 16.0 / 2.0)
              {
                const float lower = 1.0 / 4294967296.0 / 16.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 26;
              }
              else
              {
                const float lower = 1.0 / 4294967296.0 / 16.0 / 2.0;
                percent = (power - lower) / lower;
                index = 27;
              }
          else
            if (power < 1.0 / 4294967296.0 / 4.0)
              if (power < 1.0 / 4294967296.0 / 4.0 / 2.0)
              {
                const float lower = 1.0 / 4294967296.0 / 4.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 28;
              }
              else
              {
                const float lower = 1.0 / 4294967296.0 / 4.0 / 2.0;
                percent = (power - lower) / lower;
                index = 29;
              }
            else
              if (power < 1.0 / 4294967296.0 / 2.0)
              {
                const float lower = 1.0 / 4294967296.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 30;
              }
              else
              {
                const float lower = 1.0 / 4294967296.0 / 2.0;
                percent = (power - lower) / lower;
                index = 31;
              }
    else
      if (power < 1.0 / 65536.0)
        if (power < 1.0 / 65536.0 / 256.0)
          if (power < 1.0 / 65536.0 / 256.0 / 16.0)
            if (power < 1.0 / 65536.0 / 256.0 / 16.0 / 4.0)
              if (power < 1.0 / 65536.0 / 256.0 / 16.0 / 4.0 / 2.0)
              {
                const float lower = 1.0 / 65536.0 / 256.0 / 16.0 / 4.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 32;
              }
              else
              {
                const float lower = 1.0 / 65536.0 / 256.0 / 16.0 / 4.0 / 2.0;
                percent = (power - lower) / lower;
                index = 33;
              }
            else
              if (power < 1.0 / 65536.0 / 256.0 / 16.0 / 2.0)
              {
                const float lower = 1.0 / 65536.0 / 256.0 / 16.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 34;
              }
              else
              {
                const float lower = 1.0 / 65536.0 / 256.0 / 16.0 / 2.0;
                percent = (power - lower) / lower;
                index = 35;
              }
          else
            if (power < 1.0 / 65536.0 / 256.0 / 4.0)
              if (power < 1.0 / 65536.0 / 256.0 / 4.0 / 2.0)
              {
                const float lower = 1.0 / 65536.0 / 256.0 / 4.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 36;
              }
              else
              {
                const float lower = 1.0 / 65536.0 / 256.0 / 4.0 / 2.0;
                percent = (power - lower) / lower;
                index = 37;
              }
            else
              if (power < 1.0 / 65536.0 / 256.0 / 2.0)
              {
                const float lower = 1.0 / 65536.0 / 256.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 38;
              }
              else
              {
                const float lower = 1.0 / 65536.0 / 256.0 / 2.0;
                percent = (power - lower) / lower;
                index = 39;
              }
        else
          if (power < 1.0 / 65536.0 / 16.0)
            if (power < 1.0 / 65536.0 / 16.0 / 4.0)
              if (power < 1.0 / 65536.0 / 16.0 / 4.0 / 2.0)
              {
                const float lower = 1.0 / 65536.0 / 16.0 / 4.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 40;
              }
              else
              {
                const float lower = 1.0 / 65536.0 / 16.0 / 4.0 / 2.0;
                percent = (power - lower) / lower;
                index = 41;
              }
            else
              if (power < 1.0 / 65536.0 / 16.0 / 2.0)
              {
                const float lower = 1.0 / 65536.0 / 16.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 42;
              }
              else
              {
                const float lower = 1.0 / 65536.0 / 16.0 / 2.0;
                percent = (power - lower) / lower;
                index = 43;
              }
          else
            if (power < 1.0 / 65536.0 / 4.0)
              if (power < 1.0 / 65536.0 / 4.0 / 2.0)
              {
                const float lower = 1.0 / 65536.0 / 4.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 44;
              }
              else
              {
                const float lower = 1.0 / 65536.0 / 4.0 / 2.0;
                percent = (power - lower) / lower;
                index = 45;
              }
            else
              if (power < 1.0 / 65536.0 / 2.0)
              {
                const float lower = 1.0 / 65536.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 46;
              }
              else
              {
                const float lower = 1.0 / 65536.0 / 2.0;
                percent = (power - lower) / lower;
                index = 47;
              }
      else
        if (power < 1.0 / 256.0)
          if (power < 1.0 / 256.0 / 16.0)
            if (power < 1.0 / 256.0 / 16.0 / 4.0)
              if (power < 1.0 / 256.0 / 16.0 / 4.0 / 2.0)
              {
                const float lower = 1.0 / 256.0 / 16.0 / 4.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 48;
              }
              else
              {
                const float lower = 1.0 / 256.0 / 16.0 / 4.0 / 2.0;
                percent = (power - lower) / lower;
                index = 49;
              }
            else
              if (power < 1.0 / 256.0 / 16.0 / 2.0)
              {
                const float lower = 1.0 / 256.0 / 16.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 50;
              }
              else
              {
                const float lower = 1.0 / 256.0 / 16.0 / 2.0;
                percent = (power - lower) / lower;
                index = 51;
              }
          else
            if (power < 1.0 / 256.0 / 4.0)
              if (power < 1.0 / 256.0 / 4.0 / 2.0)
              {
                const float lower = 1.0 / 256.0 / 4.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 52;
              }
              else
              {
                const float lower = 1.0 / 256.0 / 4.0 / 2.0;
                percent = (power - lower) / lower;
                index = 53;
              }
            else
              if (power < 1.0 / 256.0 / 2.0)
              {
                const float lower = 1.0 / 256.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 54;
              }
              else
              {
                const float lower = 1.0 / 256.0 / 2.0;
                percent = (power - lower) / lower;
                index = 55;
              }
        else
          if (power < 1.0 / 16.0)
            if (power < 1.0 / 16.0 / 4.0)
              if (power < 1.0 / 16.0 / 4.0 / 2.0)
              {
                const float lower = 1.0 / 16.0 / 4.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 56;
              }
              else
              {
                const float lower = 1.0 / 16.0 / 4.0 / 2.0;
                percent = (power - lower) / lower;
                index = 57;
              }
            else
              if (power < 1.0 / 16.0 / 2.0)
              {
                const float lower = 1.0 / 16.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 58;
              }
              else
              {
                const float lower = 1.0 / 16.0 / 2.0;
                percent = (power - lower) / lower;
                index = 59;
              }
          else
            if (power < 1.0 / 4.0)
              if (power < 1.0 / 4.0 / 2.0)
              {
                const float lower = 1.0 / 4.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 60;
              }
              else
              {
                const float lower = 1.0 / 4.0 / 2.0;
                percent = (power - lower) / lower;
                index = 61;
              }
            else
              if (power < 1.0 / 2.0)
              {
                const float lower = 1.0 / 2.0 / 2.0;
                percent = (power - lower) / lower;
                index = 62;
              }
              else
              {
                const float lower = 1.0 / 2.0;
                percent = (power - lower) / lower;
                index = 63;
              }
  else
    if (power > 4294967296.0)
      if (power > 4294967296.0 * 65536.0)
        if (power > 4294967296.0 * 65536.0 * 256.0)
          if (power > 4294967296.0 * 65536.0 * 256.0 * 16.0)
            if (power > 4294967296.0 * 65536.0 * 256.0 * 16.0 * 4.0)
              if (power > 4294967296.0 * 65536.0 * 256.0 * 16.0 * 4.0 * 2.0)
              {
                const float lower = 4294967296.0 * 65536.0 * 256.0 * 16.0 * 4.0 * 2.0;
                percent = (power - lower) / lower;
                if (percent > 1.0)
                  percent = 1.0;
                index = 127;
              }
              else
              {
                const float lower = 4294967296.0 * 65536.0 * 256.0 * 16.0 * 4.0;
                percent = (power - lower) / lower;
                index = 126;
              }
            else
              if (power > 4294967296.0 * 65536.0 * 256.0 * 16.0 * 2.0)
              {
                const float lower = 4294967296.0 * 65536.0 * 256.0 * 16.0 * 2.0;
                percent = (power - lower) / lower;
                index = 125;
              }
              else
              {
                const float lower = 4294967296.0 * 65536.0 * 256.0 * 16.0;
                percent = (power - lower) / lower;
                index = 124;
              }
          else
            if (power > 4294967296.0 * 65536.0 * 256.0 * 4.0)
              if (power > 4294967296.0 * 65536.0 * 256.0 * 4.0 * 2.0)
              {
                const float lower = 4294967296.0 * 65536.0 * 256.0 * 4.0 * 2.0;
                percent = (power - lower) / lower;
                index = 123;
              }
              else
              {
                const float lower = 4294967296.0 * 65536.0 * 256.0 * 4.0;
                percent = (power - lower) / lower;
                index = 122;
              }
            else
              if (power > 4294967296.0 * 65536.0 * 256.0 * 2.0)
              {
                const float lower = 4294967296.0 * 65536.0 * 256.0 * 2.0;
                percent = (power - lower) / lower;
                index = 121;
              }
              else
              {
                const float lower = 4294967296.0 * 65536.0 * 256.0;
                percent = (power - lower) / lower;
                index = 120;
              }
        else
          if (power > 4294967296.0 * 65536.0 * 16.0)
            if (power > 4294967296.0 * 65536.0 * 16.0 * 4.0)
              if (power > 4294967296.0 * 65536.0 * 16.0 * 4.0 * 2.0)
              {
                const float lower = 4294967296.0 * 65536.0 * 16.0 * 4.0 * 2.0;
                percent = (power - lower) / lower;
                index = 119;
              }
              else
              {
                const float lower = 4294967296.0 * 65536.0 * 16.0 * 4.0;
                percent = (power - lower) / lower;
                index = 118;
              }
            else
              if (power > 4294967296.0 * 65536.0 * 16.0 * 2.0)
              {
                const float lower = 4294967296.0 * 65536.0 * 16.0 * 2.0;
                percent = (power - lower) / lower;
                index = 117;
              }
              else
              {
                const float lower = 4294967296.0 * 65536.0 * 16.0;
                percent = (power - lower) / lower;
                index = 116;
              }
          else
            if (power > 4294967296.0 * 65536.0 * 4.0)
              if (power > 4294967296.0 * 65536.0 * 4.0 * 2.0)
              {
                const float lower = 4294967296.0 * 65536.0 * 4.0 * 2.0;
                percent = (power - lower) / lower;
                index = 115;
              }
              else
              {
                const float lower = 4294967296.0 * 65536.0 * 4.0;
                percent = (power - lower) / lower;
                index = 114;
              }
            else
              if (power > 4294967296.0 * 65536.0 * 2.0)
              {
                const float lower = 4294967296.0 * 65536.0 * 2.0;
                percent = (power - lower) / lower;
                index = 113;
              }
              else
              {
                const float lower = 4294967296.0 * 65536.0;
                percent = (power - lower) / lower;
                index = 112;
              }
      else
        if (power > 4294967296.0 * 256.0)
          if (power > 4294967296.0 * 256.0 * 16.0)
            if (power > 4294967296.0 * 256.0 * 16.0 * 4.0)
              if (power > 4294967296.0 * 256.0 * 16.0 * 4.0 * 2.0)
              {
                const float lower = 4294967296.0 * 256.0 * 16.0 * 4.0 * 2.0;
                percent = (power - lower) / lower;
                index = 111;
              }
              else
              {
                const float lower = 4294967296.0 * 256.0 * 16.0 * 4.0;
                percent = (power - lower) / lower;
                index = 110;
              }
            else
              if (power > 4294967296.0 * 256.0 * 16.0 * 2.0)
              {
                const float lower = 4294967296.0 * 256.0 * 16.0 * 2.0;
                percent = (power - lower) / lower;
                index = 109;
              }
              else
              {
                const float lower = 4294967296.0 * 256.0 * 16.0;
                percent = (power - lower) / lower;
                index = 108;
              }
          else
            if (power > 4294967296.0 * 256.0 * 4.0)
              if (power > 4294967296.0 * 256.0 * 4.0 * 2.0)
              {
                const float lower = 4294967296.0 * 256.0 * 4.0 * 2.0;
                percent = (power - lower) / lower;
                index = 107;
              }
              else
              {
                const float lower = 4294967296.0 * 256.0 * 4.0;
                percent = (power - lower) / lower;
                index = 106;
              }
            else
              if (power > 4294967296.0 * 256.0 * 2.0)
              {
                const float lower = 4294967296.0 * 256.0 * 2.0;
                percent = (power - lower) / lower;
                index = 105;
              }
              else
              {
                const float lower = 4294967296.0 * 256.0;
                percent = (power - lower) / lower;
                index = 104;
              }
        else
          if (power > 4294967296.0 * 16.0)
            if (power > 4294967296.0 * 16.0 * 4.0)
              if (power > 4294967296.0 * 16.0 * 4.0 * 2.0)
              {
                const float lower = 4294967296.0 * 16.0 * 4.0 * 2.0;
                percent = (power - lower) / lower;
                index = 103;
              }
              else
              {
                const float lower = 4294967296.0 * 16.0 * 4.0;
                percent = (power - lower) / lower;
                index = 102;
              }
            else
              if (power > 4294967296.0 * 16.0 * 2.0)
              {
                const float lower = 4294967296.0 * 16.0 * 2.0;
                percent = (power - lower) / lower;
                index = 101;
              }
              else
              {
                const float lower = 4294967296.0 * 16.0;
                percent = (power - lower) / lower;
                index = 100;
              }
          else
            if (power > 4294967296.0 * 4.0)
              if (power > 4294967296.0 * 4.0 * 2.0)
              {
                const float lower = 4294967296.0 * 4.0 * 2.0;
                percent = (power - lower) / lower;
                index = 99;
              }
              else
              {
                const float lower = 4294967296.0 * 4.0;
                percent = (power - lower) / lower;
                index = 98;
              }
            else
              if (power > 4294967296.0 * 2.0)
              {
                const float lower = 4294967296.0 * 2.0;
                percent = (power - lower) / lower;
                index = 97;
              }
              else
              {
                const float lower = 4294967296.0;
                percent = (power - lower) / lower;
                index = 96;
              }
    else
      if (power > 65536.0)
        if (power > 65536.0 * 256.0)
          if (power > 65536.0 * 256.0 * 16.0)
            if (power > 65536.0 * 256.0 * 16.0 * 4.0)
              if (power > 65536.0 * 256.0 * 16.0 * 4.0 * 2.0)
              {
                const float lower = 65536.0 * 256.0 * 16.0 * 4.0 * 2.0;
                percent = (power - lower) / lower;
                index = 95;
              }
              else
              {
                const float lower = 65536.0 * 256.0 * 16.0 * 4.0;
                percent = (power - lower) / lower;
                index = 94;
              }
            else
              if (power > 65536.0 * 256.0 * 16.0 * 2.0)
              {
                const float lower = 65536.0 * 256.0 * 16.0 * 2.0;
                percent = (power - lower) / lower;
                index = 93;
              }
              else
              {
                const float lower = 65536.0 * 256.0 * 16.0;
                percent = (power - lower) / lower;
                index = 92;
              }
          else
            if (power > 65536.0 * 256.0 * 4.0)
              if (power > 65536.0 * 256.0 * 4.0 * 2.0)
              {
                const float lower = 65536.0 * 256.0 * 4.0 * 2.0;
                percent = (power - lower) / lower;
                index = 91;
              }
              else
              {
                const float lower = 65536.0 * 256.0 * 4.0;
                percent = (power - lower) / lower;
                index = 90;
              }
            else
              if (power > 65536.0 * 256.0 * 2.0)
              {
                const float lower = 65536.0 * 256.0 * 2.0;
                percent = (power - lower) / lower;
                index = 89;
              }
              else
              {
                const float lower = 65536.0 * 256.0;
                percent = (power - lower) / lower;
                index = 88;
              }
        else
          if (power > 65536.0 * 16.0)
            if (power > 65536.0 * 16.0 * 4.0)
              if (power > 65536.0 * 16.0 * 4.0 * 2.0)
              {
                const float lower = 65536.0 * 16.0 * 4.0 * 2.0;
                percent = (power - lower) / lower;
                index = 87;
              }
              else
              {
                const float lower = 65536.0 * 16.0 * 4.0;
                percent = (power - lower) / lower;
                index = 86;
              }
            else
              if (power > 65536.0 * 16.0 * 2.0)
              {
                const float lower = 65536.0 * 16.0 * 2.0;
                percent = (power - lower) / lower;
                index = 85;
              }
              else
              {
                const float lower = 65536.0 * 16.0;
                percent = (power - lower) / lower;
                index = 84;
              }
          else
            if (power > 65536.0 * 4.0)
              if (power > 65536.0 * 4.0 * 2.0)
              {
                const float lower = 65536.0 * 4.0 * 2.0;
                percent = (power - lower) / lower;
                index = 83;
              }
              else
              {
                const float lower = 65536.0 * 4.0;
                percent = (power - lower) / lower;
                index = 82;
              }
            else
              if (power > 65536.0 * 2.0)
              {
                const float lower = 65536.0 * 2.0;
                percent = (power - lower) / lower;
                index = 81;
              }
              else
              {
                const float lower = 65536.0;
                percent = (power - lower) / lower;
                index = 80;
              }
      else
        if (power > 256.0)
          if (power > 256.0 * 16.0)
            if (power > 256.0 * 16.0 * 4.0)
              if (power > 256.0 * 16.0 * 4.0 * 2.0)
              {
                const float lower = 256.0 * 16.0 * 4.0 * 2.0;
                percent = (power - lower) / lower;
                index = 79;
              }
              else
              {
                const float lower = 256.0 * 16.0 * 4.0;
                percent = (power - lower) / lower;
                index = 78;
              }
            else
              if (power > 256.0 * 16.0 * 2.0)
              {
                const float lower = 256.0 * 16.0 * 2.0;
                percent = (power - lower) / lower;
                index = 77;
              }
              else
              {
                const float lower = 256.0 * 16.0;
                percent = (power - lower) / lower;
                index = 76;
              }
          else
            if (power > 256.0 * 4.0)
              if (power > 256.0 * 4.0 * 2.0)
              {
                const float lower = 256.0 * 4.0 * 2.0;
                percent = (power - lower) / lower;
                index = 75;
              }
              else
              {
                const float lower = 256.0 * 4.0;
                percent = (power - lower) / lower;
                index = 74;
              }
            else
              if (power > 256.0 * 2.0)
              {
                const float lower = 256.0 * 2.0;
                percent = (power - lower) / lower;
                index = 73;
              }
              else
              {
                const float lower = 256.0;
                percent = (power - lower) / lower;
                index = 72;
              }
        else
          if (power > 16.0)
            if (power > 16.0 * 4.0)
              if (power > 16.0 * 4.0 * 2.0)
              {
                const float lower = 16.0 * 4.0 * 2.0;
                percent = (power - lower) / lower;
                index = 71;
              }
              else
              {
                const float lower = 16.0 * 4.0;
                percent = (power - lower) / lower;
                index = 70;
              }
            else
              if (power > 16.0 * 2.0)
              {
                const float lower = 16.0 * 2.0;
                percent = (power - lower) / lower;
                index = 69;
              }
              else
              {
                const float lower = 16.0;
                percent = (power - lower) / lower;
                index = 68;
              }
          else
            if (power > 4.0)
              if (power > 4.0 * 2.0)
              {
                const float lower = 4.0 * 2.0;
                percent = (power - lower) / lower;
                index = 67;
              }
              else
              {
                const float lower = 4.0;
                percent = (power - lower) / lower;
                index = 66;
              }
            else
              if (power > 2.0)
              {
                const float lower = 2.0;
                percent = (power - lower) / lower;
                index = 65;
              }
              else
              {
                const float lower = 1.0;
                percent = (power - lower) / lower;
                index = 64;
              }
}

float MathCache_Hirange::powf_05_rough(const float power) const
{
  const int index = get_lower_index(power);
  return powf_map[5000][index];
}

float MathCache_Hirange::powf_05_close(const float power) const
{
  float percent;
  int index;
  get_lower_index_and_percent(power, index, percent);
  return (powf_map[5000][index] * (1.0 - percent)) + (powf_map[5000][index + 1] * percent);
}

float MathCache_Hirange::powf_0_1_rough_rough(const float base, const float power) const
{
  const int index1 = (int)(base * 10000);
  const int index2 = get_lower_index(power);
  return powf_map[index1][index2];
}

float MathCache_Hirange::powf_0_1_rough_close(const float base, const float power) const
{
  const int index1 = (int)(base * 10000);
  float percent2;
  int index2;
  get_lower_index_and_percent(power, index2, percent2);
  return (powf_map[index1][index2] * (1.0 - percent2)) + (powf_map[index1][index2 + 1] * percent2);
}

float MathCache_Hirange::powf_0_1_close_rough(const float base, const float power) const
{
  const int index1 = (int)(base * 10000);
  const float percent1 = (base - (float)index1 / 10000) * 10000;
  const int index2 = get_lower_index(power);
  return (powf_map[index1][index2] * (1.0 - percent1)) + (powf_map[index1 + 1][index2] * percent1);
}

float MathCache_Hirange::powf_0_1_close_close(const float base, const float power) const
{
  const int index1 = (int)(base * 10000);
  const float percent1 = (base - (float)index1 / 10000) * 10000;
  float percent2;
  int index2;
  get_lower_index_and_percent(power, index2, percent2);
  return (powf_map[index1][index2] * (1.0 - percent1) * (1.0 - percent2)) + (powf_map[index1 + 1][index2] * percent1 * (1.0 - percent2)) + (powf_map[index1][index2 + 1] * (1.0 - percent1) * percent2) + (powf_map[index1 + 1][index2 + 1] * percent1 * percent2);
}

float MathCache_Hirange::sqrt_rough(const float value) const
{
  const int index = get_lower_index(value);
  return sqrt_map[index];
}

float MathCache_Hirange::sqrt_close(const float value) const
{
  float percent;
  int index;
  get_lower_index_and_percent(value, index, percent);
  return (sqrt_map[index] * (1.0 - percent)) + (sqrt_map[index + 1] * percent);
}

float MathCache_Hirange::sin_0_2PI_rough(const float angle) const
{
  return sin_map[(int)(angle * 10000 / (2 * PI))];
}

float MathCache_Hirange::sin_0_2PI_close(const float angle) const
{
  const int index = (int)(angle * 10000 / (2 * PI));
  const float percent = remainderf(angle * 10000 / (2 * PI), 1);
  return (sin_map[index] * (1.0 - percent)) + (sin_map[index + 1] * percent);
}

float MathCache_Hirange::sin_nPI_PI_rough(const float angle) const
{
  return sin_map2[(int)(angle * 10000 / (2 * PI)) + 5000];
}

float MathCache_Hirange::sin_nPI_PI_close(const float angle) const
{
  const int index = (int)(angle * 10000 / (2 * PI)) + 5000;
  const float percent = remainderf(angle * 10000 / (2 * PI), 1);
  return (sin_map2[index] * (1.0 - percent)) + (sin_map2[index + 1] * percent);
}

float MathCache_Hirange::sin_rough(const float angle) const
{
  return sin_map[((int)(angle * 10000 / (2 * PI))) % 10000 + (angle < 0 ? 10000 : 0)];
}

float MathCache_Hirange::sin_close(const float angle) const
{
  const int index = ((int)(angle * 10000 / (2 * PI))) % 10000 + (angle < 0 ? 10000 : 0);
  const float percent = remainderf(angle * 10000 / (2 * PI), 1)  + (angle < 0 ? 1 : 0);
  return (sin_map[index] * (1.0 - percent)) + (sin_map[index + 1] * percent);
}

float MathCache_Hirange::cos_0_2PI_rough(const float angle) const
{
  return cos_map[(int)(angle * 10000 / (2 * PI))];
}

float MathCache_Hirange::cos_0_2PI_close(const float angle) const
{
  const int index = (int)(angle * 10000 / (2 * PI));
  const float percent = remainderf(angle * 10000 / (2 * PI), 1);
  return (cos_map[index] * (1.0 - percent)) + (cos_map[index + 1] * percent);
}

float MathCache_Hirange::cos_nPI_PI_rough(const float angle) const
{
  return cos_map2[(int)(angle * 10000 / (2 * PI)) + 5000];
}

float MathCache_Hirange::cos_nPI_PI_close(const float angle) const
{
  const int index = (int)(angle * 10000 / (2 * PI)) + 5000;
  const float percent = remainderf(angle * 10000 / (2 * PI), 1);
  return (cos_map2[index] * (1.0 - percent)) + (cos_map2[index + 1] * percent);
}

float MathCache_Hirange::cos_rough(const float angle) const
{
  return cos_map[((int)(angle * 10000 / (2 * PI))) % 10000 + (angle < 0 ? 10000 : 0)];
}

float MathCache_Hirange::cos_close(const float angle) const
{
  const int index = ((int)(angle * 10000 / (2 * PI))) % 10000 + (angle < 0 ? 10000 : 0);
  const float percent = remainderf(angle * 10000 / (2 * PI), 1) + (angle < 0 ? 1 : 0);
  return (cos_map[index] * (1.0 - percent)) + (cos_map[index + 1] * percent);
}
*/

MathCache::MathCache()
{
  for (int i = 1; i < 10001; i++)
  {
    float base = (float)i / 10000;
    float power = 1.0;
    for (int j = 0; j < 31; j++)
      power /= 2.0;
    for (int j = 1; j < 65; j++)
    {
      powf_map[i][j] = powf(base, power);
      power *= 2.0;
    }
    base *= 2.0;
    powf_map[i][0] = 1.0;
  }
  for (int j = 1; j < 65; j++)
    powf_map[0][j] = pow(0.25 / 10000, j);
  powf_map[0][0] = 1;
}

// Sorry about the formatting on this one, but it's such a massive unrolled
// block that the brackets would make it unwieldy.
int MathCache::get_lower_index(const float power)
{
  if (power < 1.0)
    if (power < 1.0 / 65536.0)
      if (power < 1.0 / 65536.0 / 256.0)
        if (power < 1.0 / 65536.0 / 256.0 / 16.0)
          if (power < 1.0 / 65536.0 / 256.0 / 16.0 / 4.0)
            if (power < 1.0 / 65536.0 / 256.0 / 16.0 / 4.0 / 2.0)
              return 0;
            else
              return 1;
          else
            if (power < 1.0 / 65536.0 / 256.0 / 16.0 / 2.0)
              return 2;
            else
              return 3;
        else
          if (power < 1.0 / 65536.0 / 256.0 / 4.0)
            if (power < 1.0 / 65536.0 / 256.0 / 4.0 / 2.0)
              return 4;
            else
              return 5;
          else
            if (power < 1.0 / 65536.0 / 256.0 / 2.0)
              return 6;
            else
              return 7;
      else
        if (power < 1.0 / 65536.0 / 16.0)
          if (power < 1.0 / 65536.0 / 16.0 / 4.0)
            if (power < 1.0 / 65536.0 / 16.0 / 4.0 / 2.0)
              return 8;
            else
              return 9;
          else
            if (power < 1.0 / 65536.0 / 16.0 / 2.0)
              return 10;
            else
              return 11;
        else
          if (power < 1.0 / 65536.0 / 4.0)
            if (power < 1.0 / 65536.0 / 4.0 / 2.0)
              return 12;
            else
              return 13;
          else
            if (power < 1.0 / 65536.0 / 2.0)
              return 14;
            else
              return 15;
    else
      if (power < 1.0 / 256.0)
        if (power < 1.0 / 256.0 / 16.0)
          if (power < 1.0 / 256.0 / 16.0 / 4.0)
            if (power < 1.0 / 256.0 / 16.0 / 4.0 / 2.0)
              return 4;
            else
              return 17;
          else
            if (power < 1.0 / 256.0 / 16.0 / 2.0)
              return 18;
            else
              return 19;
        else
          if (power < 1.0 / 256.0 / 4.0)
            if (power < 1.0 / 256.0 / 4.0 / 2.0)
              return 20;
            else
              return 21;
          else
            if (power < 1.0 / 256.0 / 2.0)
              return 22;
            else
              return 23;
      else
        if (power < 1.0 / 16.0)
          if (power < 1.0 / 16.0 / 4.0)
            if (power < 1.0 / 16.0 / 4.0 / 2.0)
              return 24;
            else
              return 25;
          else
            if (power < 1.0 / 16.0 / 2.0)
              return 26;
            else
              return 27;
        else
          if (power < 1.0 / 4.0)
            if (power < 1.0 / 4.0 / 2.0)
              return 28;
            else
              return 29;
          else
            if (power < 1.0 / 2.0)
              return 30;
            else
              return 31;
  else
    if (power > 65536.0)
      if (power > 65536.0 * 256.0)
        if (power > 65536.0 * 256.0 * 16.0)
          if (power > 65536.0 * 256.0 * 16.0 * 4.0)
            if (power > 65536.0 * 256.0 * 16.0 * 4.0 * 2.0)
              return 63;
            else
              return 62;
          else
            if (power > 65536.0 * 256.0 * 16.0 * 2.0)
              return 61;
            else
              return 60;
        else
          if (power > 65536.0 * 256.0 * 4.0)
            if (power > 65536.0 * 256.0 * 4.0 * 2.0)
              return 59;
            else
              return 58;
          else
            if (power > 65536.0 * 256.0 * 2.0)
              return 57;
            else
              return 56;
      else
        if (power > 65536.0 * 16.0)
          if (power > 65536.0 * 16.0 * 4.0)
            if (power > 65536.0 * 16.0 * 4.0 * 2.0)
              return 55;
            else
              return 54;
          else
            if (power > 65536.0 * 16.0 * 2.0)
              return 53;
            else
              return 52;
        else
          if (power > 65536.0 * 4.0)
            if (power > 65536.0 * 4.0 * 2.0)
              return 51;
            else
              return 50;
          else
            if (power > 65536.0 * 2.0)
              return 49;
            else
              return 48;
    else
      if (power > 256.0)
        if (power > 256.0 * 16.0)
          if (power > 256.0 * 16.0 * 4.0)
            if (power > 256.0 * 16.0 * 4.0 * 2.0)
              return 47;
            else
              return 46;
          else
            if (power > 256.0 * 16.0 * 2.0)
              return 45;
            else
              return 44;
        else
          if (power > 256.0 * 4.0)
            if (power > 256.0 * 4.0 * 2.0)
              return 43;
            else
              return 42;
          else
            if (power > 256.0 * 2.0)
              return 41;
            else
              return 40;
      else
        if (power > 16.0)
          if (power > 16.0 * 4.0)
            if (power > 16.0 * 4.0 * 2.0)
              return 39;
            else
              return 38;
          else
            if (power > 16.0 * 2.0)
              return 37;
            else
              return 36;
        else
          if (power > 4.0)
            if (power > 4.0 * 2.0)
              return 35;
            else
              return 34;
          else
            if (power > 2.0)
              return 33;
            else
              return 32;
}

void MathCache::get_lower_index_and_percent(const float power, int& index, float& percent)
{
  if (power < 1.0)
    if (power < 1.0 / 65536.0)
      if (power < 1.0 / 65536.0 / 256.0)
        if (power < 1.0 / 65536.0 / 256.0 / 16.0)
          if (power < 1.0 / 65536.0 / 256.0 / 16.0 / 4.0)
            if (power < 1.0 / 65536.0 / 256.0 / 16.0 / 4.0 / 2.0)
            {
              const float upper = 1.0 / 65536.0 / 256.0 / 16.0 / 4.0 / 2.0;
              percent = power / upper;
              index = 0;
            }
            else
            {
              const float lower = 1.0 / 65536.0 / 256.0 / 16.0 / 4.0 / 2.0;
              percent = (power - lower) / lower;
              index = 1;
            }
          else
            if (power < 1.0 / 65536.0 / 256.0 / 16.0 / 2.0)
            {
              const float lower = 1.0 / 65536.0 / 256.0 / 16.0 / 2.0 / 2.0;
              percent = (power - lower) / lower;
              index = 2;
            }
            else
            {
              const float lower = 1.0 / 65536.0 / 256.0 / 16.0 / 2.0;
              percent = (power - lower) / lower;
              index = 3;
            }
        else
          if (power < 1.0 / 65536.0 / 256.0 / 4.0)
            if (power < 1.0 / 65536.0 / 256.0 / 4.0 / 2.0)
            {
              const float lower = 1.0 / 65536.0 / 256.0 / 4.0 / 2.0 / 2.0;
              percent = (power - lower) / lower;
              index = 4;
            }
            else
            {
              const float lower = 1.0 / 65536.0 / 256.0 / 4.0 / 2.0 / 2.0;
              percent = (power - lower) / lower;
              index = 5;
            }
          else
            if (power < 1.0 / 65536.0 / 256.0 / 2.0)
            {
              const float lower = 1.0 / 65536.0 / 256.0 / 2.0 / 2.0;
              percent = (power - lower) / lower;
              index = 6;
            }
            else
            {
              const float lower = 1.0 / 65536.0 / 256.0 / 2.0;
              percent = (power - lower) / lower;
              index = 7;
            }
      else
        if (power < 1.0 / 65536.0 / 16.0)
          if (power < 1.0 / 65536.0 / 16.0 / 4.0)
            if (power < 1.0 / 65536.0 / 16.0 / 4.0 / 2.0)
            {
              const float lower = 1.0 / 65536.0 / 16.0 / 4.0 / 2.0 / 2.0;
              percent = (power - lower) / lower;
              index = 8;
            }
            else
            {
              const float lower = 1.0 / 65536.0 / 16.0 / 4.0 / 2.0;
              percent = (power - lower) / lower;
              index = 9;
            }
          else
            if (power < 1.0 / 65536.0 / 16.0 / 2.0)
            {
              const float lower = 1.0 / 65536.0 / 16.0 / 2.0 / 2.0;
              percent = (power - lower) / lower;
              index = 10;
            }
            else
            {
              const float lower = 1.0 / 65536.0 / 16.0 / 2.0;
              percent = (power - lower) / lower;
              index = 11;
            }
        else
          if (power < 1.0 / 65536.0 / 4.0)
            if (power < 1.0 / 65536.0 / 4.0 / 2.0)
            {
              const float lower = 1.0 / 65536.0 / 4.0 / 2.0 / 2.0;
              percent = (power - lower) / lower;
              index = 12;
            }
            else
            {
              const float lower = 1.0 / 65536.0 / 4.0 / 2.0;
              percent = (power - lower) / lower;
              index = 13;
            }
          else
            if (power < 1.0 / 65536.0 / 2.0)
            {
              const float lower = 1.0 / 65536.0 / 2.0 / 2.0;
              percent = (power - lower) / lower;
              index = 14;
            }
            else
            {
              const float lower = 1.0 / 65536.0 / 2.0;
              percent = (power - lower) / lower;
              index = 15;
            }
    else
      if (power < 1.0 / 256.0)
        if (power < 1.0 / 256.0 / 16.0)
          if (power < 1.0 / 256.0 / 16.0 / 4.0)
            if (power < 1.0 / 256.0 / 16.0 / 4.0 / 2.0)
            {
              const float lower = 1.0 / 256.0 / 16.0 / 4.0 / 2.0 / 2.0;
              percent = (power - lower) / lower;
              index = 16;
            }
            else
            {
              const float lower = 1.0 / 256.0 / 16.0 / 4.0 / 2.0;
              percent = (power - lower) / lower;
              index = 17;
            }
          else
            if (power < 1.0 / 256.0 / 16.0 / 2.0)
            {
              const float lower = 1.0 / 256.0 / 16.0 / 2.0 / 2.0;
              percent = (power - lower) / lower;
              index = 18;
            }
            else
            {
              const float lower = 1.0 / 256.0 / 16.0 / 2.0;
              percent = (power - lower) / lower;
              index = 19;
            }
        else
          if (power < 1.0 / 256.0 / 4.0)
            if (power < 1.0 / 256.0 / 4.0 / 2.0)
            {
              const float lower = 1.0 / 256.0 / 4.0 / 2.0 / 2.0;
              percent = (power - lower) / lower;
              index = 20;
            }
            else
            {
              const float lower = 1.0 / 256.0 / 4.0 / 2.0;
              percent = (power - lower) / lower;
              index = 21;
            }
          else
            if (power < 1.0 / 256.0 / 2.0)
            {
              const float lower = 1.0 / 256.0 / 2.0 / 2.0;
              percent = (power - lower) / lower;
              index = 22;
            }
            else
            {
              const float lower = 1.0 / 256.0 / 2.0;
              percent = (power - lower) / lower;
              index = 23;
            }
      else
        if (power < 1.0 / 16.0)
          if (power < 1.0 / 16.0 / 4.0)
            if (power < 1.0 / 16.0 / 4.0 / 2.0)
            {
              const float lower = 1.0 / 16.0 / 4.0 / 2.0 / 2.0;
              percent = (power - lower) / lower;
              index = 24;
            }
            else
            {
              const float lower = 1.0 / 16.0 / 4.0 / 2.0;
              percent = (power - lower) / lower;
              index = 25;
            }
          else
            if (power < 1.0 / 16.0 / 2.0)
            {
              const float lower = 1.0 / 16.0 / 2.0 / 2.0;
              percent = (power - lower) / lower;
              index = 26;
            }
            else
            {
              const float lower = 1.0 / 16.0 / 2.0;
              percent = (power - lower) / lower;
              index = 27;
            }
        else
          if (power < 1.0 / 4.0)
            if (power < 1.0 / 4.0 / 2.0)
            {
              const float lower = 1.0 / 4.0 / 2.0 / 2.0;
              percent = (power - lower) / lower;
              index = 28;
            }
            else
            {
              const float lower = 1.0 / 4.0 / 2.0;
              percent = (power - lower) / lower;
              index = 29;
            }
          else
            if (power < 1.0 / 2.0)
            {
              const float lower = 1.0 / 2.0 / 2.0;
              percent = (power - lower) / lower;
              index = 30;
            }
            else
            {
              const float lower = 1.0 / 2.0;
              percent = (power - lower) / lower;
              index = 31;
            }
  else
    if (power > 65536.0)
      if (power > 65536.0 * 256.0)
        if (power > 65536.0 * 256.0 * 16.0)
          if (power > 65536.0 * 256.0 * 16.0 * 4.0)
            if (power > 65536.0 * 256.0 * 16.0 * 4.0 * 2.0)
            {
              const float lower = 65536.0 * 256.0 * 16.0 * 4.0 * 2.0;
              percent = (power - lower) / lower;
              if (percent > 1.0)
                percent = 1.0;
              index = 63;
            }
            else
            {
              const float lower = 65536.0 * 256.0 * 16.0 * 4.0;
              percent = (power - lower) / lower;
              index = 62;
            }
          else
            if (power > 65536.0 * 256.0 * 16.0 * 2.0)
            {
              const float lower = 65536.0 * 256.0 * 16.0 * 2.0;
              percent = (power - lower) / lower;
              index = 61;
            }
            else
            {
              const float lower = 65536.0 * 256.0 * 16.0;
              percent = (power - lower) / lower;
              index = 60;
            }
        else
          if (power > 65536.0 * 256.0 * 4.0)
            if (power > 65536.0 * 256.0 * 4.0 * 2.0)
            {
              const float lower = 65536.0 * 256.0 * 4.0 * 2.0;
              percent = (power - lower) / lower;
              index = 59;
            }
            else
            {
              const float lower = 65536.0 * 256.0 * 4.0;
              percent = (power - lower) / lower;
              index = 58;
            }
          else
            if (power > 65536.0 * 256.0 * 2.0)
            {
              const float lower = 65536.0 * 256.0 * 2.0;
              percent = (power - lower) / lower;
              index = 57;
            }
            else
            {
              const float lower = 65536.0 * 256.0;
              percent = (power - lower) / lower;
              index = 56;
            }
      else
        if (power > 65536.0 * 16.0)
          if (power > 65536.0 * 16.0 * 4.0)
            if (power > 65536.0 * 16.0 * 4.0 * 2.0)
            {
              const float lower = 65536.0 * 16.0 * 4.0 * 2.0;
              percent = (power - lower) / lower;
              index = 55;
            }
            else
            {
              const float lower = 65536.0 * 16.0 * 4.0;
              percent = (power - lower) / lower;
              index = 54;
            }
          else
            if (power > 65536.0 * 16.0 * 2.0)
            {
              const float lower = 65536.0 * 16.0 * 2.0;
              percent = (power - lower) / lower;
              index = 53;
            }
            else
            {
              const float lower = 65536.0 * 16.0;
              percent = (power - lower) / lower;
              index = 52;
            }
        else
          if (power > 65536.0 * 4.0)
            if (power > 65536.0 * 4.0 * 2.0)
            {
              const float lower = 65536.0 * 4.0 * 2.0;
              percent = (power - lower) / lower;
              index = 51;
            }
            else
            {
              const float lower = 65536.0 * 4.0;
              percent = (power - lower) / lower;
              index = 50;
            }
          else
            if (power > 65536.0 * 2.0)
            {
              const float lower = 65536.0 * 2.0;
              percent = (power - lower) / lower;
              index = 49;
            }
            else
            {
              const float lower = 65536.0;
              percent = (power - lower) / lower;
              index = 48;
            }
    else
      if (power > 256.0)
        if (power > 256.0 * 16.0)
          if (power > 256.0 * 16.0 * 4.0)
            if (power > 256.0 * 16.0 * 4.0 * 2.0)
            {
              const float lower = 256.0 * 16.0 * 4.0 * 2.0;
              percent = (power - lower) / lower;
              index = 47;
            }
            else
            {
              const float lower = 256.0 * 16.0 * 4.0;
              percent = (power - lower) / lower;
              index = 46;
            }
          else
            if (power > 256.0 * 16.0 * 2.0)
            {
              const float lower = 256.0 * 16.0 * 2.0;
              percent = (power - lower) / lower;
              index = 45;
            }
            else
            {
              const float lower = 256.0 * 16.0;
              percent = (power - lower) / lower;
              index = 44;
            }
        else
          if (power > 256.0 * 4.0)
            if (power > 256.0 * 4.0 * 2.0)
            {
              const float lower = 256.0 * 4.0 * 2.0;
              percent = (power - lower) / lower;
              index = 43;
            }
            else
            {
              const float lower = 256.0 * 4.0;
              percent = (power - lower) / lower;
              index = 42;
            }
          else
            if (power > 256.0 * 2.0)
            {
              const float lower = 256.0 * 2.0;
              percent = (power - lower) / lower;
              index = 41;
            }
            else
            {
              const float lower = 256.0;
              percent = (power - lower) / lower;
              index = 40;
            }
      else
        if (power > 16.0)
          if (power > 16.0 * 4.0)
            if (power > 16.0 * 4.0 * 2.0)
            {
              const float lower = 16.0 * 4.0 * 2.0;
              percent = (power - lower) / lower;
              index = 39;
            }
            else
            {
              const float lower = 16.0 * 4.0;
              percent = (power - lower) / lower;
              index = 38;
            }
          else
            if (power > 16.0 * 2.0)
            {
              const float lower = 16.0 * 2.0;
              percent = (power - lower) / lower;
              index = 37;
            }
            else
            {
              const float lower = 16.0;
              percent = (power - lower) / lower;
              index = 36;
            }
        else
          if (power > 4.0)
            if (power > 4.0 * 2.0)
            {
              const float lower = 4.0 * 2.0;
              percent = (power - lower) / lower;
              index = 35;
            }
            else
            {
              const float lower = 4.0;
              percent = (power - lower) / lower;
              index = 34;
            }
          else
            if (power > 2.0)
            {
              const float lower = 2.0;
              percent = (power - lower) / lower;
              index = 33;
            }
            else
            {
              const float lower = 1.0;
              percent = (power - lower) / lower;
              index = 32;
            }
}

float MathCache::powf_05_rough(const float power) const
{
  const int index = get_lower_index(power);
  return powf_map[5000][index];
}

float MathCache::powf_05_close(const float power) const
{
  float percent;
  int index;
  get_lower_index_and_percent(power, index, percent);
  return (powf_map[5000][index] * (1.0 - percent)) + (powf_map[5000][index + 1] * percent);
}

float MathCache::powf_0_1_rough_rough(const float base, const float power) const
{
  const int index1 = (int)(base * 10000);
  const int index2 = get_lower_index(power);
  return powf_map[index1][index2];
}

float MathCache::powf_0_1_rough_close(const float base, const float power) const
{
  const int index1 = (int)(base * 10000);
  float percent2;
  int index2;
  get_lower_index_and_percent(power, index2, percent2);
  return (powf_map[index1][index2] * (1.0 - percent2)) + (powf_map[index1][index2 + 1] * percent2);
}

float MathCache::powf_0_1_close_rough(const float base, const float power) const
{
  const int index1 = (int)(base * 10000);
  const float percent1 = (base - (float)index1 / 10000) * 10000;
  const int index2 = get_lower_index(power);
  return (powf_map[index1][index2] * (1.0 - percent1)) + (powf_map[index1 + 1][index2] * percent1);
}

float MathCache::powf_0_1_close_close(const float base, const float power) const
{
  const int index1 = (int)(base * 10000);
  const float percent1 = (base - (float)index1 / 10000) * 10000;
  float percent2;
  int index2;
  get_lower_index_and_percent(power, index2, percent2);
  return (powf_map[index1][index2] * (1.0 - percent1) * (1.0 - percent2)) + (powf_map[index1 + 1][index2] * percent1 * (1.0 - percent2)) + (powf_map[index1][index2 + 1] * (1.0 - percent1) * percent2) + (powf_map[index1 + 1][index2 + 1] * percent1 * percent2);
}


///////////////////////////////////////////////////////////////////////////////

};

#endif	// #ifdef EYE_CANDY
