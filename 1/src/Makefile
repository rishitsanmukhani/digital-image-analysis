CC = g++ -O2 -std=c++11 -g

LIBRARY_PATHS = -L"D:\OpenCVMinGW\x64\bin"

INCLUDE_PATHS = -I"D:\opencv\build\include"

LIBS = -lopencv_calib3d300 -lopencv_core300 -lopencv_features2d300 -lopencv_flann300 -lopencv_highgui300 -lopencv_imgproc300 -lopencv_ml300 -lopencv_objdetect300 -lopencv_photo300 -lopencv_stitching300 -lopencv_video300 -lopencv_videostab300 -lopencv_imgcodecs300

median_cut:
	$(CC) $(LIBRARY_PATHS) $(INCLUDE_PATHS) $(LIBS) median_cut.cpp -o median_cut

emboss:
	$(CC) $(LIBRARY_PATHS) $(INCLUDE_PATHS) $(LIBS) emboss.cpp -o emboss

segment:
	$(CC) $(LIBRARY_PATHS) $(INCLUDE_PATHS) $(LIBS) segment.cpp -o segment	

dog:
	$(CC) $(LIBRARY_PATHS) $(INCLUDE_PATHS) $(LIBS) dog.cpp -o dog	
