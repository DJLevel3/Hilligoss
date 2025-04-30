#include "hilligoss.h"

//#define TIMEIT

void hilligoss(const std::vector<unsigned char> image, std::vector<int16_t>& destination, int targetCount, unsigned char blackThreshold, unsigned char whiteThreshold, int jumpPeriod, int searchDistance) {
    std::random_device rd;
    std::mt19937 rng(rd());
#ifdef TIMEIT
	auto now = std::chrono::high_resolution_clock::now();
#endif
    
    std::vector<int> pixels = choosePixels(image, targetCount, blackThreshold, whiteThreshold, rng);

    std::vector<int16_t> samples = determinePath(pixels, targetCount, jumpPeriod, searchDistance, rng);

#ifdef TIMEIT
    double timeToExecute = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - now).count();
    std::cout << "thread time: " << timeToExecute << std::endl << std::endl;
#endif
    
    destination.insert(destination.end(), samples.begin(), samples.end());
}

template <class T>
void quickDelete(std::vector<T>& vec, int idx)
{
    vec[idx] = vec.back();
    vec.pop_back();
}

std::vector<int> choosePixels(const std::vector<unsigned char>& image, int targetCount, unsigned char black, unsigned char white, std::mt19937& g) {
    int pixelCount = 0;
    double lookup[256];
    int x, y;
    double z, v;
    std::vector<int> pixels;
    pixels.reserve(targetCount);

    for (double i = 0; i < 256; i++) {
        lookup[(int)i] = pow(i / white, 2.5) * white;
    }

    std::vector<int> candidates(PIX_CT * PIX_CT);
	for (int i = 0; i < candidates.size(); i++) candidates[i] = i;
    std::shuffle(candidates.begin(), candidates.end(),g);

    double greed = 0.7;

    int index = 0;
    std::vector<int> erase;
    while (pixelCount < targetCount) {
        z = rand() % (100 * white) * 0.01;
		if (z < lookup[black] * greed) continue;
        x = candidates[index] % PIX_CT;
        y = (int)(candidates[index] / PIX_CT) % PIX_CT;
        v = lookup[image[y * PIX_CT + x]] * greed;
        if (image[y * PIX_CT + x] <= black) {
            quickDelete(candidates, index);
            index--;
        }
        else if (v > z) {
            pixelCount++;
            pixels.push_back(x);
            pixels.push_back(y);
            quickDelete(candidates, index);
            index--;
        }
        if (candidates.size() < targetCount - pixelCount) {
            for (index = 0; index < candidates.size(); index++) {
                x = candidates[index] % PIX_CT;
                y = (int)(candidates[index] / PIX_CT) % PIX_CT;
                v = lookup[image[y * PIX_CT + x]];
                if (image[y * PIX_CT + x] > black) {
                    pixels.push_back(x);
                    pixels.push_back(y);
                }
            }
            break;
        }
        index+= rand() % PIX_CT;
        if (index >= candidates.size()) {
            index = rand() % PIX_CT;
            //std::shuffle(candidates.begin(), candidates.end(),g);
            greed = greed * 1.01;
            if (greed > 1.5) break; // many loops through
        }
    }
    pixels.resize(targetCount, PIX_CT / 2);
    return pixels;
}

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

std::vector<int16_t> determinePath(std::vector<int> pixels, int targetCount, int jumpPeriod, int searchDistance, std::mt19937& g)
{
    std::vector<int16_t> outputFile;

    std::vector<long> path;
    path.reserve(pixels.size());

    int pathLength = 0;

    std::vector<int> visited;
	
	std::vector<bool> map(PIX_CT * PIX_CT);
	
    int nPix = 0;
    for (int i = 0; i < pixels.size() - 1;) {
        int p = pixels[i];
        i++;
        int p2 = pixels[i];
        i++;
		map[p2 * PIX_CT + p] = true;
        nPix++;
	}

    while (pathLength < targetCount && pixels.size() > 0)
    {
        visited.clear();
        std::vector<int> pixelsTemp;

        pixels.clear();
        pixels.reserve(nPix);

        for (int x = 0; x < PIX_CT; x++) {
            for (int y = 0; y < PIX_CT; y++) {
                if (map[y * PIX_CT + x] == true) {
                    pixels.push_back(x);
                    pixels.push_back(y);
                }
            }
        }
		
        std::vector<int> pixelIndices(pixels.size() / 2);
		for (int i = 0; i < pixelIndices.size(); i++) pixelIndices[i] = i;
        std::shuffle(pixelIndices.begin(), pixelIndices.end(),g);

        for (int i = 0; i < pixelIndices.size(); i++) {
            pixelsTemp.push_back(pixels[pixelIndices[i] * 2]);
            pixelsTemp.push_back(pixels[pixelIndices[i] * 2 + 1]);
        }
        pixels.clear();
        pixels.resize(pixelsTemp.size());
        for (int i = 0; i < pixelsTemp.size(); i++) {
            pixels[i] = pixelsTemp[i];
        }

        int startPoint = -1;
        int temp = -1;
        bool fucked = pixels.size() < 2;

        if (fucked) {
            debug("Something went very wrong, stopping early to prevent a crash!");
            break;
        }

        startPoint = rand() % (pixels.size() / 2);

        path.push_back(pixels[startPoint*2]);
        path.push_back(pixels[startPoint*2+1]);
        map[pixels[startPoint * 2 + 1] * PIX_CT + pixels[startPoint * 2]] = false;
        nPix--;
        visited.push_back(startPoint);
        pathLength++;

        for (int jumpCounter = 0; jumpCounter < jumpPeriod && pathLength < targetCount; jumpCounter++)
        {
            long closestIndex = -1;
            double minDistance = DBL_MAX;
			
			// slow! optimize this!
			/*
            for (int i = 0; i < (pixels.size() / 2); i++)
            {
                if (std::find(visited.begin(), visited.end(), i) == visited.end())
                {
                    double dist = sqrt(pow(path[(pathLength-1)*2] - pixels[i*2], 2) + pow(path[(pathLength - 1)*2+1] - pixels[i*2+1], 2));
                    if (dist < minDistance)
                    {
                        minDistance = dist;
                        closestIndex = i;
                    }
                }
            }
            if (closestIndex != -1) // Add that fucker to the path
            {
                path[pathLength*2] = pixels[closestIndex*2];
                path[pathLength*2+1] = pixels[closestIndex*2+1];
                visited.push_back(closestIndex);
                pathLength++;
            }
            else
            {
                break; // Break the loop if no unvisited pixels are found
            }*/
			
            // search within searchDistance
            for (long x = std::max(0l, path[path.size() - 2] - searchDistance);x < std::min((long)(PIX_CT), path[path.size() - 2] + searchDistance); x++) {
                for (long y = std::max(0l, path[path.size() - 1] - searchDistance); y < std::min((long)(PIX_CT), path[path.size() - 1] + searchDistance); y++) {
                    if ((map[y * PIX_CT + x] == true) && (x != path[path.size() - 2]) && (y != path[path.size() - 1])) {
                        double dist = sqrt(pow(double(path[(path.size() - 2)]) - double(x), 2.0) + pow(double(path[(path.size() - 1)] - y), 2.0));
                        //std::cout << x << " " << y << " " << dist << std::endl;
                        if (dist < minDistance)
                        {
                            minDistance = dist;
                            closestIndex = (y * PIX_CT) + x;
                        }
                    }
                }
            }

            if (closestIndex >= 0) // Add that fucker to the path
            {
                //std::cout << "l " << path.size() << std::endl;
                path.push_back((closestIndex % PIX_CT + PIX_CT) % PIX_CT);
                path.push_back((long(closestIndex / PIX_CT) % PIX_CT + PIX_CT) % PIX_CT);
                map[closestIndex] = false;
                nPix--;
                //std::cout << minDistance << std::endl;
                //std::cout << closestIndex << " " << path[path.size() - 2] << " " << path[path.size() - 1] << std::endl; // todo
                pathLength++;
            }
            else
            {
                break; // Break the loop if no unvisited pixels are found within distance
            }
        }
        //std::cout << path[path.size() - 2] << " " << path[path.size() - 1] << std::endl;

        //std::sort(visited.rbegin(), visited.rend());
        /*for (int a : visited) {
            pixels.erase(pixels.begin() + (a * 2 + 1));
            pixels.erase(pixels.begin() + (a * 2));
        }*/

        if (nPix == 0)
        {
            break;
        }
    }

    int i;
    int pixelsWritten = 0;
    int16_t xShort, yShort;
    for (i = 0; i < path.size() / 2; i++)
    {
        xShort = (int16_t)(path[i*2] % PIX_CT) * (65536 / PIX_CT) - 32767;
        yShort = -(int16_t)(path[i*2+1] % PIX_CT) * (65536 / PIX_CT) - 32767;

        outputFile.push_back(xShort);
        outputFile.push_back(yShort);

        pixelsWritten++;
    }

    i = 0;

    while (pixelsWritten < targetCount)
    {
        xShort = (int16_t)(path[i * 2] % PIX_CT) * (65536 / PIX_CT) - 32767;
        yShort = -(int16_t)(path[i * 2 + 1] % PIX_CT) * (65536 / PIX_CT) - 32767;

        outputFile.push_back(xShort);
        outputFile.push_back(yShort);

        pixelsWritten++;
        i++;

        if (i >= pathLength)
        {
            i = 0; // Loop back to start if we reach the end of the path
        }
    }

    return outputFile;
}