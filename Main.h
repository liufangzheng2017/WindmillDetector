#pragma once

#include <chrono>
#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "WindmillDetector.h"

enum HierarchyValue {
	Next = 0,
	Prev = 1,
	Child = 2,
	Parent = 3
};
enum HSVValue {
	H = 0,
	S = 1,
	V = 2
};
