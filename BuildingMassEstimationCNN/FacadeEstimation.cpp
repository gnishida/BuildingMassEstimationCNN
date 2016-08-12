#include "FacadeEstimation.h"
#include <iostream>
#include <QString>
#include <time.h>
#include "HoughTransform.h"

namespace fe {

	void adjustFacadeImage(cv::Mat& img) {
		ht::warpImageByDominantOrientation(img);
	}
	
}
