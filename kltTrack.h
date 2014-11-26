#ifndef _KLT_TRACK_H
#define _KLT_TRACK_H


struct track{
	int trackId;
	int startFrm, endFrm;
	CvPoint startPos, endPos;
};

int kltTrack(std::string vidPath, int startFrm, int endFrm, std::string trksPath);

#endif
