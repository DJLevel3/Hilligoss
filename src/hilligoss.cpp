/*
Copyright 2025 BUS ERROR Collective

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "hilligoss.h"

// Quickly delete an item from a vector by swapping the target and the last element,
// then popping the last element. This does not preserve the order of the vector, but
// we're already working in a random order so it's fine
template <class T>
void quickDelete(std::vector<T>& vec, int idx) {
    vec[idx] = vec.back();
    vec.pop_back();
}

inline double lerp(double a, double b, double t) {
	return a + ((b - a) * t);
}

void hilligoss(const std::vector<unsigned char> image, std::vector<int16_t>& destination, int targetCount, unsigned char blackThreshold, unsigned char whiteThreshold, int jumpPeriod, int searchDistance, double boost, double curve, int mode, int frameNumber, int borderSamples) {
    // Create an RNG device for all the subfunctions
    std::random_device rd;
    std::mt19937 rng(rd());

    // Select a subset of pixels from the image
    std::vector<int> pixels = choosePixels(image, targetCount, blackThreshold, whiteThreshold, boost, curve, mode, rng, frameNumber);

    // Order the pixels and convert them into samples
    std::vector<int16_t> samples = determinePath(pixels, targetCount, jumpPeriod, searchDistance, mode, rng, frameNumber);
	
	if (borderSamples > 0) {
		std::vector<double> border;
		border.reserve(borderSamples * 2);
		int sideSamples = borderSamples / 4;
		int extraSideSamples = borderSamples % 4;
		
		// top
		for (int i = 0; i < sideSamples; i++) {
			border.push_back(lerp(-1.0, 1.0, i / (double)sideSamples));
			border.push_back(1.0);
		}
		
		// right
		for (int i = 0; i < sideSamples; i++) {
			border.push_back(1.0);
			border.push_back(lerp(1.0, -1.0, i / (double)sideSamples));
		}
		
		// bottom
		for (int i = 0; i < sideSamples; i++) {
			border.push_back(lerp(1.0, -1.0, i / (double)sideSamples));
			border.push_back(-1.0);
		}
		
		// left
		for (int i = 0; i < sideSamples + extraSideSamples; i++) {
			border.push_back(-1.0);
			border.push_back(lerp(-1.0, 1.0, i / (double)(extraSideSamples + sideSamples)));
		}
		
		samples.reserve(samples.size() + borderSamples * 2);
		
		for (int i = 0; i < borderSamples * 2; i++) {
			samples.push_back((int16_t)(border[i] * 32767));
		}
	}

    // Add the samples onto the end of the destination vector
    destination.insert(destination.end(), samples.begin(), samples.end());
}

// Choose <targetCount> pixels from <image> that are greater than <black>, skewing towards <white> with a curve factor of <curve> then boosting everything by <boost>.
std::vector<int> choosePixels(const std::vector<unsigned char>& image, int targetCount, unsigned char black, unsigned char white, double boost, double curve, int mode, std::mt19937& g, int frameNumber) {
    int pixelCount = 0;
    int x, y, x_temp, y_temp, s;
    double z, v;

    // This will be the list of chosen pixels
    std::vector<int> pixels;
    pixels.reserve(targetCount);

    // Create a lookup table of all the possible pixel values and their curved counterparts
    double lookup[256];
    for (double i = 0; i < 256; i++) {
        // Set white level using (i - black) / (white - black), clamped to prevent problems
        z = std::max(0.0, i - black) / std::max(0.00001, double(white - black));

        // Apply curve to adjusted pixels
        z = pow(z, pow(2, curve));

        // Apply boost
        z = boost + (255 - boost) * z;

        // Ensure values are not unreasonably high (to prevent rounding errors later)
        lookup[int(i)] = std::min(512.0, z);
    }

    // Create a list of candidates to select, then shuffle them up
    std::vector<int> candidates;
    candidates.reserve(PIX_CT * PIX_CT);
    for (int i = 0; i < PIX_CT * PIX_CT; i++) {
        if (mode >= 3 && mode <= 6) {
            x = i % PIX_CT;
            y = (int)(i / PIX_CT) % PIX_CT;

            x_temp = ((x >> (mode - 2)) << (mode - 2)) + ((frameNumber) % (int)std::pow(2, mode - 2));
            y_temp = ((y >> (mode - 2)) << (mode - 2)) + ((frameNumber) % (int)std::pow(2, mode - 2));

            if (x == x_temp || y == y_temp) {
                candidates.push_back(i);
            }
        }
        else {
            candidates.push_back(i);
        }
    }
    std::shuffle(candidates.begin(), candidates.end(),g);

    // This controls how likely a pixel is to be rejected when it's just above the threshold
    double greed = 0.8;

    // Start on the first pixel in the shuffled list
    int index = 0;

    // Loop until we've picked enough pixels
    while (pixelCount < targetCount) {
        // Pick a random number between 0 and <white> with some extra resolution
        z = rand() % (100 * white) * 0.01;

        // Removed to test different curve system
        //// If that random number is so low it can't be picked, don't check any further
		// if (z < lookup[black] * greed) continue;

        // Get the pixel value at the current index and curve it
        v = lookup[image[candidates[index]]] * greed;

        // get X and Y coordinates at the current index
        x = candidates[index] % PIX_CT;
        y = (int)(candidates[index] / PIX_CT) % PIX_CT;

        // If the candidate is less than the black value...
        if (image[candidates[index]] <= black) {
            // Remove it from the list, making sure not to skip a candidate
            quickDelete(candidates, index);
            index--;
        }

        // Otherwise, if the candidate's curved pixel value is greater than the random number...
        else if (v > z) {
            // Add it to the chosen pixels and increase the tally of chosen pixels
            pixels.push_back(x);
            pixels.push_back(y);
            pixelCount++;

            // Remove it from the list, making sure not to skip a candidate
            quickDelete(candidates, index);
            index--;
        }

        // If there's no candidates in the list now that we've removed some...
        if (candidates.size() <= 0) {
            // If there's zero valid pixels, add one in the center of the image
            while (pixels.size() < 2) pixels.push_back(PIX_CT >> 1);

            // Break out of the loop, as there aren't any candidates to potentially choose
            break;
        }

        // If the number of candidates left in the list is less than the number of candidates needed...
        if (candidates.size() < targetCount - pixelCount) {
            // Go through the remaining candidates
            for (index = 0; index < candidates.size(); index++) {
                // Get X and Y coordinates
                x = candidates[index] % PIX_CT;
                y = (int)(candidates[index] / PIX_CT) % PIX_CT;

                // If the candidate's pixel value is above the black threshold...
                if (image[candidates[index]] > black) {
                    // Add it to the list
                    pixels.push_back(x);
                    pixels.push_back(y);
                }
            }
            // Break out of the loop, as we've picked all the candidates
            break;
        }

        if (mode == 1 || mode == 2) {
            // Advance by a randomized amount of pixels to make reshuffling unnecessary
            index += rand() % PIX_CT;
        }
        else {
            // Advance by a small but randomized amount of pixels
            // This means we don't have to re-shuffle each time, making the algorithm significantly faster
            index += rand() % 32;
        }

        // If we've advanced past the end of the candidate list...
        if (index >= candidates.size()) {
            // Loop back through
            index = index % candidates.size();

            if (mode == 1) {
                // Increase greed a decent amount, breaking out relatively quickly
                greed = greed * 1.05;
                if (greed > 5) break;
            }
            else if (mode == 2) {
                // super duper stipply
                greed = greed * 1.05;
                if (greed > 2) break;
            }
            else {
                // Increase greed a bit, breaking out if it's big enough to prevent potential infinite loops
                greed = greed * 1.01;
                if (greed > 2) break;
            }
        }
    }

    // Return the finalized list of chosen candidates' X and Y coordinates.
    s = pixels.size();
    return pixels;
}

// Shorthand debug statements that are easier to find-and-replace away
void debug(int i) {
    std::cout << i << std::endl;
}
void debug(double i) {
    std::cout << i << std::endl;
}
void debug(long i, long i2) {
    std::cout << i << " " << i2 << std::endl;
}
void debug(std::string s) {
    std::cout << s << std::endl;
}

// Find an order through which the pixels should be traversed and convert it into 16-bit PCM audio
std::vector<int16_t> determinePath(std::vector<int>& pixelsOriginal, int targetCount, int jumpPeriod, int searchDistance, int mode, std::mt19937& g, int frameNumber)
{
    std::vector<long> path;
    path.reserve(pixelsOriginal.size());

    int pathLength = 0;
	
    // Create and populate a map of the pixels, where each true value is a pixel in the list
	std::vector<bool> map(PIX_CT * PIX_CT);
    int nPix = 0;
    for (int i = 0; i < pixelsOriginal.size() - 1; i++) {
        int s = pixelsOriginal.size();
        int p = pixelsOriginal[i];
        int p2 = pixelsOriginal[++i];
		map[p2 * PIX_CT + p] = true;
        nPix++;
	}


    // While we haven't hit the target count, and while there are still pixels on the map...
    while (pathLength < targetCount && nPix > 0)
    {
        // Allocate some memory for some pixel vectors so they don't have to expand later
        std::vector<int> pixels, pixelsTemp;
        pixelsTemp.reserve(nPix * 2);
        pixels.reserve(nPix * 2);

        // Get all the pixels currently in the map and put them in a temporary vector
        for (int x = 0; x < PIX_CT; x++) {
            for (int y = 0; y < PIX_CT; y++) {
                if (map[y * PIX_CT + x] == true) {
                    pixelsTemp.push_back(x);
                    pixelsTemp.push_back(y);
                }
            }
        }
		
        // Create a list of pixel indices, and shuffle it up
        std::vector<int> pixelIndices(pixelsTemp.size() / 2);
		for (int i = 0; i < pixelIndices.size(); i++) pixelIndices[i] = i;
        std::shuffle(pixelIndices.begin(), pixelIndices.end(),g);

        // Use the shuffled indices to shuffle the actual pixels
        for (int i = 0; i < pixelIndices.size(); i++) {
            pixels.push_back(pixelsTemp[pixelIndices[i] * 2]);
            pixels.push_back(pixelsTemp[pixelIndices[i] * 2 + 1]);
        }

        // If we don't have enough pixels to choose another, we can stop now
        if (pixels.size() < 2) {
            break;
        }

        // Choose a random starting point
        int startPoint = rand() % (pixels.size() / 2);
        
        // Add that starting point to the path
        path.push_back(pixels[startPoint*2]);
        path.push_back(pixels[startPoint*2+1]);
        pathLength++;

        // Remove the starting point from the map
        map[pixels[startPoint * 2 + 1] * PIX_CT + pixels[startPoint * 2]] = false;
        nPix--;

        for (int jumpCounter = 0; jumpCounter < jumpPeriod && pathLength < targetCount; jumpCounter++)
        {
            // Closest index is invalid for now
            long closestIndex = -1;

            // Closest distance is infinite for now
            double minDistance = DBL_MAX;
			
            // search square centered at current position and extending in each direction by searchDistance
            for (long x = std::max(0l, path[path.size() - 2] - searchDistance); x < std::min((long)(PIX_CT), path[path.size() - 2] + searchDistance); x++) {
                for (long y = std::max(0l, path[path.size() - 1] - searchDistance); y < std::min((long)(PIX_CT), path[path.size() - 1] + searchDistance); y++) {
                    // If there's a pixel on the map here and it's not the current pixel
                    if ((map[y * PIX_CT + x] == true) && (x != path[path.size() - 2]) && (y != path[path.size() - 1])) {
                        // Calculate the distance to the current pixel (sqrt(dx^2 + dy^2))
                         double dist = sqrt(pow(double(path[(path.size() - 2)]) - double(x), 2.0) + pow(double(path[(path.size() - 1)] - y), 2.0));
                        // If it's a new record for the closest point
                        if ((dist > 0) && (dist <= minDistance))
                        {
                            // Set the closest index and distance to the new record point
                            closestIndex = (y * PIX_CT) + x;
                            minDistance = dist;
                        }
                    }
                }
            }

            // If we found a pixel in the search radius
            if (closestIndex >= 0) 
            {
                // Add it to the path
                path.push_back((closestIndex % PIX_CT + PIX_CT) % PIX_CT);
                path.push_back((long(closestIndex / PIX_CT) % PIX_CT + PIX_CT) % PIX_CT);
                pathLength++;

                // Remove it from the map and the tally
                map[closestIndex] = false;
                nPix--;
            }
            // Otherwise, begin a new stroke
            else
            {
                break; 
            }
        }
    }
    if (path.size() == 0) {
        path.push_back(PIX_CT / 2);
        path.push_back(PIX_CT / 2);
        pathLength++;
    }

    // This will be returned once it's populated
    std::vector<int16_t> outputFile;

    // The 16-bit PCM values will be calculated here
    int16_t xShort, yShort;

    // How many pixels have been written to the actual return vector
    int pixelsWritten = 0;

    // How much farther we want to draw compared to how far we actually do draw per pixel
    double ratio = targetCount / (double)pathLength;

    // How far we expect to be through the path right now
    double expectedProgress = 0;

    for (int i = 0; i < path.size() / 2; i++)
    {
        // Convert X and Y to PCM values
        xShort = (int16_t)(path[i*2] % PIX_CT) * (65536 / PIX_CT) - 32768;
        yShort = -(int16_t)(path[i*2+1] % PIX_CT) * (65536 / PIX_CT) - 32768;

        // Put them in the PCM stream
        outputFile.push_back(xShort);
        outputFile.push_back(yShort);
        pixelsWritten++;

        // Increase the expected progress by the ratio found earlier
        expectedProgress += ratio;

        // If we haven't written as many pixels as expected, repeat the current one however much is needed
        while (pixelsWritten < expectedProgress) {
            outputFile.push_back(xShort);
            outputFile.push_back(yShort);
            pixelsWritten++;
        }
    }

    // Make 100% sure that there really are the right number of pixels in the target vector
    int i = 0;
    while (pixelsWritten < targetCount)
    {
        // Convert X and Y to PCM values
        xShort = (int16_t)(path[i * 2] % PIX_CT) * (65536 / PIX_CT) - 32767;
        yShort = -(int16_t)(path[i * 2 + 1] % PIX_CT) * (65536 / PIX_CT) - 32767;

        // Put them in the PCM stream
        outputFile.push_back(xShort);
        outputFile.push_back(yShort);
        pixelsWritten++;

        // Increment and wrap i around
        i = (i + 1) % (path.size() / 2);

    }

    return outputFile;
}
