/*
Copyright 2025 BUS ERROR Collective

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "hilligoss.h"

#include "AudioFile.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

#ifdef WIN32
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif

#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <fstream>
#include <iostream>
#include <ctime>

void show(const cv::Mat &img){
	cv::imshow("input",img);
}

bool kbhit(int time)
{
    if (time > 0) {
        nodelay(stdscr, FALSE);
    }
    else {
        nodelay(stdscr, TRUE);
    }
    int ch = wgetch(stdscr);

    if (ch != ERR) {
        ungetch(ch);
        return true;
    }
    else {
        return false;
    }
}

void resizeKeepAspectRatio(const cv::Mat& src, cv::Mat& dst, const cv::Size& dstSize, const cv::Scalar& backgroundColor)
{
    // Don't handle anything in this corner case
    if (dstSize.width <= 0 || dstSize.height <= 0)
        return;

    // Not job is needed here, let's avoid any copy
    if (src.cols == dstSize.width && src.rows == dstSize.height)
    {
        dst = src;
        return;
    }

    // Try not to reallocate memory if possible
    cv::Mat output = [&]()
        {
            if (dst.data != src.data && dst.cols == dstSize.width && dst.rows == dstSize.height && dst.type() == src.type())
                return dst;
            return cv::Mat(dstSize.height, dstSize.width, src.type());
        }();

    // 'src' inside 'dst'
    const auto imageBox = [&]()
        {
            const auto h1 = int(dstSize.width * (src.rows / (double)src.cols));
            const auto w2 = int(dstSize.height * (src.cols / (double)src.rows));

            const bool horizontal = h1 <= dstSize.height;

            const auto width = horizontal ? dstSize.width : w2;
            const auto height = horizontal ? h1 : dstSize.height;

            const auto x = horizontal ? 0 : int(double(dstSize.width - width) / 2.);
            const auto y = horizontal ? int(double(dstSize.height - height) / 2.) : 0;

            return cv::Rect(x, y, width, height);
        }();

    cv::Rect firstBox;
    cv::Rect secondBox;

    if (imageBox.width > imageBox.height)
    {
        // ┌──────────────►  x
        // │ ┌────────────┐
        // │ │┼┼┼┼┼┼┼┼┼┼┼┼│ firstBox
        // │ x────────────►
        // │ │            │
        // │ ▼────────────┤
        // │ │┼┼┼┼┼┼┼┼┼┼┼┼│ secondBox
        // │ └────────────┘
        // ▼
        // y

        firstBox.x = 0;
        firstBox.width = dstSize.width;
        firstBox.y = 0;
        firstBox.height = imageBox.y;

        secondBox.x = 0;
        secondBox.width = dstSize.width;
        secondBox.y = imageBox.y + imageBox.height;
        secondBox.height = dstSize.height - secondBox.y;
    }
    else
    {
        // ┌──────────────►  x
        // │ ┌──x──────►──┐
        // │ │┼┼│      │┼┼│
        // │ │┼┼│      │┼┼│
        // │ │┼┼│      │┼┼│
        // │ └──▼──────┴──┘
        // ▼  firstBox  secondBox
        // y

        firstBox.y = 0;
        firstBox.height = dstSize.height;
        firstBox.x = 0;
        firstBox.width = imageBox.x;

        secondBox.y = 0;
        secondBox.height = dstSize.height;
        secondBox.x = imageBox.x + imageBox.width;
        secondBox.width = dstSize.width - secondBox.x;
    }

    // Resizing to final image avoid useless memory allocation
    cv::Mat outputImage = output(imageBox);
    assert(outputImage.cols == imageBox.width);
    assert(outputImage.rows == imageBox.height);
    const auto* dataBeforeResize = outputImage.data;
    cv::resize(src, outputImage, cv::Size(outputImage.cols, outputImage.rows));
    assert(dataBeforeResize == outputImage.data);

    const auto drawBox = [&](const cv::Rect& box)
        {
            if (box.width > 0 && box.height > 0)
            {
                cv::rectangle(output, cv::Point(box.x, box.y), cv::Point(box.x + box.width, box.y + box.height), backgroundColor, -1);
            }
        };

    drawBox(firstBox);
    drawBox(secondBox);

    // Finally copy output to dst, like that user can use src & dst to the same cv::Mat
    dst = output;
}

int main(int argc, char*argv[]) {
    auto now = std::chrono::steady_clock::now();
	// parse args
	std::vector<std::string> args(argv + 1, argv + argc);
    std::string infname = "input.mp4";
    double sampleRate = 192000;
    int BATCH_SIZE = 1;
    int black_level = 30;
    int white_level = 230;
    int jump_timer = 100;
	double fps = -1;
    int searchDistance = 30;
    bool showPreview = false;
    int syncCount = 1;
    int boost = 30;
    double curve = 1;
    int frameLoop = 1;
    int mode = 0;

    std::time_t timestamp = time(NULL);
    char timestring[50];
    strftime(timestring, 50, "hilligoss-%m%d%y-%H%M%S.wav", localtime(&timestamp));
    std::string outfname = timestring;

    // Loop over command-line args
    // (Actually I usually use an ordinary integer loop variable and compare
    // args[i] instead of *i -- don't tell anyone! ;)
    if (argc < 2) {
        args.push_back("-h");
        args[0] = "-h";
    } else if (args.size() < 2 || std::find(args.begin(), args.end(), "-i") >= args.end() - 1) {
        args.push_back("-h");
        args[0] = "-h";
    }
    for (auto i = args.begin(); i != args.end(); ++i) {
        if (*i == "-h" || *i == "--help") {
            std::cout << "Syntax: Hilligoss-OpenCV -i <input filename>" <<
                "\n Options: -o <output filename>" <<
                "\n          -b <black level (0-255)>" <<
                "\n          -w <white level (0-255)>" <<
                "\n          -j <jump spacing (>= 1)>" <<
                "\n          -r <sample rate (>= 1)>" <<
                "\n          -t <thread count (>= 1)>" << 
                "\n          -f <framerate (>= 0.1)>" <<
                "\n          -d <search radius (<= 0 to disable)>" <<
                "\n          -p (enable preview)" <<
                "\n          -s (sync/tonal mode)" <<
                "\n          -c <curve (-2.0 to +2.0) - linear is 0.0, default is 1.0>" <<
                "\n          -fl <frame loop>" <<
                "\n          -bo <pixel brightness boost>" <<
                "\n          -mode <special mode>" <<
                "\n              0: normal" <<
                "\n              1: sparkly" <<
                "\n              2: extra sparkly" <<
                "\n              3-6: scrolling grid" << std::endl;

            return 0;
        }
        else if (*i == "-i" || *i == "-input") {
            infname = *++i;
        }
        else if (*i == "-o" || *i == "-output") {
            outfname = *++i;
        }
        else if (*i == "-b" || *i == "-black") {
            black_level = std::min(255, std::max(0, int(stod(*++i))));
        }
        else if (*i == "-w" || *i == "-white") {
            white_level = std::min(255, std::max(0, int(stod(*++i))));
        }
        else if (*i == "-j" || *i == "-jump") {
            jump_timer = std::max(stoi(*++i), 1);
        }
        else if (*i == "-r" || *i == "-rate") {
            sampleRate = std::max(1.0, stod(*++i));
        }
        else if (*i == "-t" || *i == "-threads") {
            BATCH_SIZE = std::max(1, stoi(*++i));
        }
		else if (*i == "-f" || *i == "-framerate" || *i == "-frequency") {
			fps = std::max(0.1, stod(*++i));
		}
        else if (*i == "-d" || *i == "-distance") {
            searchDistance = int(stod(*++i));
            if (searchDistance < 1) searchDistance = PIX_CT;
        }
        else if (*i == "-p" || *i == "-preview") {
            showPreview = true;
        }
        else if (*i == "-s" || *i == "-sync") {
            syncCount = 2;
        }
        else if (*i == "-c" || *i == "-curve") {
            curve = std::min(2.0, std::max(-2.0, stod(*++i)));
        }
        else if (*i == "-fl" || *i == "-frameloop") {
            frameLoop = std::max(1, stoi(*++i));
        }
        else if (*i == "-bo" || *i == "-boost") {
            boost = stod(*++i);
        }
        else if (*i == "-mode") {
            mode = std::max(0, int(stod(*++i)));
        }
    }

    initscr();
    cbreak();
    nodelay(stdscr, TRUE);
    noecho();
    flushinp();

    printw("Hilligoss 2.0\n");

    cv::String inFile(infname);

    cv::VideoCapture capture(inFile);
    if (!capture.isOpened()) {
        //error in opening the video input
        endwin();
        std::cout << "Hilligoss 2.0 - Unable to open video file!\n" << std::endl;
        return -1;
    }
    else {
        printw("Press enter to save to disk and quit, or shift+q to quit without saving.\n");
    }

    cv::Mat inFrame, procFrame;
    std::vector<unsigned char> frame(PIX_CT * PIX_CT);
    std::vector<int16_t> pcm;

    if (fps == -1) fps = capture.get(cv::CAP_PROP_FPS);
    double nFrames = capture.get(cv::CAP_PROP_FRAME_COUNT);
    double delta = 1000.0 / fps;
    int targetPointCount = (int)(sampleRate / fps / syncCount / frameLoop);

    srand((unsigned)time(NULL));

    std::vector<std::thread> threads;
    std::vector<std::vector<int16_t>> results(BATCH_SIZE);

    bool done = false;

	int frameNumber = 0;
    int counter = 0;
    while (done == false) {
        if (BATCH_SIZE < 1) {
            BATCH_SIZE = 1;
        }
        int f = (frameNumber / frameLoop);
        double progress = 100.0 * f / nFrames;
        if (BATCH_SIZE == 1) printw("\r%2.1f percent processed - Running frame %d", progress, f);
        else printw("\r%2.1f\% - Running frames %d through %d", progress, f, (frameNumber + BATCH_SIZE)/ frameLoop );
        for (int t = 0; t < BATCH_SIZE; t++) {
            results[t].clear();
            if (counter == 0) {
                capture >> inFrame;

                if (inFrame.empty()) {
                    done = true;
                    break;
                }

                cv::cvtColor(inFrame, inFrame, cv::COLOR_BGR2GRAY);
                inFrame.convertTo(procFrame, CV_8UC1);
                resizeKeepAspectRatio(procFrame, inFrame, cv::Size(PIX_CT, PIX_CT), {});
            }
            counter = (counter + 1) % frameLoop;


            // frame is now PIX_CTxPIX_CT, 8-bit grayscale
            if (t == 0 && showPreview) {
				show(inFrame);
			}

            frame = (inFrame.isContinuous() ? inFrame : inFrame.clone()).reshape(1, 1); // data copy here
            //frame = std::vector<uchar>(inFrame.begin<uchar>(), inFrame.end<uchar>());

            threads.push_back(std::thread(hilligoss, frame, std::ref(results[t]), targetPointCount, black_level, white_level, jump_timer, searchDistance, boost, curve, mode, frameNumber));

			frameNumber++;
        }
        refresh();
        if (kbhit(0)) {
            auto key = getch();
            if (key == '\n') {
                done = true;
            }
            else if (key == 'Q') {
                endwin();
                std::cout << "Hilligoss 2.0 - Cancelled due to user input!" << std::endl;
                for (std::thread& t : threads) {
                    t.join();
                }
                return 0;
            }
        }
        for (std::thread& t : threads) {
            t.join();
        }
        for (int t = 0; t < BATCH_SIZE; t++) {
            for (int s = 0; s < syncCount; s++) {
                pcm.insert(pcm.end(), results[t].begin(), results[t].end());
            }
        }
        threads.clear();
    }

    AudioFile<int16_t> outFile;
    outFile.setNumChannels(2);
    outFile.setSampleRate(sampleRate);
    outFile.setNumSamplesPerChannel(pcm.size() / 2);

    for (int channel = 0; channel < 2; channel++) {
        for (int i = 0; i < pcm.size() / 2; i++) {
            outFile.samples[channel][i] = pcm[i * 2 + channel];
        }
    }

    outFile.save(outfname, AudioFileFormat::Wave);

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - now).count() * 0.001;
    endwin();
    std::cout << "Hilligoss 2.0 - Execution took " << duration << " seconds to process " << int(frameNumber / frameLoop) << " frames. That's " << frameNumber / frameLoop / duration << " frames per second, or a speed factor of " << frameNumber / frameLoop / duration / fps << " (where >=1 is realtime)." << std::endl;
}
