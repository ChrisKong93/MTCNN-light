#include "network.h"
#include "mtcnn.h"
//#include "facenet.h"
#include <time.h>

void run() {
    int b = 0;
    if (b == 0) {
        Mat image = imread("../1.jpeg");
//        Mat image = imread("C:\\Users\\Lenovo\\Desktop\\image/1156546.jpg");
        mtcnn find(image.rows, image.cols);
        clock_t start;
        start = clock();
        find.findFace(image);
        imshow("result", image);
        imwrite("../result.jpg", image);
        start = clock() - start;
        cout << "time is " << (double) start / CLOCKS_PER_SEC * 1000 << "ms" << endl;
        waitKey(0);
        image.release();
    } else {
        Mat image;
        VideoCapture cap(0);
        if (!cap.isOpened())
            cout << "fail to open!" << endl;
        cap >> image;
        if (!image.data) {
            cout << "读取视频失败" << endl;
            return;
        }

        mtcnn find(image.rows, image.cols);
        clock_t start;
        while (true) {
            start = clock();
            cap >> image;
            find.findFace(image);
            imshow("result", image);
            if (waitKey(1) >= 0) break;
            start = clock() - start;
            cout << "time is " << (double) start / CLOCKS_PER_SEC * 1000 << "ms" << endl;
        }
        waitKey(0);
        image.release();
    }
    return;
}


int main() {
    run();
    return 0;
}