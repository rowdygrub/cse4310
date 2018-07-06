//
//    Copyright 2018 Christopher D. McMurrough
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

/*******************************************************************************************************************//**
 * @file cv_ellipse.cpp
 * @brief C++ example of Canny edge detection and ellipse model fitting in OpenCV
 * @author Christopher D. McMurrough
 **********************************************************************************************************************/

// include necessary dependencies
#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"

// configuration parameters
#define NUM_COMNMAND_LINE_ARGUMENTS 1

/*******************************************************************************************************************//**
 * @brief program entry point
 * @param[in] argc number of command line arguments
 * @param[in] argv string array of command line arguments
 * @return return code (0 for normal termination)
 * @author Christoper D. McMurrough
 **********************************************************************************************************************/
int main(int argc, char **argv)
{
    cv::Mat imageIn;

    // validate and parse the command line arguments
    if(argc != NUM_COMNMAND_LINE_ARGUMENTS + 1)
    {
        std::printf("USAGE: %s <image_path> \n", argv[0]);
        return 0;
    }
    else
    {
        imageIn = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);

        // check for file error
        if(!imageIn.data)
        {
            std::cout << "Error while opening file " << argv[1] << std::endl;
            return 0;
        }
    }

    // get the image size
    std::cout << "image width: " << imageIn.size().width << std::endl;
    std::cout << "image height: " << imageIn.size().height << std::endl;
    std::cout << "image channels: " << imageIn.channels() << std::endl;

    // convert the image to grayscale
    cv::Mat imageGray;
    cv::cvtColor(imageIn, imageGray, cv::COLOR_BGR2GRAY);

    cv::Mat imageThresh;
    cv::threshold(imageGray,imageThresh,75,256,0);

    cv::Mat morphKernel = getStructuringElement(cv::MORPH_RECT, cv::Size(1, 1));
    cv::Mat imageDilate;
    cv::dilate(imageThresh, imageDilate, morphKernel, cv::Point(-1, -1), 1);


    morphKernel = getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
    cv::Mat imageErode;
    cv::erode(imageDilate,imageErode,morphKernel,cv::Point(-1,-1),2);

    cv::Mat imageBlur;
    cv::blur(imageErode, imageBlur, cv::Size(1,1));



    // find the image edges
    cv::Mat imageEdges;
    const double cannyThreshold1 = 100;
    const double cannyThreshold2 = 300;
    const int cannyAperture = 3;
    cv::Canny(imageBlur, imageEdges, cannyThreshold1, cannyThreshold2, cannyAperture);

    // locate the image contours (after applying a threshold or canny)
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(imageEdges, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    // draw the contours
    cv::Mat imageContours = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
    cv::RNG rand(12345);
    for(int i = 0; i < contours.size(); i++)
    {
        cv::Scalar color = cv::Scalar(rand.uniform(0, 256), rand.uniform(0,256), rand.uniform(0,256));
        cv::drawContours(imageContours, contours, i, color);
    }

    // compute minimum area bounding rectangles
    std::vector<cv::RotatedRect> minAreaRectangles(contours.size());
    for(int i = 0; i < contours.size(); i++)
    {
        // compute a minimum area bounding rectangle for the contour
        minAreaRectangles[i] = cv::minAreaRect(contours[i]);
    }

    // draw the rectangles
    cv::Mat imageRectangles = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
    for(int i = 0; i < contours.size(); i++)
    {
        cv::Scalar color = cv::Scalar(rand.uniform(0, 256), rand.uniform(0,256), rand.uniform(0,256));
        cv::Point2f rectanglePoints[4];
        minAreaRectangles[i].points(rectanglePoints);
        for(int j = 0; j < 4; j++)
        {
            cv::line(imageRectangles, rectanglePoints[j], rectanglePoints[(j+1) % 4], color);
        }
    }

    // fit ellipses to contours containing sufficient inliers
    std::vector<cv::RotatedRect> fittedEllipses(contours.size());
    for(int i = 0; i < contours.size(); i++)
    {
        // compute an ellipse only if the contour has more than 5 points (the minimum for ellipse fitting)
        if(contours.at(i).size() > 5)
        {
            fittedEllipses[i] = cv::fitEllipse(contours[i]);
        }
    }

    // draw the ellipses
    cv::Mat imageEllipse = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
    const int minEllipseInliers = 150;
    for(int i = 0; i < contours.size(); i++)
    {
        // draw any ellipse with sufficient inliers
        if(contours.at(i).size() > minEllipseInliers && fittedEllipses[i].size.width >= 100)
        {
            cv::Scalar color = cv::Scalar(rand.uniform(0, 256), rand.uniform(0,256), rand.uniform(0,256));
            cv::ellipse(imageIn, fittedEllipses[i], color, 2);
            std::cout << fittedEllipses[i].size << std::endl;
            //std::cout << fittedEllipses[i] << std::end;
            cv::imshow("imageIn", imageIn);
            cv::waitKey();
        }
    }

    // display the images
    cv::imshow("imageIn", imageIn);
    // cv::imshow("imageGray", imageGray);

    //cv::imshow("imageDilate",imageDilate);
    //cv::imshow("imageErode",imageErode);
    cv::imshow("imageThresh",imageThresh);
    cv::imshow("imageBlur",imageBlur);
    //cv::imshow("imageEdges", imageEdges);
    //cv::imshow("imageContours", imageContours);
    cv::imshow("imageRectangles", imageRectangles);
    // cv::imshow("imageIn", imageEllipse);
    cv::waitKey();
}
