#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define PIX_CT 512

#include <vector>
#include <random>
#include <iostream>
#include <cfloat>

void hilligoss(const std::vector<unsigned char> image, std::vector<int16_t>& destination, int targetCount,
    unsigned char blackThreshold, unsigned char whiteThreshold, int jumpPeriod, int searchDistance, double curve = 2.5);
std::vector<int> choosePixels(const std::vector<unsigned char>& image, int targetCount, unsigned char black, unsigned char white, std::mt19937& g, double curve);
std::vector<int16_t> determinePath(std::vector<int> pixels, int targetCount, int jumpFrequency, int searchDistance, std::mt19937& g);

#endif
