CC = g++ -std=c++11

LIBRARY_PATHS = -L "C:\opencv\build\include\bin" 

INCLUDE_PATHS = -I "C:\opencv\build\include"

LIBS = -lopencv_calib3d300 -lopencv_core300 -lopencv_features2d300 -lopencv_flann300 -lopencv_highgui300 -lopencv_imgproc300 -lopencv_ml300 -lopencv_objdetect300 -lopencv_photo300 -lopencv_stitching300 -lopencv_video300 -lopencv_videostab300 -lopencv_imgcodecs300

OBJS = main.cpp

all: $(OBJS)
	$(CC) $(LIBRARY_PATHS) $(INCLUDE_PATHS) $(LIBS) $(OBJS) -o img

run:
	img

