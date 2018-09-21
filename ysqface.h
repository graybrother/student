#ifndef YSQFACE_H
#define YSQFACE_H

#include <QtCore/qglobal.h>

#if defined(YSQFACE_LIBRARY)
#  define YSQFACESHARED_EXPORT Q_DECL_EXPORT
#else
#  define YSQFACESHARED_EXPORT Q_DECL_IMPORT
#endif

YSQFACESHARED_EXPORT int * facedetect_frontal(unsigned char * result_buffer, //buffer memory for storing face detection results, !!its size must be 0x20000 Bytes!!
                               unsigned char * gray_image_data, int width, int height, int step, //input image, it must be gray (single-channel) image!
                               float scale, //scale factor for scan windows
                               int min_neighbors, //how many neighbors each candidate rectangle should have to retain it
                               int min_object_width, //Minimum possible face size. Faces smaller than that are ignored.
                               int max_object_width = 0, //Maximum possible face size. Faces larger than that are ignored. It is the largest posible when max_object_width=0.
                               int doLandmark=0); // landmark detection

YSQFACESHARED_EXPORT int * facedetect_frontal_surveillance(unsigned char * result_buffer, //buffer memory for storing face detection results, !!its size must be 0x20000 Bytes!!
                               unsigned char * gray_image_data, int width, int height, int step, //input image, it must be gray (single-channel) image!
                               float scale, //scale factor for scan windows
                               int min_neighbors, //how many neighbors each candidate rectangle should have to retain it
                               int min_object_width, //Minimum possible face size. Faces smaller than that are ignored.
                               int max_object_width = 0, //Maximum possible face size. Faces larger than that are ignored. It is the largest posible when max_object_width=0.
                               int doLandmark = 0); // landmark detection

YSQFACESHARED_EXPORT int * facedetect_multiview(unsigned char * result_buffer, //buffer memory for storing face detection results, !!its size must be 0x20000 Bytes!!
                               unsigned char * gray_image_data, int width, int height, int step, //input image, it must be gray (single-channel) image!
                               float scale, //scale factor for scan windows
                               int min_neighbors, //how many neighbors each candidate rectangle should have to retain it
                               int min_object_width, //Minimum possible face size. Faces smaller than that are ignored.
                               int max_object_width = 0, //Maximum possible face size. Faces larger than that are ignored. It is the largest posible when max_object_width=0.
                               int doLandmark = 0); // landmark detection

YSQFACESHARED_EXPORT int * facedetect_multiview_reinforce(unsigned char * result_buffer, //buffer memory for storing face detection results, !!its size must be 0x20000 Bytes!!
                               unsigned char * gray_image_data, int width, int height, int step, //input image, it must be gray (single-channel) image!
                               float scale, //scale factor for scan windows
                               int min_neighbors, //how many neighbors each candidate rectangle should have to retain it
                               int min_object_width, //Minimum possible face size. Faces smaller than that are ignored.
                               int max_object_width = 0, //Maximum possible face size. Faces larger than that are ignored. It is the largest posible when max_object_width=0.
                               int doLandmark = 0); // landmark detection


#endif // YSQFACE_H
