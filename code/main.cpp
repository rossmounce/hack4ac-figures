#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <functional>

using namespace cv;
using namespace std;
using namespace std::placeholders;

template <class V, class T>
V filter (V vec, T comp){
	V tmp (vec.size());
	auto IT = copy_if (vec.begin(), vec.end(), tmp.begin()
			, comp
			);
	tmp.resize(distance(tmp.begin(),IT));
	return tmp;

}


// Vec4i [x1,y1,x2,y2]

double linelength(Vec4i l){
	return sqrt(pow(l[0]-l[2],2) + pow(l[1]-l[3],2));
}

bool isLonger (Vec4i i, Vec4i j) { return (linelength(i)>linelength(j)); }

bool ismoreleft(Vec4i i, Vec4i j) {
	if( i[0] + i[2] < j[0] + j[2]){
		return true;
	} else {
		return false;
	}
}

bool isTooCloseVertical(Vec4i i, Vec4i j, double dist){
//	cout << i[0] << " " << j[0] << endl;
	if (abs(double(i[0]) - double(j[0])) < dist){
		return true;
	} else {
		return false;
	}

}


Vec4i makeMinMax(Vec4i l){
	if(l[2] < l[0])
		swap(l[2],l[0]);

	if(l[3] < l[1])
		swap(l[3],l[1]);
	return l;
}

bool isInRegion(Vec4i l, double xmin, double ymin, double xmax, double ymax , double fuz){
//	cout << xmin << " "  << xmax  << " " << ymin  << " " << ymax  << endl;
//	cout << l[0] << " "  << l[2]  << " " << l[1]  << " " << l[3]  << endl;

	if( xmin - l[0] < fuz && xmin - l[2] < l[2]
	&&  fuz > l[0] - xmax && fuz > l[2] - xmax
	&&  ymin - l[1] < fuz && ymin - l[3] < fuz
	&&  fuz > l[1] - ymax && fuz > l[3] - ymax
	) {
		return true;
	} else {
		return false;
	}
}

bool isMainlyVertical(Vec4i l){
	if (abs(l[0]-l[2]) < abs(l[1]-l[3])){
		return true;
	} else {
		return false;
	}
}
bool isMainlyHorizontal(Vec4i l){ return !(isMainlyVertical(l));}

bool isStartingYAt(Vec4i l, double y, double fuzzy){
	if (abs(l[3] - y) < fuzzy){
		return true;
	} else { 
		return false;
	}
}

int main( int argc, char** argv )
{
	if( argc != 2) {
		cout <<" Usage: display_image ImageToLoadAndDisplay" << endl;
		return -1;
	}

	Mat src = imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Read the file

	if(! src.data ){                              // Check for invalid input
		cout <<  "Could not open or find the image" << endl ;
		return -1;
	}


	Mat dst, cdst;

	GaussianBlur(src, dst, Size( 3,3 ), 0, 0 );
	//adaptiveThreshold(dst, dst,255,CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY,75,10);  
 	cv::bitwise_not(dst, dst);
	Canny(dst, dst, 100, 200, 3);
	cvtColor(dst, cdst, CV_GRAY2BGR);


	// Find Lines
	vector<Vec4i> lines;
	HoughLinesP(dst, lines, 1, CV_PI/180, 50, 50, 10 );
	transform(lines.begin(),lines.end(), lines.begin(), makeMinMax);
	sort(lines.begin(), lines.end(), isLonger);
	for( size_t i = 0; i < lines.size(); i++ )
	{
		Vec4i l = lines[i];
//		cout << l << '\t' << linelength(l) <<   endl;
		line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,100), 1, CV_AA);
	}



	cout << "Number of Lines Detected " << lines.size() << endl;
 	
	// Find Axis
	Vec4i xaxis, yaxis; 
	
	for (size_t i=0, numvert=0, numhorz=0; numvert == 0 || numhorz == 0 ; i++){
		Vec4i l = lines[i];
	//	line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,255,255), 2, CV_AA);


		if(isMainlyVertical(l)){ // set x and y axis
			if (0 == numvert++)
				yaxis = (lines[i]);
		} else {
			if (0 == numhorz++)
				xaxis = (lines[i]);
		}
	}
//	cout << "xaxis" << xaxis << endl;
//	cout << "yaxis" << yaxis << endl;
	xaxis[0] = yaxis[0];
	yaxis[3] = xaxis[3];

	cout << "xaxis" << xaxis << endl;
	cout << "yaxis" << yaxis << endl;

	


	// Find Lines Within the Axes
	vector<Vec4i> linesWithinAxes = filter (lines
			, bind(isInRegion, _1,xaxis[0],yaxis[1],xaxis[2],yaxis[3], 10)
			      );
	cout << "Number of Lines Within Axes " << linesWithinAxes.size() << endl;


	for(Vec4i l : linesWithinAxes)
	{
		line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 2, CV_AA);
	}
	
	
	//Get the vertical lines
	vector<Vec4i> verticalLines = filter(linesWithinAxes, isMainlyVertical);


	// Filter out lines that are close
	vector <Vec4i> tmp;
	verticalLines.push_back(yaxis);
	for(size_t i = 0; i < verticalLines.size(); i++){
		bool tooclose = false;
		for(size_t j = i+1; j < verticalLines.size(); j++){
			if(isTooCloseVertical(verticalLines[i], verticalLines[j], 10)){
				verticalLines[j] = max(verticalLines[i],verticalLines[j]
						, isLonger);
				tooclose = true;
				break;
			}
		}

		if (!(tooclose)){
			tmp.push_back(verticalLines[i]);
		}
	}

	verticalLines=tmp;
	verticalLines.pop_back();

	sort(verticalLines.begin(), verticalLines.end(), ismoreleft);
	cout << "Number of Vertical Lines Within Axes " << verticalLines.size() << endl;
	for(Vec4i l : verticalLines)
	{
//		cout << l << endl;
		line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,255,0), 2, CV_AA);
	}
	
	vector <Vec4i> partsOfBars = filter(verticalLines
			, bind(isStartingYAt, _1, xaxis[3],10)

			);

	sort(partsOfBars.begin(), partsOfBars.end(), ismoreleft);

	cout << "Number of Parts of Bars: " << partsOfBars.size() << endl;

	for(Vec4i l : partsOfBars)
	{
//		cout << l << endl;
		line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255,0,0), 2, CV_AA);
	}


	// Assuming Bar Charts 
	cout << "The Bars:" << endl;
	cout << "num" << '\t' << "Rel" << '\t' << "*0.6" << '\t' << "*1.2" << '\t' << "*25" << endl;

	stringstream sstmp; 
	sstmp << argv[2] << " " << argv[3] << endl;
	double m, c;
	sstmp >> m >> c;
	for (size_t i = 0; i < partsOfBars.size(); i+=2){
		Vec4i l = partsOfBars[i];
		Vec4i r = partsOfBars[i+1];
		double y = abs(((l[1] +double(r[1]))/2 - xaxis[1]) / double (yaxis[1]-xaxis[1]));
		cout << i/2 << '\t' << y << '\t' << y*0.6<< '\t' << y *1.2 << '\t' << y * 25 << endl;
	}
	
	line(cdst, Point(xaxis[0], xaxis[1]), Point(xaxis[2], xaxis[3]), Scalar(0,255,255), 2, CV_AA);
	line(cdst, Point(yaxis[0], yaxis[1]), Point(yaxis[2], yaxis[3]), Scalar(0,255,255), 2, CV_AA);
	imshow("Display window", src);                   // Show our image inside it.
	imshow("Detected Lines", cdst);
	//resizeWindow("Display window",500,500);
	//resizeWindow("Detected Lines",500,500);
	cvNamedWindow("Display window",CV_WINDOW_AUTOSIZE);
	while(cv::waitKey(1) != 27);

	return 0;
}
