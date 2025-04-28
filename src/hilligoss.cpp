#include "hilligoss.h"

void hilligoss(const std::vector<unsigned char> image, std::vector<int16_t>& destination, int targetCount, unsigned char blackThreshold, unsigned char whiteThreshold, int jumpPeriod) {
    std::random_device rd;
    std::mt19937 rng(rd());
    //auto now = std::chrono::high_resolution_clock::now();
    
    std::vector<int> pixels = choosePixels(image, targetCount, blackThreshold, whiteThreshold, rng);
    
    //double timeToExecute = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - now).count();
    //now = std::chrono::high_resolution_clock::now();

    std::vector<int16_t> samples = determinePath(pixels, targetCount, jumpPeriod, rng);
    
    //std::cout << "choosePixels: " << timeToExecute << ", determinePath: ";
    //timeToExecute = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - now).count();
    //std::cout << timeToExecute << std::endl;
    
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
            index = 0;
            std::shuffle(candidates.begin(), candidates.end(),g);
            greed = greed * 1.01;
            if (greed > 1.5) break; // many loops through
        }
    }
    pixels.resize(targetCount, PIX_CT / 2);
    return pixels;
}

std::vector<int16_t> determinePath(std::vector<int> pixels, int targetCount, int jumpPeriod, std::mt19937& g)
{
    std::vector<int16_t> outputFile;

    std::vector<int> path;
    path.resize(pixels.size());

    int pathLength = 0;

    std::vector<int> visited;

    while (pathLength < targetCount)
    {
        visited.clear();
        std::vector<int> pixelIndices(pixels.size() / 2);
		for (int i = 0; i < pixelIndices.size(); i++) pixelIndices[i] = i;
        std::shuffle(pixelIndices.begin(), pixelIndices.end(),g);

        std::vector<int> pixelsTemp;
        for (int i = 0; i < size(pixelIndices); i++) {
            pixelsTemp.push_back(pixels[pixelIndices[i] * 2]);
            pixelsTemp.push_back(pixels[pixelIndices[i] * 2+1]);
        }
        pixels = pixelsTemp;

        int startPoint = 0;

        path[pathLength*2] = pixels[startPoint*2];
        path[pathLength*2+1] = pixels[startPoint*2+1];
        visited.push_back(startPoint);
        pathLength++;
        for (int jumpCounter = 0; jumpCounter < jumpPeriod && pathLength < targetCount; jumpCounter++)
        {
            int closestIndex = -1;
            double minDistance = DBL_MAX;

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
            }
        }

        std::sort(visited.rbegin(), visited.rend());
        for (int a : visited) {
            pixels.erase(pixels.begin() + (a * 2 + 1));
            pixels.erase(pixels.begin() + (a * 2));
        }

        if (pixels.size() == 0)
        {
            break;
        }
    }
    int i;
    int pixelsWritten = 0;
    int16_t xShort, yShort;
    for (i = 0; i < path.size() / 2; i++)
    {
        xShort = (int16_t)(path[i*2] & 0x1FF) * 128 - 32767;
        yShort = -(int16_t)(path[i*2+1] & 0x1FF) * 128 - 32767;

        outputFile.push_back(xShort);
        outputFile.push_back(yShort);

        pixelsWritten++;
    }

    i = 0;

    while (pixelsWritten < targetCount)
    {
        xShort = (int16_t)(path[i*2] & 0x1FF) * 128 - 32767;
        yShort = -(int16_t)(path[i*2+1] & 0x1FF) * 128 - 32767;

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