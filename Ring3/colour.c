/*
 * HFS/2 - A Hierarchical File System Driver for OS/2
 * Copyright (C) 1996, 1997 Marcus Better
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* colour.c
   Default Macintosh colour tables used for bitmap conversion.
   */

#define INCL_GPIBITMAPS
#include <os2.h>

RGB colour_table_4[16] = {
    {248, 252, 248},       /* colour   0 */
    {  0, 203, 248},       /* colour   1 */
    {  0,  74, 194},       /* colour   2 */
    {  1,   6, 180},       /* colour   3 */
    { 64,   1, 174},       /* colour   4 */
    { 37,   1,  34},       /* colour   5 */
    { 30,   1,  32},       /* colour   6 */
    {247, 129,   8},       /* colour   7 */
    {  2, 131,  37},       /* colour   8 */
    {  1,  94,   4},       /* colour   9 */
    {  0,   1,  97},       /* colour  10 */
    { 24,  53, 116},       /* colour  11 */
    {201, 166, 167},       /* colour  12 */
    { 85, 103, 104},       /* colour  13 */
    {  3,  35,  35},       /* colour  14 */
    {  0,   0,   0}        /* colour  15 */
};

RGB colour_table_8[256] = {
    {248, 252, 248},       /* colour   0 */
    {131, 208, 240},       /* colour   1 */
    {129, 208, 240},       /* colour   2 */
    {  8, 204, 236},       /* colour   3 */
    {  9, 204, 236},       /* colour   4 */
    {  0, 204, 248},       /* colour   5 */
    {167, 130, 197},       /* colour   6 */
    {242, 189, 188},       /* colour   7 */
    {132, 131, 210},       /* colour   8 */
    { 21, 132, 216},       /* colour   9 */
    { 19, 131, 216},       /* colour  10 */
    {  0, 131, 210},       /* colour  11 */
    {167, 131, 197},       /* colour  12 */
    {133, 131, 210},       /* colour  13 */
    {133, 131, 210},       /* colour  14 */
    { 15, 134, 213},       /* colour  15 */
    { 20, 130, 215},       /* colour  16 */
    {  0, 134, 206},       /* colour  17 */
    { 80,  41, 191},       /* colour  18 */
    { 80,  39, 191},       /* colour  19 */
    { 80,  39, 192},       /* colour  20 */
    {  7,  44, 191},       /* colour  21 */
    {  6,  43, 191},       /* colour  22 */
    {  0,  75, 194},       /* colour  23 */
    { 78,  37, 191},       /* colour  24 */
    { 78,  37, 191},       /* colour  25 */
    { 78,  37, 194},       /* colour  26 */
    {  8,  43, 194},       /* colour  27 */
    {  9,  43, 194},       /* colour  28 */
    {  1,  74, 194},       /* colour  29 */
    { 66,   2, 157},       /* colour  30 */
    { 64,   1, 173},       /* colour  31 */
    { 64,   1, 176},       /* colour  32 */
    {  3,   1, 175},       /* colour  33 */
    {  4,   1, 174},       /* colour  34 */
    {  0,  19, 188},       /* colour  35 */
    {247, 207, 194},       /* colour  36 */
    {128, 194, 194},       /* colour  37 */
    {128, 197, 194},       /* colour  38 */
    {  6, 197, 194},       /* colour  39 */
    {  6, 197, 194},       /* colour  40 */
    {  0, 197, 194},       /* colour  41 */
    {183, 131, 184},       /* colour  42 */
    {241, 189, 189},       /* colour  43 */
    {199, 164, 165},       /* colour  44 */
    { 21, 134, 194},       /* colour  45 */
    { 18, 132, 194},       /* colour  46 */
    {  0, 131, 194},       /* colour  47 */
    {178, 129, 184},       /* colour  48 */
    {200, 164, 164},       /* colour  49 */
    {133, 131, 194},       /* colour  50 */
    { 19, 131, 194},       /* colour  51 */
    { 19, 131, 194},       /* colour  52 */
    {  0, 130, 194},       /* colour  53 */
    { 77,  37, 136},       /* colour  54 */
    { 82,  43, 191},       /* colour  55 */
    { 82,  43, 191},       /* colour  56 */
    { 20,  52, 194},       /* colour  57 */
    { 20,  52, 194},       /* colour  58 */
    {  0,  66, 193},       /* colour  59 */
    { 77,  37, 136},       /* colour  60 */
    { 82,  44, 191},       /* colour  61 */
    { 80,  44, 191},       /* colour  62 */
    { 15,  47, 195},       /* colour  63 */
    { 18,  50, 193},       /* colour  64 */
    {  0,  67, 194},       /* colour  65 */
    { 66,   9, 134},       /* colour  66 */
    { 66,   1, 153},       /* colour  67 */
    { 66,   1, 154},       /* colour  68 */
    {  8,   1, 173},       /* colour  69 */
    {  6,   1, 173},       /* colour  70 */
    {  0,   6, 180},       /* colour  71 */
    {248, 194, 132},       /* colour  72 */
    { 69, 162, 121},       /* colour  73 */
    { 69, 162, 121},       /* colour  74 */
    {  1, 157, 128},       /* colour  75 */
    {  0, 158, 128},       /* colour  76 */
    {  0, 176, 132},       /* colour  77 */
    {241, 128, 106},       /* colour  78 */
    {195, 162, 163},       /* colour  79 */
    {136, 132, 122},       /* colour  80 */
    { 21, 134, 120},       /* colour  81 */
    { 20, 134, 120},       /* colour  82 */
    {  0, 134, 128},       /* colour  83 */
    {242, 132, 105},       /* colour  84 */
    {137, 134, 121},       /* colour  85 */
    {119, 123, 124},       /* colour  86 */
    { 26, 131, 120},       /* colour  87 */
    { 27, 131, 120},       /* colour  88 */
    {  0, 134, 127},       /* colour  89 */
    { 89,  36,  99},       /* colour  90 */
    { 92,  47, 110},       /* colour  91 */
    { 92,  48, 109},       /* colour  92 */
    { 24,  54, 117},       /* colour  93 */
    { 22,  52, 117},       /* colour  94 */
    {  0,  59, 131},       /* colour  95 */
    { 80,  36, 100},       /* colour  96 */
    { 92,  47, 110},       /* colour  97 */
    { 92,  48, 109},       /* colour  98 */
    { 18,  48, 117},       /* colour  99 */
    { 17,  48, 117},       /* colour 100 */
    {  0,  62, 132},       /* colour 101 */
    { 56,   1,  97},       /* colour 102 */
    { 56,   1,  97},       /* colour 103 */
    { 55,   1,  97},       /* colour 104 */
    { 26,   1, 122},       /* colour 105 */
    { 27,   1, 122},       /* colour 106 */
    {  1,   2, 133},       /* colour 107 */
    {175, 159, 101},       /* colour 108 */
    { 67, 133,  95},       /* colour 109 */
    { 67, 133,  96},       /* colour 110 */
    {  0, 135,  96},       /* colour 111 */
    {  0, 137,  94},       /* colour 112 */
    {  0, 153, 120},       /* colour 113 */
    {248, 128,  94},       /* colour 114 */
    {134, 133,  92},       /* colour 115 */
    {135, 133,  93},       /* colour 116 */
    { 20, 132,  93},       /* colour 117 */
    { 20, 131,  93},       /* colour 118 */
    {  0, 134,  95},       /* colour 119 */
    {248, 128,  95},       /* colour 120 */
    {133, 131,  93},       /* colour 121 */
    {133, 131,  97},       /* colour 122 */
    { 20, 131,  97},       /* colour 123 */
    { 19, 131,  96},       /* colour 124 */
    {  0, 134,  94},       /* colour 125 */
    { 82,  36,  93},       /* colour 126 */
    {102,  46,  92},       /* colour 127 */
    {104,  47,  92},       /* colour 128 */
    { 65,  96,  95},       /* colour 129 */
    { 17,  49,  93},       /* colour 130 */
    {  0,  59,  95},       /* colour 131 */
    { 82,  35,  93},       /* colour 132 */
    {104,  47,  93},       /* colour 133 */
    {104,  47,  94},       /* colour 134 */
    { 20,  51,  93},       /* colour 135 */
    {  1,  32,  33},       /* colour 136 */
    {  0,  59,  95},       /* colour 137 */
    { 41,   1,  68},       /* colour 138 */
    { 42,   1,  70},       /* colour 139 */
    { 42,   1,  70},       /* colour 140 */
    { 23,   1,  95},       /* colour 141 */
    { 23,   1,  96},       /* colour 142 */
    {  0,   1,  96},       /* colour 143 */
    {135, 139,  92},       /* colour 144 */
    { 48, 130,  42},       /* colour 145 */
    { 47, 131,  43},       /* colour 146 */
    {  0, 130,  78},       /* colour 147 */
    {  0, 130,  79},       /* colour 148 */
    {  0, 140, 100},       /* colour 149 */
    {246, 130,  37},       /* colour 150 */
    {131, 130,  36},       /* colour 151 */
    {131, 130,  37},       /* colour 152 */
    { 10, 130,  37},       /* colour 153 */
    { 10, 129,  36},       /* colour 154 */
    {  0, 130,  54},       /* colour 155 */
    {246, 128,  37},       /* colour 156 */
    {131, 130,  37},       /* colour 157 */
    {131, 131,  35},       /* colour 158 */
    { 11, 128,  33},       /* colour 159 */
    { 14, 128,  37},       /* colour 160 */
    {  0, 134,  54},       /* colour 161 */
    { 78,  37,  37},       /* colour 162 */
    {106,  43,  37},       /* colour 163 */
    {106,  43,  37},       /* colour 164 */
    { 14,  45,  36},       /* colour 165 */
    {  2,  34,  33},       /* colour 166 */
    {  0,  52,  36},       /* colour 167 */
    { 82,  34,  37},       /* colour 168 */
    {106,  43,  37},       /* colour 169 */
    {106,  43,  37},       /* colour 170 */
    {  3,  34,  34},       /* colour 171 */
    {  0,  32,  32},       /* colour 172 */
    {  0,  52,  37},       /* colour 173 */
    { 33,   1,  37},       /* colour 174 */
    { 30,   1,  35},       /* colour 175 */
    { 36,   0,  37},       /* colour 176 */
    { 25,   1,  37},       /* colour 177 */
    { 25,   0,  37},       /* colour 178 */
    {  0,   1,  44},       /* colour 179 */
    {129, 135,  53},       /* colour 180 */
    { 19, 128,  33},       /* colour 181 */
    { 20, 127,  33},       /* colour 182 */
    {  1, 129,  37},       /* colour 183 */
    {  1, 130,  37},       /* colour 184 */
    {  0, 134,  95},       /* colour 185 */
    {244, 128,   7},       /* colour 186 */
    {112, 124,  10},       /* colour 187 */
    {112, 124,  10},       /* colour 188 */
    {  6, 106,   1},       /* colour 189 */
    {  4, 105,   2},       /* colour 190 */
    {  1, 130,  36},       /* colour 191 */
    {246, 128,   8},       /* colour 192 */
    {112, 124,   8},       /* colour 193 */
    {112, 124,   8},       /* colour 194 */
    {  1, 103,   2},       /* colour 195 */
    {  2, 104,   2},       /* colour 196 */
    {  2, 128,  36},       /* colour 197 */
    { 78,  34,  22},       /* colour 198 */
    {110,  43,   2},       /* colour 199 */
    {110,  43,   2},       /* colour 200 */
    { 21,  54,   3},       /* colour 201 */
    { 20,  54,   2},       /* colour 202 */
    {  3,  94,   2},       /* colour 203 */
    { 78,  34,  22},       /* colour 204 */
    {110,  43,   2},       /* colour 205 */
    {110,  44,   2},       /* colour 206 */
    { 16,  50,   2},       /* colour 207 */
    {  0,  32,  32},       /* colour 208 */
    {  2,  94,   3},       /* colour 209 */
    { 31,   0,  32},       /* colour 210 */
    { 28,   1,   6},       /* colour 211 */
    { 29,   1,   7},       /* colour 212 */
    { 28,   1,   5},       /* colour 213 */
    { 29,   1,   2},       /* colour 214 */
    {  0,  19, 189},       /* colour 215 */
    {  0,   5, 180},       /* colour 216 */
    {  0,   2, 169},       /* colour 217 */
    {  0,   2, 169},       /* colour 218 */
    {  0,   2, 133},       /* colour 219 */
    {  0,   2,  98},       /* colour 220 */
    {  0,   2,  97},       /* colour 221 */
    {  0,   1,  43},       /* colour 222 */
    {  0,   1,   2},       /* colour 223 */
    {  0,   0,   0},       /* colour 224 */
    {  0, 136,  96},       /* colour 225 */
    {  0, 136,  95},       /* colour 226 */
    {  0, 131,  35},       /* colour 227 */
    {  1, 131,  34},       /* colour 228 */
    {  1, 131,  35},       /* colour 229 */
    {  1,  96,   3},       /* colour 230 */
    {  1,  96,   3},       /* colour 231 */
    {  0,  96,   3},       /* colour 232 */
    {  0,   1,   1},       /* colour 233 */
    {  0,   0,   0},       /* colour 234 */
    { 31,   1,  32},       /* colour 235 */
    { 30,   1,  32},       /* colour 236 */
    { 28,   1,   6},       /* colour 237 */
    { 28,   1,   6},       /* colour 238 */
    { 26,   1,   6},       /* colour 239 */
    { 29,   1,   6},       /* colour 240 */
    { 27,   1,   6},       /* colour 241 */
    { 28,   1,   6},       /* colour 242 */
    {  0,   1,   1},       /* colour 243 */
    {  0,   1,   1},       /* colour 244 */
    {247, 203, 203},       /* colour 245 */
    {246, 194, 194},       /* colour 246 */
    {173, 152, 153},       /* colour 247 */
    {137, 133, 133},       /* colour 248 */
    { 83, 104, 104},       /* colour 249 */
    { 68,  98,  98},       /* colour 250 */
    { 14,  45,  45},       /* colour 251 */
    {  6,  34,  34},       /* colour 252 */
    {  0,  12,  12},       /* colour 253 */
    {  0,   2,   2},       /* colour 254 */
    {  0,   0,   0}        /* colour 255 */
};
