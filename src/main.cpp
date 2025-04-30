#include "hilligoss.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

void monitor(bool* flag) {
    std::string str;
    if (fgetc(stdin) != EOF) *flag = true;
    *flag = true;
}

void show(const cv::Mat &img){
	cv::imshow("input",img);
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
    bool realtime = false;
	// parse args
	std::vector<std::string> args(argv + 1, argv + argc);
    std::string infname = "input.mp4";
    std::string outfname = "output.pcm";
    double sampleRate = 192000;
    int BATCH_SIZE = 1;
    int black_level = 30;
    int white_level = 230;
    int jump_timer = 100;
	double fps = -1;
    int searchDistance = 30;
    bool showPreview = false;
    int syncCount = 1;

    // Loop over command-line args
    // (Actually I usually use an ordinary integer loop variable and compare
    // args[i] instead of *i -- don't tell anyone! ;)
	if (args.size() < 2 || args[0] != "-i") args.push_back("-h");
    for (auto i = args.begin(); i != args.end(); ++i) {
        if (*i == "-h" || *i == "--help") {
            std::cout << "Syntax: Hilligoss-OpenCV -i <infile>\n Options: -o <outfile>" <<
                "\n          -b <black level>\n          -w <white level>\n          -j <jump spacing>" <<
                "\n          -r <sample rate>\n          -t <thread count>" << std::endl;
            return 0;
        }
        else if (*i == "-s" || *i == "-sync") {
            syncCount = 2;
        }
        else if (*i == "-i" || *i == "-input") {
            infname = *++i;
        }
        else if (*i == "-o" || *i == "-output") {
            outfname = *++i;
        }
        else if (*i == "-b" || *i == "-black") {
            black_level = stoi(*++i);
        }
        else if (*i == "-w" || *i == "-white") {
            white_level = stoi(*++i);
        }
        else if (*i == "-r" || *i == "-rate") {
            sampleRate = stod(*++i);
        }
        else if (*i == "-t" || *i == "-threads") {
            BATCH_SIZE = stoi(*++i);
        }
        else if (*i == "-j" || *i == "-jump") {
            jump_timer = stoi(*++i);
        }
		else if (*i == "-f" || *i == "-framerate" || *i == "-frequency") {
			fps = stod(*++i);
		}
        else if (*i == "-d" || *i == "-distance") {
            searchDistance = stoi(*++i);
            if (searchDistance < 1) searchDistance = PIX_CT;
        }
        else if (*i == "-p" || *i == "-preview") {
            showPreview = true;
        }
    }

    cv::String inFile(infname);

    cv::VideoCapture capture(inFile);
    if (!capture.isOpened()) {
        //error in opening the video input
        std::cerr << "Unable to open file!" << std::endl;
        return 0;
    }
    else {
        std::cout << "File opened!" << std::endl;
    }

    cv::Mat inFrame, procFrame;
    std::vector<unsigned char> frame(PIX_CT * PIX_CT);
    std::vector<int16_t> pcm;

    if (fps == -1) fps = capture.get(cv::CAP_PROP_FPS);
    double delta = 1000.0 / fps;
    int targetPointCount = (int)(sampleRate / fps / syncCount);

    srand((unsigned)time(NULL));

    std::vector<std::thread> threads;
    std::vector<std::vector<int16_t>> results(BATCH_SIZE);

    bool done = false;
    std::thread monitorThread(monitor, &done);

	int frameNumber = 0;
    while (done == false) {
        if (BATCH_SIZE < 1) {
            BATCH_SIZE = 1;
        }
        if (BATCH_SIZE == 1) std::cout << "Running frame " << frameNumber + 1 << std::endl;
        else std::cout << "Running frames " << frameNumber + 1 << " through " << frameNumber + BATCH_SIZE << std::endl;
        for (int t = 0; t < BATCH_SIZE; t++) {
            results[t].clear();
            capture >> inFrame;
            if (inFrame.empty()) {
                done = true;
                break;
            }

            cv::cvtColor(inFrame, inFrame, cv::COLOR_BGR2GRAY);
            inFrame.convertTo(procFrame, CV_8UC1);
            resizeKeepAspectRatio(procFrame, inFrame, cv::Size(PIX_CT, PIX_CT), {});

            // frame is now PIX_CTxPIX_CT, 8-bit grayscale
            if (t == 0 && showPreview) {
				show(inFrame);
			}

            //std::cout << t << std::endl;

            frame = (inFrame.isContinuous() ? inFrame : inFrame.clone()).reshape(1, 1); // data copy here
            //frame = std::vector<uchar>(inFrame.begin<uchar>(), inFrame.end<uchar>());

            // do the hilligoss stuff
            threads.push_back(std::thread(hilligoss, frame, std::ref(results[t]), targetPointCount, black_level, white_level, jump_timer, searchDistance));

			frameNumber++;
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

    std::ofstream outFile = std::ofstream(outfname.c_str(), std::ios_base::binary);

    outFile.write((char*)pcm.data(), pcm.size() * sizeof(int16_t));

    outFile.flush();
    outFile.close();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - now).count() * 0.001;
    std::cout << "Execution took " << (duration) << " seconds to process " << frameNumber << " frames." << std::endl;
    std::cout << "That's " << frameNumber / duration << " frames per second," << std::endl;
    std::cout << "or a speed factor of " << frameNumber / duration / fps << " (where >=1 is realtime)." << std::endl;
    std::cout << "Press enter to finish if you haven't already!" << std::endl;
    monitorThread.join();
}