#ifndef NETWORK_H
#define NETWORK_H

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <algorithm>
#include <stdlib.h>
#include <memory.h>
#include <fstream>
#include <cstring>
#include <string>
#include <math.h>
#include "pBox.h"

using namespace cv;

void addbias(struct pBox *pbox, mydataFmt *pbias);

void image2Matrix(const Mat &image, const struct pBox *pbox);

void maxPooling(const pBox *pbox, pBox *Matrix, int kernelSize, int stride);

void prelu(struct pBox *pbox, mydataFmt *pbias, mydataFmt *prelu_gmma);

void fullconnect(const Weight *weight, const pBox *pbox, pBox *outpBox);

void readData(string filename, long dataNumber[], mydataFmt *pTeam[]);

long initConvAndFc(struct Weight *weight, int schannel, int lchannel, int kersize, int stride, int pad);

void initpRelu(struct pRelu *prelu, int width);

void softmax(const struct pBox *pbox);

void image2MatrixInit(Mat &image, struct pBox *pbox);

void maxPoolingInit(const pBox *pbox, pBox *Matrix, int kernelSize, int stride);

void feature2MatrixInit(const pBox *pbox, pBox *Matrix, const Weight *weight);

void convolutionInit(const Weight *weight, const pBox *pbox, pBox *outpBox, const struct pBox *matrix);

void fullconnectInit(const Weight *weight, pBox *outpBox);


bool cmpScore(struct orderScore lsh, struct orderScore rsh);

void nms(vector<struct Bbox> &boundingBox_, vector<struct orderScore> &bboxScore_, const float overlap_threshold,
         string modelname = "Union");

void refineAndSquareBbox(vector<struct Bbox> &vecBbox, const int &height, const int &width);

void vectorXmatrix(float *matrix, float *v, int size, int v_w, int v_h, float *p);

void convolution(const Weight *weight, const pBox *pbox, pBox *outpBox);

#endif