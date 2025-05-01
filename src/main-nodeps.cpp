#include "hilligoss.h"

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);
    std::string infname = "input.pgm";
    std::string outfname = "output.pcm";
    double sampleRate = 192000;
    int BATCH_SIZE = 1;
    int black_level = 30;
    int white_level = 230;
    int jump_timer = 100;
    double fps = 24;
    int searchDistance = 30;
    bool showPreview = false;
    int syncCount = 1;
    int targetPointCount = int(sampleRate / fps);

    // Loop over command-line args
    // (Actually I usually use an ordinary integer loop variable and compare
    // args[i] instead of *i -- don't tell anyone! ;)
    if (args.size() < 2) args.push_back("-h");
    for (auto i = args.begin(); i != args.end(); ++i) {
        if (*i == "-h" || *i == "--help") {
            std::cout << "Usage: %s -f <filename> -c <desired vectors per frame> -b <black threshold 0-255> -w <white threshold 1-255>" << std::endl;
            std::cout << "    Example: % s - f image.pgm - c2000 - b20 - w234 - x160 - y100 - j200\n    Notes : Images must be 8 - bit ASCII PGM, 512x512 only.\n" << std::endl;
            return 1;
        }
        std::string s = *i;
        if (s.substr(0, 2) == "-f") {
            infname = s.substr(2, s.size() - 2);
        }
        if (s.substr(0, 2) == "-c") {
            targetPointCount = std::stoi(s.substr(2, s.size() - 2));
        }
        if (s.substr(0, 2) == "-b") {
            black_level = std::stoi(s.substr(2, s.size() - 2));
        }
        if (s.substr(0, 2) == "-w") {
            white_level = std::stoi(s.substr(2, s.size() - 2));
        }
        if (s.substr(0, 2) == "-j") {
            jump_timer = std::stoi(s.substr(2, s.size() - 2));
        }
    }

    outfname = infname.substr(0, infname.size() - 4).append(".pcm");

    int row = 0, col = 0, numrows = 0, numcols = 0, discard = 0;
    std::ifstream infile(infname);
    std::stringstream ss;
    std::string inputLine = "";

    // First line : version
    getline(infile, inputLine);
    if (inputLine != "P5") {
        std::cerr << "Version error!" << std::endl;
        return -1;
    }

    // Continue with a stringstream
    ss << infile.rdbuf();
    // Third line : size
    ss >> numcols >> numrows;
    if (numcols != PIX_CT || numrows != PIX_CT) {
        std::cerr << "Dimension mismatch: Expected " << PIX_CT << "x" << PIX_CT << ", got " << numrows << "x" << numcols << std::endl;
        return -2;
    }
    ss >> discard;

    std::vector<unsigned char> array;
    array.reserve(PIX_CT * PIX_CT);
    unsigned char res = 0;
    // Following lines : data 
    for (row = 0; row < numrows; ++row) {
        for (col = 0; col < numcols; ++col) {
            array.push_back(ss.get());
        }
    }
    infile.close();

    std::vector<int16_t> pcm;
    hilligoss(array, pcm, targetPointCount, black_level, white_level, jump_timer, searchDistance);

    std::ofstream outFile = std::ofstream(outfname.c_str(), std::ios_base::binary);

    outFile.write((char*)pcm.data(), pcm.size() * sizeof(int16_t));

    outFile.flush();
    outFile.close();
}