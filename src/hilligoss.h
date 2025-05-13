/*
Copyright 2025 BUS ERROR Collective

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once
// ================== Hilligoss ==================
// 
// A library for converting grayscale bitmapped
// images into PCM audio! One source file and one
// header file. Inserts the resultant PCM onto
// the end of the destination vector, this is not
// very thread-safe so make sure that it gets its
// own vector all to itself with an std::ref if
// calling from a thread! If you're getting this
// from the Hilligoss-OpenCV repo, example syntax
// can be found in main-opencv.cpp if needed.
// 
// =============== DJ_Level_3 2025 ===============
// =============== BUS ERROR  2025 ===============

// The side length of the image Hilligoss expects
#define PIX_CT 512

#include <vector>
#include <random>
#include <iostream>
#include <cfloat>
#include <algorithm>

// Convert an 8-bit grayscale image into 16-bit stereo PCM
//   image: the image to convert, flattened row-by-row
//   destination: the vector to put the 16-bit samples into, alternating left and right
//   targetCount: how many samples to generate
//   blackThreshold: always eliminate pixels below this value
//   whiteThreshold: always accept pixels above this value
//   jumpPeriod: maximum number of samples in one stroke
//   searchDistance: how far to search for next sample in stroke
//   boost: how far to increase the pixel value at the black level
//   curve: how aggressively to curve the input pixels
//   mode: special modes (0 is default, 1 is stippling mode)
void hilligoss(const std::vector<unsigned char> image, std::vector<int16_t>& destination, int targetCount,
    unsigned char blackThreshold, unsigned char whiteThreshold, int jumpPeriod, int searchDistance, double boost, double curve, int mode = 0);

std::vector<int16_t> determinePath(std::vector<int> pixelsOriginal, int targetCount, int jumpPeriod, int searchDistance, std::mt19937& g);
std::vector<int> choosePixels(const std::vector<unsigned char>& image, int targetCount, unsigned char black, unsigned char white, double boost, double curve, int mode, std::mt19937& g);