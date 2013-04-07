/**
 * Copyright (c) 2013-2013 Stardrad Yin
 * email:yin8086+support@gmail.com
 *
 * Report bugs and download new versions at https://github.com/yin8086/UnityImageIn
 *
 * This software is distributed under the MIT License.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <QApplication>
#include <QtCore>
#include <QImage>
#include <QDataStream>
#include <QFile>
#include <QRunnable>
#include <QFileDialog>

#include "pvrtc_dll.h"

QPair<int, QString> getType(const QString& fName) {
    //QString binName=fName.left(fName.lastIndexOf("_"))+".bin";
    QString testStr=fName.left(fName.length() - 4);
    QString binName = testStr.left(testStr.lastIndexOf("."));
    QString typeStr=testStr.right(testStr.length() - binName.length() - 1);

    int type=1;
    if(typeStr == "Alpha8") {
        type = 0;
    }
    else if(typeStr == "rgba4444") {
        type = 1;
    }
    else if(typeStr == "argb1555") {
        type = 2;
    }
    else if(typeStr == "rgb565") {
        type = 3;
    }
    else if(typeStr == "rgb888") {
        type = 4;
    }
    else if(typeStr == "rgba8888") {
        type = 5;
    }
    else if(typeStr == "PVRTC4") {
        type = 6;
    }
    else if(typeStr == "argb8888") {
        type = 7;
    }
    else if(typeStr == "argb4444") {
        type = 8;
    }
    return qMakePair(type, binName);
}

void convert(const uchar* src, char* dest,
             quint32 dataSize, int pSize,
             int type) {
    if(pSize == 1 && type == 0) {
        //Alpha8
        quint32 i, j;
        for(i=0, j=0; i < dataSize; i+=pSize, j+=4) {
            dest[i] = src[j+3];
        }
    }
    else if(pSize == 2) {
        //2bpp
        quint32 i, j;
        for(i=0, j=0; i < dataSize; i+=pSize, j+=4) {
            quint16 pixelVal = 0;
            quint16 r,g,b,a;
            if(type == 1) {
                //rgba4444
                r = (src[j+2]   * 15 + 127) / 255;
                g = (src[j+1] * 15 + 127) / 255;
                b = (src[j]  * 15 + 127) / 255;
                a = (src[j+3] * 15 + 127) / 255;
                pixelVal=(r<<12)|(g<<8)|(b<<4)|a;
            }
            else if(type == 2) {
                //argb1555
                r = (src[j+2]   * 31 + 127) / 255;
                g = (src[j+1] * 31 + 127) / 255;
                b = (src[j]  * 31 + 127) / 255;
                a = (src[j+3]/0xff);
                pixelVal=(a<<15)|(r<<10)|(g<<5)|b;

            }
            else if(type == 3) {
                //rgb565
                r = (src[j+2]   * (31*2) + 255) / (255*2);
                g = (src[j+1] * 63 + 127) / 255;
                b = (src[j]  * 31 + 127) / 255;
                pixelVal = (r << 11) | (g <<  5) | b;
            }
            else if(type == 8) {
                //argb4444
                r = (src[j+2]   * 15 + 127) / 255;
                g = (src[j+1] * 15 + 127) / 255;
                b = (src[j]  * 15 + 127) / 255;
                a = (src[j+3] * 15 + 127) / 255;
                pixelVal = (a << 12) | (r << 8) | (g << 4) | b;
            }
            *(quint16*)(dest+i) = pixelVal;
        }
    }
    else if(pSize == 3 && type == 4) {
        //rgb888
        quint32 i, j;
        for(i=0, j=0; i < dataSize; i+=pSize, j+=4) {
            dest[i]     = src[j+2];
            dest[i+1]   = src[j+1];
            dest[i+2]   = src[j];
        }
    }
    else if(pSize == 4) {
        //rgba8888
        quint32 i, j;
        for(i=0, j=0; i < dataSize; i+=pSize, j+=4) {
            dest[i]     = src[j+2];
            dest[i+1]   = src[j+1];
            dest[i+2]   = src[j];
            dest[i+3]   = src[j+3];
        }
    }
    else if(pSize == 5 && type == 7) {
        //argb8888
        quint32 i, j;
        pSize = 4;
        for(i=0, j=0; i < dataSize; i+=pSize, j+=4) {
            dest[i]     = src[j+3];
            dest[i+1]   = src[j+2];
            dest[i+2]   = src[j+1];
            dest[i+3]   = src[j];
        }
    }
}

void pngParse(const QString& fName) {
    QPair<int, QString> typeName=getType(fName);
    if(!QFile::exists(fName))
    {
        printf("Image doesn't exists!\n");
    }
    else if(!QFile::exists(typeName.second)) {
        printf("InputFile doesn't exists!\n");
    }
    else {
        QFile destf(typeName.second);
        QImage im(fName);
        if(destf.open(QIODevice::ReadWrite) || !im.isNull()) {
            QDataStream br(&destf);
            br.setByteOrder(QDataStream::LittleEndian);
            quint32 len;
            br>>len;
            destf.seek(destf.pos()+len);
            if(destf.pos()%4 != 0) {
                destf.seek((destf.pos()/4+1)*4);
            }
            quint32 width,height;
            quint32 imageDataSize;
            quint32 pixelSize;
            br>>width>>height>>imageDataSize>>pixelSize;

            //quint32 testSize=destf.pos()+0x28+imageDataSize;
            //if(destf.size()-4 <= testSize && testSize <= destf.size() &&
            if(destf.size() > imageDataSize + 20 &&  imageDataSize > 0 &&
                    width == im.width() &&
                    height == im.height()) {
                if ( (1 <= pixelSize && pixelSize <=7 && pixelSize != 6) ||
                        ( pixelSize == 0x20 ||pixelSize == 0x21) ){
                    if (pixelSize == 7) {
                        pixelSize = 2;
                    }
                    im=im.mirrored(false,true)/*.rgbSwapped()*/;
                    quint32 imageSize = width*height*pixelSize;
                    if (pixelSize ==0x20 || pixelSize == 0x21) {
                        imageSize = width*height/2;
                    }
                    else if(pixelSize == 5) {
                        imageSize = width*height*4;
                    }
                    uchar* pixelTable=im.bits();
                    char* tarTable=new char[imageSize];
                    if (pixelSize ==0x20 || pixelSize == 0x21) {
                        pvrtc_compress(pixelTable, tarTable, width, height, 0, 1, 1, 0);
                    }
                    else {
                        convert(pixelTable, tarTable, imageSize, pixelSize, typeName.first);
                    }
                    //destf.seek(destf.pos()+0x28);
                    destf.seek(destf.size() - imageDataSize);

                    br.writeRawData(tarTable,imageSize);
                    if(imageSize != imageDataSize) {
                        while(width/2 >=1 && height/2 >=1) {
                            width /= 2;
                            height /= 2;
                            im = im.scaledToWidth(width,Qt::SmoothTransformation);
                            pixelTable = im.bits();
                            imageSize /= 4;
                            if (pixelSize ==0x20 || pixelSize == 0x21) {
                                pvrtc_compress(pixelTable, tarTable, width, height, 0, 1, 1, 0);
                            }
                            else {
                                convert(pixelTable, tarTable, imageSize, pixelSize, typeName.first);
                            }
                            br.writeRawData(tarTable,imageSize);
                        }
                    }
                    printf(QObject::tr("%1 Completed!\n").arg(destf.fileName())
                           .toLatin1().data());
                    delete [] tarTable;
                }
                else {
                    printf("%s Unknown format!\n",
                           destf.fileName().toLatin1().data());
                }
            }
            else {
                printf("%s Not an image!\n",
                       destf.fileName().toLatin1().data());
            }
            destf.close();
        }
    }
}

class MyRun : public QRunnable {
    QString fName;
    int type;
public:
    MyRun(const QString& fn, int t):QRunnable(),fName(fn),type(t) {}
    void run();
};
void MyRun::run() {
    pngParse(fName);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    printf("Work in 2 modes:\n");
    printf("1. UnityImageIn.exe\n"
           "\tUse file dialog to select multi pictures\n");

    printf("2. UnityImageIn.exe *.png\n"
           "\tTransform *.png\n");

    if (argc == 2) {
        pngParse(argv[1]);
    }
    else {

        QStringList files = QFileDialog::getOpenFileNames(
                                0,
                                QObject::tr("Select one or more files to open"),
                                QDir::current().absolutePath(),
                                QObject::tr("All files (*.png)"));

        QList<QRunnable *> runList;

        foreach(const QString &fn, files) {
            MyRun* tmpR=new MyRun(fn,0);
            tmpR->setAutoDelete(true);
            QThreadPool::globalInstance()->start(tmpR);
            runList.append(tmpR);
        }

    }
    return 0;
}
