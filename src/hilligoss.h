#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define PIX_CT 512

#include <ranges>
#include <cfloat>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <numeric>
#include <random>
#include <fstream>

void hilligoss(const std::vector<unsigned char> image, std::vector<int16_t>& destination, int targetCount,
    unsigned char blackThreshold, unsigned char whiteThreshold, int jumpPeriod);
std::vector<int> choosePixels(const std::vector<unsigned char>& image, int targetCount, unsigned char black, unsigned char white, std::mt19937& g);
std::vector<int16_t> determinePath(std::vector<int> pixels, int targetCount, int jumpFrequency, std::mt19937& g);

#endif