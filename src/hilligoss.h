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
void hilligoss(const std::vector<unsigned char> image, std::vector<int16_t>& destination, int targetCount,
    unsigned char blackThreshold, unsigned char whiteThreshold, int jumpPeriod, int searchDistance, double boost, double curve);