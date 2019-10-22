#include "network.h"

void addbias(struct pBox *pbox, mydataFmt *pbias) {
    if (pbox->pdata == NULL) {
        cout << "Relu feature is NULL!!" << endl;
        return;
    }
    if (pbias == NULL) {
        cout << "the  Relu bias is NULL!!" << endl;
        return;
    }
    mydataFmt *op = pbox->pdata;
    mydataFmt *pb = pbias;

    long dis = pbox->width * pbox->height;
    for (int channel = 0; channel < pbox->channel; channel++) {
        for (int col = 0; col < dis; col++) {
            *op = *op + *pb;
            op++;
        }
        pb++;
    }
}

void image2MatrixInit(Mat &image, struct pBox *pbox) {
    if ((image.data == NULL) || (image.type() != CV_8UC3)) {
        cout << "image's type is wrong!!Please set CV_8UC3" << endl;
        return;
    }
    pbox->channel = image.channels();
    pbox->height = image.rows;
    pbox->width = image.cols;

    pbox->pdata = (mydataFmt *) malloc(pbox->channel * pbox->height * pbox->width * sizeof(mydataFmt));
    if (pbox->pdata == NULL)cout << "the image2MatrixInit failed!!" << endl;
    memset(pbox->pdata, 0, pbox->channel * pbox->height * pbox->width * sizeof(mydataFmt));
}

void image2Matrix(const Mat &image, const struct pBox *pbox) {
    if ((image.data == NULL) || (image.type() != CV_8UC3)) {
        cout << "image's type is wrong!!Please set CV_8UC3" << endl;
        return;
    }
    if (pbox->pdata == NULL) {
        return;
    }
    mydataFmt *p = pbox->pdata;
    for (int rowI = 0; rowI < image.rows; rowI++) {
        for (int colK = 0; colK < image.cols; colK++) {
            *p = (image.at<Vec3b>(rowI, colK)[0] - 127.5) * 0.0078125;
            *(p + image.rows * image.cols) = (image.at<Vec3b>(rowI, colK)[1] - 127.5) * 0.0078125;
            *(p + 2 * image.rows * image.cols) = (image.at<Vec3b>(rowI, colK)[2] - 127.5) * 0.0078125;
            p++;
        }
    }
}

void featurePadInit(const pBox *pbox, pBox *outpBox, const int pad) {
    if (pad <= 0) {
        cout << "the data needn't to pad,please check you network!" << endl;
        return;
    }
    outpBox->channel = pbox->channel;
    outpBox->height = pbox->height + 2 * pad;
    outpBox->width = pbox->width + 2 * pad;
    long RowByteNum = outpBox->width * sizeof(mydataFmt);
    outpBox->pdata = (mydataFmt *) malloc(outpBox->channel * outpBox->height * RowByteNum);
    if (outpBox->pdata == NULL)cout << "the featurePadInit is failed!!" << endl;
    memset(outpBox->pdata, 0, outpBox->channel * outpBox->height * RowByteNum);
}

void featurePad(const pBox *pbox, const pBox *outpBox, const int pad) {
    mydataFmt *p = outpBox->pdata;
    mydataFmt *pIn = pbox->pdata;

    for (int row = 0; row < outpBox->channel * outpBox->height; row++) {

        if ((row % outpBox->height) < pad || (row % outpBox->height > (outpBox->height - pad - 1))) {
            p += outpBox->width;
            continue;
        }
        p += pad;
        memcpy(p, pIn, pbox->width * sizeof(mydataFmt));
        p += pbox->width + pad;
        pIn += pbox->width;
    }
}

void convolutionInit(const Weight *weight, const pBox *pbox, pBox *outpBox) {
    outpBox->channel = weight->selfChannel;
    outpBox->width = (pbox->width - weight->kernelSize) / weight->stride + 1;
    outpBox->height = (pbox->height - weight->kernelSize) / weight->stride + 1;
    outpBox->pdata = (mydataFmt *) malloc(outpBox->width * outpBox->height * outpBox->channel * sizeof(mydataFmt));
    if (outpBox->pdata == NULL)cout << "the convolutionInit is failed!!" << endl;
    memset(outpBox->pdata, 0, outpBox->width * outpBox->height * outpBox->channel * sizeof(mydataFmt));
}

void convolution(const Weight *weight, const pBox *pbox, pBox *outpBox) {
    if (weight->pad != 0) {
        struct pBox *padpbox = new pBox;
        featurePad(pbox, padpbox, weight->pad);
    }
    int ckh, ckw, ckd, cknum, imginputh, imginputw, imginputd;
    float *ck, *imginput;
    float *r = outpBox->pdata;
    float temp;
    ck = weight->pdata;
    ckh = weight->kernelSize;
    ckw = weight->kernelSize;
    ckd = weight->lastChannel;
    cknum = weight->selfChannel;
    imginput = pbox->pdata;
    imginputh = pbox->height;
    imginputw = pbox->width;
    imginputd = pbox->channel;
    for (int i = 0; i < cknum; ++i) {
        for (int j = 0; j < imginputh - ((ckh + 1) / 2); ++j) {
            for (int k = 0; k < imginputw - ((ckw + 1) / 2); ++k) {
                temp = 0.0;
                for (int m = 0; m < ckd; ++m) {
                    for (int n = 0; n < ckh; ++n) {
                        for (int i1 = 0; i1 < ckw; ++i1) {
                            temp +=
                                    imginput[(j + n) * imginputw + (k + i1) + m * imginputh * imginputw]
                                    * ck[i * ckh * ckw * ckd + m * ckh * ckw + n * ckw + i1];
                        }
                    }
                }
                //按照顺序存储
                r[i * outpBox->height * outpBox->width + j * outpBox->width + k] = temp;
            }

        }
    }
}

void maxPoolingInit(const pBox *pbox, pBox *Matrix, int kernelSize, int stride) {
    Matrix->width = ceil((float) (pbox->width - kernelSize) / stride + 1);
    Matrix->height = ceil((float) (pbox->height - kernelSize) / stride + 1);
    Matrix->channel = pbox->channel;
    Matrix->pdata = (mydataFmt *) malloc(Matrix->channel * Matrix->width * Matrix->height * sizeof(mydataFmt));
    if (Matrix->pdata == NULL)cout << "the maxPoolingInit is failed!!" << endl;
    memset(Matrix->pdata, 0, Matrix->channel * Matrix->width * Matrix->height * sizeof(mydataFmt));
}

void maxPooling(const pBox *pbox, pBox *Matrix, int kernelSize, int stride) {
    if (pbox->pdata == NULL) {
        cout << "the feature2Matrix pbox is NULL!!" << endl;
        return;
    }
    mydataFmt *p = Matrix->pdata;
    mydataFmt *pIn;
    mydataFmt *ptemp;
    mydataFmt maxNum = 0;
    if ((pbox->width - kernelSize) % stride == 0 && (pbox->height - kernelSize) % stride == 0) {
        for (int row = 0; row < Matrix->height; row++) {
            for (int col = 0; col < Matrix->width; col++) {
                pIn = pbox->pdata + row * stride * pbox->width + col * stride;
                for (int channel = 0; channel < pbox->channel; channel++) {
                    ptemp = pIn + channel * pbox->height * pbox->width;
                    maxNum = *ptemp;
                    for (int kernelRow = 0; kernelRow < kernelSize; kernelRow++) {
                        for (int i = 0; i < kernelSize; i++) {
                            if (maxNum < *(ptemp + i + kernelRow * pbox->width))
                                maxNum = *(ptemp + i + kernelRow * pbox->width);
                        }
                    }
                    *(p + channel * Matrix->height * Matrix->width) = maxNum;
                }
                p++;
            }
        }
    } else {
        int diffh = 0, diffw = 0;
        for (int channel = 0; channel < pbox->channel; channel++) {
            pIn = pbox->pdata + channel * pbox->height * pbox->width;
            for (int row = 0; row < Matrix->height; row++) {
                for (int col = 0; col < Matrix->width; col++) {
                    ptemp = pIn + row * stride * pbox->width + col * stride;
                    maxNum = *ptemp;
                    diffh = row * stride - pbox->height + 1;
                    diffw = col * stride - pbox->width + 1;
                    for (int kernelRow = 0; kernelRow < kernelSize; kernelRow++) {
                        if ((kernelRow + diffh) > 0)break;
                        for (int i = 0; i < kernelSize; i++) {
                            if ((i + diffw) > 0)break;
                            if (maxNum < *(ptemp + i + kernelRow * pbox->width))
                                maxNum = *(ptemp + i + kernelRow * pbox->width);
                        }
                    }
                    *p++ = maxNum;
                }
            }
        }
    }
}

void prelu(struct pBox *pbox, mydataFmt *pbias, mydataFmt *prelu_gmma) {
    if (pbox->pdata == NULL) {
        cout << "the  Relu feature is NULL!!" << endl;
        return;
    }
    if (pbias == NULL) {
        cout << "the  Relu bias is NULL!!" << endl;
        return;
    }
    mydataFmt *op = pbox->pdata;
    mydataFmt *pb = pbias;
    mydataFmt *pg = prelu_gmma;

    long dis = pbox->width * pbox->height;
    for (int channel = 0; channel < pbox->channel; channel++) {
        for (int col = 0; col < dis; col++) {
            *op = *op + *pb;
            *op = (*op > 0) ? (*op) : ((*op) * (*pg));
            op++;
        }
        pb++;
        pg++;
    }
}

void fullconnectInit(const Weight *weight, pBox *outpBox) {

    outpBox->channel = weight->selfChannel;
    outpBox->width = 1;
    outpBox->height = 1;
    outpBox->pdata = (mydataFmt *) malloc(weight->selfChannel * sizeof(mydataFmt));
    if (outpBox->pdata == NULL)cout << "the fullconnectInit is failed!!" << endl;
    memset(outpBox->pdata, 0, weight->selfChannel * sizeof(mydataFmt));
}

void fullconnect(const Weight *weight, const pBox *pbox, pBox *outpBox) {
    if (pbox->pdata == NULL) {
        cout << "the fc feature is NULL!!" << endl;
        return;
    }
    if (weight->pdata == NULL) {
        cout << "the fc weight is NULL!!" << endl;
        return;
    }
    memset(outpBox->pdata, 0, weight->selfChannel * sizeof(mydataFmt));
    //Y←αAX + βY    β must be 0(zero)
    //               row         no trans         A's row               A'col
    //cblas_sgemv(CblasRowMajor, CblasNoTrans, weight->selfChannel, weight->lastChannel, 1, weight->pdata, weight->lastChannel, pbox->pdata, 1, 0, outpBox->pdata, 1);
    vectorXmatrix(pbox->pdata, weight->pdata,
                  pbox->width * pbox->height * pbox->channel,
                  weight->lastChannel, weight->selfChannel,
                  outpBox->pdata);
}

void vectorXmatrix(float *matrix, float *v, int size, int v_w, int v_h, float *p) {
    for (int i = 0; i < v_h; i++) {
        for (int j = 0; j < v_w; j++) {
            p[i] += matrix[j] * v[i * v_w + j];
        }
    }
}

void readData(string filename, long dataNumber[], mydataFmt *pTeam[]) {
    ifstream in(filename.data());
    string line;
    if (in) {
        int i = 0;
        int count = 0;
        int pos = 0;
        while (getline(in, line)) {
            try {
                if (i < dataNumber[count]) {
                    line.erase(0, 1);
                    pos = line.find(']');
                    line.erase(pos, 1);
                    *(pTeam[count])++ = atof(line.data());
                } else {
                    count++;
                    dataNumber[count] += dataNumber[count - 1];

                    line.erase(0, 1);
                    pos = line.find(']');
                    line.erase(pos, 1);
                    *(pTeam[count])++ = atof(line.data());
                }
                i++;
            }
            catch (exception &e) {
                cout << " yichang " << i << endl;
                return;
            }
        }
    } else {
        cout << "no such file" << filename << endl;
    }
}

//									w			  sc			 lc			ks				s		p
long initConvAndFc(struct Weight *weight, int schannel, int lchannel, int kersize, int stride, int pad) {
    weight->selfChannel = schannel;
    weight->lastChannel = lchannel;
    weight->kernelSize = kersize;
    weight->stride = stride;
    weight->pad = pad;
    weight->pbias = (mydataFmt *) malloc(schannel * sizeof(mydataFmt));
    if (weight->pbias == NULL)cout << "neicun muyou shenqing chengong!!";
    memset(weight->pbias, 0, schannel * sizeof(mydataFmt));
    long byteLenght = weight->selfChannel * weight->lastChannel * weight->kernelSize * weight->kernelSize;
    weight->pdata = (mydataFmt *) malloc(byteLenght * sizeof(mydataFmt));
    if (weight->pdata == NULL)cout << "neicun muyou shenqing chengong!!";
    memset(weight->pdata, 0, byteLenght * sizeof(mydataFmt));

    return byteLenght;
}

void initpRelu(struct pRelu *prelu, int width) {

    prelu->width = width;
    prelu->pdata = (mydataFmt *) malloc(width * sizeof(mydataFmt));
    if (prelu->pdata == NULL)cout << "prelu apply for memory failed!!!!";
    memset(prelu->pdata, 0, width * sizeof(mydataFmt));
}

void softmax(const struct pBox *pbox) {
    if (pbox->pdata == NULL) {
        cout << "the softmax's pdata is NULL , Please check !" << endl;
        return;
    }
    mydataFmt *p2D = pbox->pdata;
    mydataFmt *p3D = NULL;
    long mapSize = pbox->width * pbox->height;
    mydataFmt eleSum = 0;
    for (int row = 0; row < pbox->height; row++) {
        for (int col = 0; col < pbox->width; col++) {
            eleSum = 0;
            for (int channel = 0; channel < pbox->channel; channel++) {
                p3D = p2D + channel * mapSize;
                *p3D = exp(*p3D);
                eleSum += *p3D;
            }
            for (int channel = 0; channel < pbox->channel; channel++) {
                p3D = p2D + channel * mapSize;
                *p3D = (*p3D) / eleSum;
            }
            p2D++;
        }
    }
}

bool cmpScore(struct orderScore lsh, struct orderScore rsh) {
    if (lsh.score < rsh.score)
        return true;
    else
        return false;
}

void nms(vector<struct Bbox> &boundingBox_, vector<struct orderScore> &bboxScore_, const float overlap_threshold,
         string modelname) {
    if (boundingBox_.empty()) {
        return;
    }
    std::vector<int> heros;
    //sort the score
    sort(bboxScore_.begin(), bboxScore_.end(), cmpScore);

    int order = 0;
    float IOU = 0;
    float maxX = 0;
    float maxY = 0;
    float minX = 0;
    float minY = 0;
    while (bboxScore_.size() > 0) {
        order = bboxScore_.back().oriOrder;
        bboxScore_.pop_back();
        if (order < 0)continue;
        heros.push_back(order);
        boundingBox_.at(order).exist = false;//delete it

        for (int num = 0; num < boundingBox_.size(); num++) {
            if (boundingBox_.at(num).exist) {
                //the iou
                maxX = (boundingBox_.at(num).x1 > boundingBox_.at(order).x1) ? boundingBox_.at(num).x1
                                                                             : boundingBox_.at(order).x1;
                maxY = (boundingBox_.at(num).y1 > boundingBox_.at(order).y1) ? boundingBox_.at(num).y1
                                                                             : boundingBox_.at(order).y1;
                minX = (boundingBox_.at(num).x2 < boundingBox_.at(order).x2) ? boundingBox_.at(num).x2
                                                                             : boundingBox_.at(order).x2;
                minY = (boundingBox_.at(num).y2 < boundingBox_.at(order).y2) ? boundingBox_.at(num).y2
                                                                             : boundingBox_.at(order).y2;
                //maxX1 and maxY1 reuse 
                maxX = ((minX - maxX + 1) > 0) ? (minX - maxX + 1) : 0;
                maxY = ((minY - maxY + 1) > 0) ? (minY - maxY + 1) : 0;
                //IOU reuse for the area of two bbox
                IOU = maxX * maxY;
                if (!modelname.compare("Union"))
                    IOU = IOU / (boundingBox_.at(num).area + boundingBox_.at(order).area - IOU);
                else if (!modelname.compare("Min")) {
                    IOU = IOU / ((boundingBox_.at(num).area < boundingBox_.at(order).area) ? boundingBox_.at(num).area
                                                                                           : boundingBox_.at(
                                    order).area);
                }
                if (IOU > overlap_threshold) {
                    boundingBox_.at(num).exist = false;
                    for (vector<orderScore>::iterator it = bboxScore_.begin(); it != bboxScore_.end(); it++) {
                        if ((*it).oriOrder == num) {
                            (*it).oriOrder = -1;
                            break;
                        }
                    }
                }
            }
        }
    }
    for (int i = 0; i < heros.size(); i++)
        boundingBox_.at(heros.at(i)).exist = true;
}

void refineAndSquareBbox(vector<struct Bbox> &vecBbox, const int &height, const int &width) {
    if (vecBbox.empty()) {
        cout << "Bbox is empty!!" << endl;
        return;
    }
    float bbw = 0, bbh = 0, maxSide = 0;
    float h = 0, w = 0;
    float x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    for (vector<struct Bbox>::iterator it = vecBbox.begin(); it != vecBbox.end(); it++) {
        if ((*it).exist) {
            bbh = (*it).x2 - (*it).x1 + 1;
            bbw = (*it).y2 - (*it).y1 + 1;
            x1 = (*it).x1 + (*it).regreCoord[1] * bbh;
            y1 = (*it).y1 + (*it).regreCoord[0] * bbw;
            x2 = (*it).x2 + (*it).regreCoord[3] * bbh;
            y2 = (*it).y2 + (*it).regreCoord[2] * bbw;

            h = x2 - x1 + 1;
            w = y2 - y1 + 1;

            maxSide = (h > w) ? h : w;
            x1 = x1 + h * 0.5 - maxSide * 0.5;
            y1 = y1 + w * 0.5 - maxSide * 0.5;
            (*it).x2 = round(x1 + maxSide - 1);
            (*it).y2 = round(y1 + maxSide - 1);
            (*it).x1 = round(x1);
            (*it).y1 = round(y1);

            //boundary check
            if ((*it).x1 < 0)(*it).x1 = 0;
            if ((*it).y1 < 0)(*it).y1 = 0;
            if ((*it).x2 > height)(*it).x2 = height - 1;
            if ((*it).y2 > width)(*it).y2 = width - 1;

            it->area = (it->x2 - it->x1) * (it->y2 - it->y1);
        }
    }
}