// ========== Drop-in Hilligoss Wrapper ==========
// 
// Note: May require slight modification to the
// processing scripts, as the original was not
// consistent with the formatting on the options.
// Convention here is to have a space between an
// option and its corresponding value.
//     (e.g. "-c 2000" instead of "-c2000")
// 
// =============== DJ_Level_3 2025 ===============

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include "hilligoss.h"

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);
    std::string inputFileName = "";

    // Set defaults
    int black_level = 30;
    int white_level = 230;
    int jump_timer = 100;
    int syncCount = 1;
    int searchDistance = 30;
    double boost = 30;
    double curve = 1;

    // Get the default target point count from sample rate and fps
    double sampleRate = 192000;
    double fps = 24;
    int targetPointCount = int(sampleRate / fps);


    // Make sure there are arguments and that one of them is an input filename
    if (args.size() < 2 || std::find(args.begin(), args.end(), "-f") == args.end()) args[0] = "-h";

    // Loop over command-line arguments
    for (auto i = args.begin(); i != args.end(); ++i) {
        // get a string from the args
        std::string s = *i;

        // Help message
        if (s == "-h" || s == "--help") {
            std::cout << "Usage: hilligoss-nodeps -f <filename> -c <desired vectors per frame> -b <black threshold 0-255> -w <white threshold 1-255>" << std::endl;
            std::cout << "                        -j <jump time 1-10000> [-v (enable verbose mode)] [-t (enable tonal mode)]" << std::endl;
            std::cout << "    Defaults: hilligoss-nodeps -f <your_input_here.pgm> -c 8000 -b 30 -w 230 -j 100" << std::endl;
            std::cout << "    Notes : Images must be 8 - bit ASCII PGM, 512x512 only." << std::endl << std::endl;
            return 1;
        }

        // filename
        if (s == "-f") {
            inputFileName = *++i;
        }

        // target count
        else if (s == "-c") {
            targetPointCount = (int)std::stod(*++i);
        }

        // black level
        else if (s == "-b") {
            black_level = (int)std::stod(*++i);
        }

        // white level
        else if (s == "-w") {
            white_level = (int)std::stod(*++i);
        }

        // jump count
        else if (s == "-j") {
            jump_timer = (int)std::stod(*++i);
        }

        // tonal
        else if (s == "-t") {
            syncCount = 2;
        }
    }

    // Divide target point count by the sync count, resultant samples will be repeated later to equal the original target
    targetPointCount /= syncCount;

    // Temp variables
    int numRows = 0, numCols = 0;

    // This one's just for discarding unnecessary variables
    int trashCan = 0;
    std::stringstream ss;
    std::string inputLine = "";

    // Open the PGM file
    std::ifstream infile(inputFileName);

    // Make sure PGM version is P5
    getline(infile, inputLine);
    if (inputLine != "P5") {
        std::cerr << "Version error!" << std::endl;
        return -1;
    }

    // Read the rest of the file into the stringstream, this will tokenize it
    ss << infile.rdbuf();

    // Get the size of the image
    ss >> numCols >> numRows;

    // Make sure image size matches expected size
    if (numCols != PIX_CT || numRows != PIX_CT) {
        std::cerr << "Dimension mismatch: Expected " << PIX_CT << "x" << PIX_CT << ", got " << numRows << "x" << numCols << std::endl;
        return -2;
    }

    // Discard the next token, it's not needed
    ss >> trashCan;

    // Reserve the array of pixels
    std::vector<unsigned char> image;
    image.reserve(PIX_CT * PIX_CT);

    // Loop over the file
    for (int row = 0; row < numRows; ++row) {
        for (int col = 0; col < numCols; ++col) {
            // Get the next character as an unsigned char and put it in the array
            image.push_back(ss.get());
        }
    }

    // We're done with the input file now, so close it
    infile.close();

    // Create the vector of samples that Hilligoss will populate
    std::vector<int16_t> pcm;

    // Run Hilligoss!
    hilligoss(image, pcm, targetPointCount, black_level, white_level, jump_timer, searchDistance, boost, curve);

    // Generate the output file name and open it
    std::string outputFileName = inputFileName.substr(0, inputFileName.size() - 4).append(".pcm");
    std::ofstream outFile = std::ofstream(outputFileName.c_str(), std::ios_base::binary);

    // Write the PCM data to the file as many times as needed
    for (int i = 0; i < syncCount; i++) {
        outFile.write((char*)pcm.data(), pcm.size() * sizeof(int16_t));
    }

    // make sure file is fully written and then close it
    outFile.flush();
    outFile.close();
}