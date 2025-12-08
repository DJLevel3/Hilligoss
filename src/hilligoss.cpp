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

void hilligoss(const std::vector<unsigned char> image, std::vector<int16_t>& destination, int targetCount, unsigned char blackThreshold, unsigned char whiteThreshold, int jumpPeriod, int searchDistance, double boost, double curve, int mode, int frameNumber, int borderSamples, bool invert, std::mt19937 rng) {

#ifdef TIMEIT
    auto now1 = std::chrono::steady_clock::now();
#endif

    // Select a subset of pixels from the image
    std::vector<int> pixels = choosePixels(image, targetCount, blackThreshold, whiteThreshold, boost, curve, mode, rng, frameNumber, invert);

#ifdef TIMEIT

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now1).count() * 0.001;
    std::cout << " choosePixels - " << duration << " milliseconds" << std::endl;
    auto now2 = std::chrono::steady_clock::now();
#endif

    // Order the pixels and convert them into samples
    std::vector<int16_t> samples = determinePath(pixels, targetCount, jumpPeriod, searchDistance, rng);

#ifdef TIMEIT
    duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now2).count() * 0.001;
    std::cout << "determinePath - " << duration << " milliseconds" << std::endl;
    duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now1).count() * 0.001;
    std::cout << "        total - " << duration << " milliseconds" << std::endl << std::endl;
#endif

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
std::vector<int> choosePixels(const std::vector<unsigned char>& image, int targetCount, unsigned char black, unsigned char white, double boost, double curve, int mode, std::mt19937& g, int frameNumber, bool invert) {
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
        unsigned char pixelValue = image[candidates[index]] * (1 - (2 * invert));
        v = lookup[pixelValue] * greed;

        // get X and Y coordinates at the current index
        x = candidates[index] % PIX_CT;
        y = (int)(candidates[index] / PIX_CT) % PIX_CT;

        // If the candidate is less than the black value...
        if (pixelValue <= black) {
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
    if (s == 0) {
        pixels.push_back(0);
        pixels.push_back(0);
        s = pixels.size();
    }
    int ct = 0;
    while (s < targetCount * 2) {
        pixels.push_back(pixels[ct++]);
        pixels.push_back(pixels[ct++]);
        s = pixels.size();
    }
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
std::vector<int16_t> determinePath(std::vector<int>& pixelsOriginal, int targetCount, int jumpPeriod, int searchDistance, std::mt19937& rng)
{
	if (pixelsOriginal.size() == 0) {
		std::vector<int16_t> ret;
		ret.reserve(targetCount * 2);
		for (int i = 0; i < targetCount * 2; i++) {
			ret[i] = 0;
		}
		return ret;
	}
    std::vector<int16_t> path;
    path.resize(targetCount * 2, 0);

    int pathLength = 0;
    int nPix = targetCount;
    long x;
    long y;

    std::vector<int> indices;
    indices.resize(pixelsOriginal.size() / 2);
    for (int i = 0; i < indices.size(); i++) {
        indices[i] = i;
    }
    std::ranges::shuffle(indices, rng);
    for (int i = 0; i < indices.size(); i++) {
        pixelsOriginal[indices[i] * 2] = (pixelsOriginal[indices[i] * 2] - PIX_CT / 2) * SHRT_MAX / (PIX_CT);
        pixelsOriginal[indices[i] * 2 + 1] = -((pixelsOriginal[indices[i] * 2 + 1] - PIX_CT / 2) * SHRT_MAX / (PIX_CT)) - 1;
    }

    long sD = (searchDistance) * SHRT_MAX / (PIX_CT);
    sD = (sD * sD) + (sD * sD);

    long dist;

    // While we haven't hit the target count, and while there are still pixels on the map...
    while (pathLength < targetCount && nPix > 0)
    {
        // Add that starting point to the path
        path[pathLength * 2] = (pixelsOriginal[0]);
        path[pathLength * 2 + 1] = (pixelsOriginal[1]);
        pathLength++;

        // Remove the starting point from the map
        nPix--;
        pixelsOriginal[0] = pixelsOriginal[nPix * 2];
        pixelsOriginal[1] = pixelsOriginal[nPix * 2 + 1];

        for (int jumpCounter = 0; jumpCounter < jumpPeriod && pathLength < targetCount; jumpCounter++)
        {
            // Closest index is invalid for now
            long closestIndex = -1;

            // Closest distance is infinite for now
            long minDistance = LONG_MAX;

            for (int pixel = 0; pixel < nPix; pixel++) {
                x = (path[(pathLength * 2 - 2)]);
                y = (path[(pathLength * 2 - 1)]);
                // Calculate the distance to the current pixel (sqrt(dx^2 + dy^2))
                dist = (pixelsOriginal[pixel * 2] - x) * (pixelsOriginal[pixel * 2] - x)
                    + (pixelsOriginal[pixel * 2 + 1] - y) * (pixelsOriginal[pixel * 2 + 1] - y);
                // If it's a new record for the closest point
                if (dist < sD && ((dist > 0) && (dist <= minDistance)))
                {
                    // Set the closest index and distance to the new record point
                    closestIndex = pixel;
                    minDistance = dist;
                }
            }

            // If we found a pixel
            if (closestIndex >= 0)
            {
                // Add it to the path
                path[pathLength * 2] = (pixelsOriginal[closestIndex * 2]);
                path[pathLength * 2 + 1] = (pixelsOriginal[closestIndex * 2 + 1]);
                pathLength++;

                nPix--;
                pixelsOriginal[closestIndex * 2] = pixelsOriginal[nPix * 2];
                pixelsOriginal[closestIndex * 2 + 1] = pixelsOriginal[nPix * 2 + 1];
            }
            // Otherwise, begin a new stroke
            else
            {
                break;
            }
        }
    }
    
    for (int i = 0; i < path.size(); i++) path[i] *= 2;
    return path;
}
