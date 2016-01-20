/////////////////////////////////////////////////////////////////////////
//
// webcam.cpp --a part of libdecodeqr
//
// Copyright(C) 2007 NISHI Takao <zophos@koka-in.org>
//                   JMA  (Japan Medical Association)
//                   NaCl (Network Applied Communication Laboratory Ltd.)
//
// This is free software with ABSOLUTELY NO WARRANTY.
// You can redistribute and/or modify it under the terms of LGPL.
//
// $Id$
//
#include <stdio.h>
#include <string.h>
#include <highgui.h>
#include "decodeqr.h"
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

int usage(char *program_name);

int thresh = 50;
const char* wndname = "src";

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2 
double angle( CvPoint* pt1, CvPoint* pt2, CvPoint* pt0 )
{
    double dx1 = pt1->x - pt0->x;
    double dy1 = pt1->y - pt0->y;
    double dx2 = pt2->x - pt0->x;
    double dy2 = pt2->y - pt0->y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

double veclen(CvPoint* pt1, CvPoint* pt2)
{
    double dx1 = pt1->x - pt2->x;
    double dy1 = pt1->y - pt2->y;
    return sqrt((dx1*dx1)+(dy1*dy1));
}


int main(int argc,char *argv[])
{

    char found=0;
    int show_bin_image=0;
    char *str=(char*)malloc(10);
    unsigned char *text=NULL;
    int text_size=0;
    CvSeq* result;
//    if(argc>1){
//        if(strcmp(argv[1],"-d"))
//            return(usage(argv[0]));
//        else
            show_bin_image=1;
//    }


//    CvCapture *capture=cvCaptureFromFile("20120824_161616.mp4");
//    CvCapture *capture=cvCaptureFromFile("20120824_165616.mp4");
//    CvCapture *capture=cvCaptureFromFile("rtsp://192.168.1.101:554/ch0_0.h264");
    CvCapture *capture=cvCaptureFromCAM(0);
    if(!capture)
        return(-1);

    static CvScalar red_color[] ={0,0,255};


    cvNamedWindow("src",1);

    cvNamedWindow("bin",1);

    int key=-1;

    IplImage *camera=cvQueryFrame(capture);
    IplImage *src=NULL,*bin,  *dst_img = cvCreateImage(cvGetSize(camera),IPL_DEPTH_8U,1),*qrcode_img;
    IplImage *blur_frame=cvCloneImage(camera);
    CvMat* prevgray = 0, *gray =0;
    IplImage *img = cvCreateImage(cvGetSize(camera),IPL_DEPTH_8U,1);
    bin=cvCloneImage(camera);
    CvMemStorage *storage = cvCreateMemStorage (0);
    CvSeq *contours = 0;
    CvBox2D rect;
    CvMemStorage* MemStorage = cvCreateMemStorage(0);
    CvSeq * filtered_seq = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq),sizeof(CvPoint), MemStorage);
    CvPoint* pt4= new CvPoint();
    CvPoint* pt5= new CvPoint();
    int sz;
    int radius = 1;
    double ang;
    while(key<=0){

        camera=cvQueryFrame(capture);
        if(!camera)
            break;

        cvCvtColor(camera, img, CV_BGR2GRAY);
        cvThreshold(img, dst_img, 100, 255, CV_THRESH_BINARY);
        int contours_count=cvFindContours (dst_img, storage, &contours, 
        sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE 
                    ,cvPoint(0,0));
        int filtered=0,minx=2000,miny=2000,maxx=0,maxy=0;

        CvFont font;
        cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX_SMALL, 1.0, 1.0,0,1,8);

         for(CvSeq* seq0 = contours;seq0!=0;seq0 = seq0->h_next)
            {
            CvSlice slice=CV_WHOLE_SEQ;
            double area = fabs(cvContourArea(seq0));
            double perim = cvContourPerimeter(seq0);
//cvDrawContours(camera, seq0, CV_RGB(255,0,0), CV_RGB(0,0,255), 0, 1, 8); // рисуем контур
            if (area>=10 && area<=3000)
                {
                rect=cvMinAreaRect2 (seq0,storage);
                if (rect.size.width>=6 && rect.size.height>=6 && rect.size.width<=300 && rect.size.height<=300)
                    {
                    result = cvApproxPoly( seq0, sizeof(CvContour), storage,
                    CV_POLY_APPROX_DP, cvContourPerimeter(seq0)*0.02, 0 );
                    if (result->total==4)
                        {
                        float af=rect.size.width/rect.size.height;
                        if (0.75<af && af<1.25 && area/perim>=3)
                            {
                            filtered++;
                            cvDrawContours(camera, seq0, CV_RGB(255,0,0), CV_RGB(0,0,255), 0, 1, 8,cvPoint(0,0)); // рисуем контур
                            CvPoint pt=cvPoint(cvRound(rect.center.x),cvRound(rect.center.y));
                            cvSeqPush(filtered_seq, &pt);
                            }
                        }
                   }
               }
            }

/// После всех фильров должно остаться несколько точек, указывающищ на квадратные контуры, пусть будет от 3 до 40
if (filtered>=4 && filtered<=40)
    {
found=0;
    CvSeq* s1,*s2,*s3;
    for(int i=0;i<=filtered_seq->total;i++)
        {
        for(int j=0;j<=filtered_seq->total;j++)
            {
            for(int k=0;k<=filtered_seq->total;k++)
                {
                if (k==j || k==i || j==i) continue;
                CvPoint* pt1 = (CvPoint*)cvGetSeqElem( filtered_seq, i );
                CvPoint* pt2 = (CvPoint*)cvGetSeqElem( filtered_seq, j );
                CvPoint* pt3 = (CvPoint*)cvGetSeqElem( filtered_seq, k );

                if ((pt1->x==pt2->x)&&(pt1->y==pt2->y)) continue;
                if ((pt1->x==pt3->x)&&(pt1->y==pt3->y)) continue;
                if ((pt2->x==pt3->x)&&(pt2->y==pt3->y)) continue;
                // Посчитаем угол между точками
                ang=angle(pt2,pt3,pt1);
                // Если угол около 90гр то это похоже на qr
                if (ang<0.15 && ang>-0.15)
                    {
                        int l1=(int)veclen(pt1,pt2);
                        int l2=(int)veclen(pt1,pt3);
                        if (l1-13<=l2 && l1+13>=l2)
                         {


//    cout<<"draw"<<endl;

//                    cvLine(camera,cvPoint(pt1->x,pt1->y),cvPoint(pt2->x,pt2->y),CV_RGB(0,255,255),1,8,0);
//                    cvLine(camera,cvPoint(pt1->x,pt1->y),cvPoint(pt3->x,pt3->y),CV_RGB(255,255,0),1,8,0);

                    //int x4,y4;
                    CvPoint* pt0= new CvPoint();
//                    CvPoint* pt5= new CvPoint();
                    int bx=pt1->x-pt3->x;
                    int by=pt1->y-pt3->y;
                    int dx=pt1->x-pt2->x;
                    int dy=pt1->y-pt2->y;
                    pt4->x=pt1->x-dx-bx;
                    pt4->y=pt1->y-dy-by;

                    pt5->x=pt1->x-dx/2-bx/2;
                    pt5->y=pt1->y-dy/2-by/2;
                    minx=pt5->x-sz;
                    miny=pt5->y-sz;
                    maxx=pt5->x+sz;
                    maxy=pt5->y+sz;

                    pt0->x=minx;
                    pt0->y=miny;


                    sz=veclen(pt5,pt1)*1.5;
                    ang=angle(pt1,pt0,pt5)*90;
//		    sprintf(str,"%d",(int)ang);
//		    cvPutText(camera,str,cvPoint(30,30),&font,CV_RGB(255,0,0));
                    cvLine(camera,cvPoint(pt0->x,pt0->y),cvPoint(pt5->x,pt5->y),CV_RGB(255,255,255),1,8,0);
                    cvLine(camera,cvPoint(pt1->x,pt1->y),cvPoint(pt5->x,pt5->y),CV_RGB(255,255,255),1,8,0);
//ut<<"ang="<<ang<<endl;
//
//                  if (pt5->y>pt1->y && pt5->x<pt1->x) {ang=-45-ang;cout<<" 1";}
//                  if (pt5->y>pt1->y && pt5->x>pt1->x) {ang=ang-63;cout<<" 2";}
//                  if (pt5->y<pt1->y && pt5->x<pt1->x) {ang=45+ang;cout<<" 3";}
//                  if (pt5->y<pt1->y && pt5->x>pt1->x) {ang=45-ang;cout<<" 4";}




//                        cvEllipse(camera,
//                              cvPoint(pt5->x,pt5->y),
//                              cvSize(sz,sz),
//                              0,
//                              0,360,
//                              CV_RGB(0,255,0),2);



//                    cvLine(camera,cvPoint(pt4->x,pt4->y),cvPoint(pt2->x,pt2->y),CV_RGB(0,0,255),1,8,0);
//                    cvLine(camera,cvPoint(pt4->x,pt4->y),cvPoint(pt3->x,pt3->y),CV_RGB(255,0,0),1,8,0);


//                    cvLine(camera,cvPoint(pt1->x,pt1->y),cvPoint(pt0->x,pt0->y),CV_RGB(255,255,0),1,8);

                    found=1;
                        }
                    }

                if (found==1) break;
                }
            if (found==1) break;
            }
        if (found==1) break;
      }
cvClearSeq(filtered_seq);
}

if (found)
{
found=0;

if (camera->width>=maxx && camera->height>=maxy && minx>=0 && miny>=0 && maxx-minx>=20 && maxy-miny>=20)
{

    cvRectangle(camera,cvPoint(minx,miny),cvPoint(maxx,maxy),CV_RGB(255,0,0),3,8,0);
    cvSetImageROI(camera, cvRect(minx,miny,maxx-minx,maxy-miny));
    qrcode_img=cvCreateImage(cvGetSize(camera), camera->depth, camera->nChannels);
    cvCopy(camera, qrcode_img,NULL);
    Point2f src_center(qrcode_img->width/2,qrcode_img->height/2);
    CvMat *rot_mat = cvCreateMat( 2, 3, CV_32FC1 );
// getRotationMatrix2D(src_center, ang, 1.0);
    cv2DRotationMatrix(src_center, ang, 1.0, rot_mat);
// CvMat rot=imgMat(qrcode_img);
    IplImage *dst = cvCreateImage(cvGetSize(camera), camera->depth,camera->nChannels);;
    cvWarpAffine(qrcode_img, dst, rot_mat,  CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS, cvScalarAll(0));
    cvResetImageROI(camera);
    cvCopy(dst,qrcode_img);
    
//    cout<<"fitered="<<filtered<<endl;
            cvShowImage("bin",qrcode_img);

/*        QrDecoderHandle decoder=qr_decoder_open();
        printf("libdecodeqr version %s\n",qr_decoder_version());

        qr_decoder_set_image_buffer(decoder,qrcode_img);

        if (!qr_decoder_is_busy(decoder))
        {
            short sz,stat;
            sz=25,stat=0;
            for(sz=25,stat=0;
                (sz>=3)&&((stat&QR_IMAGEREADER_DECODED)==0);
                sz-=2)
                stat=qr_decoder_decode(decoder,sz);

            QrCodeHeader header;
            if(qr_decoder_get_header(decoder,&header))
                {
                if(text_size<header.byte_size+1)
                    {
                        if(text) delete [] text;
                        text_size=header.byte_size+1;
                        text=new unsigned char[text_size];
                    }
                    qr_decoder_get_body(decoder,text,text_size);
                    printf("stat=%d text=%s\n",(stat&QR_IMAGEREADER_DECODED),text);
                /// вывод дебага от libdecodeqr
                    int i;
                    CvBox2D *boxes=qr_decoder_get_finderpattern_boxes(decoder);
                    for(i=0;i<3;i++)
                        {
                        CvSize sz=cvSize((int)boxes[i].size.width/2,
                                     (int)boxes[i].size.height/2);
                        cvEllipse(qrcode_img,
                              cvPointFrom32f(boxes[i].center),
                              sz,
                              boxes[i].angle,
                              0,360,
                              CV_RGB(0,255,0),2);
                        }
                   cvDrawContours (img, contours, CV_RGB(255,0,0), cvScalarAll(255),1,1,8,cvPoint(0,0));

                }
        }
qr_decoder_close(decoder);    
*/
//                cvShowImage("bin",qrcode_img);
}
} //found


                cvShowImage("src",camera);

                key=cvWaitKey(1);

}

//while (key!=27)
//{
//key=cvWaitKey(100);
//}
    if(bin)
        cvReleaseImage(&bin);
    if(src)
        cvReleaseImage(&src);
    if(dst_img)
        cvReleaseImage(&dst_img);

        
    cvReleaseCapture(&capture);

    return(0);
}

int usage(char *program_name)
{
    fprintf(stderr,"usage: %s [-d|-h]\n",program_name);
    fprintf(stderr,"-d\tturn on debug mode.\n");
    fprintf(stderr,"-h\tshow thismessage and quit.\n\n");
    
    return(-1);
}
