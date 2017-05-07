#include <map>
#include "sickio.h"

using namespace SickToolbox;

Wrapper SickFrame::scanCounterWrapper("scanCounter",0xffff);
Wrapper SickFrame::scanTimeWrapper("scanCounter",0xffffffff);
				      
SickFrame::SickFrame() {
    // Will be filled in by read()
}

// Peek into a file to determine its version
// Assumes fd is positioned at beginning
// Leaves the file positioned at the first record
int SickFrame::getFileVersion(FILE *fd) {
    char line[100];
    int version;
    if (fgets(line,100,fd) == NULL) {
	std::cerr << "Failed to read header line from file" << std::endl;
	exit(1);
    }
    if (strncmp(line,"FEREC-",6)==0) {
	// found header
	version=atoi(&line[6]);
    } else {
	// version 1, no header
	version=1;
	rewind(fd);
    }
    if (strlen(line)>0)
	line[strlen(line)-1] = 0;  // Remove newline
    dbg("SickFrame.getFileVersion",1) << "File version " << version <<  " with first line: <" << line << ">" << std::endl;
    assert(version>=1 && version<=2);
    return version;
}

void SickFrame::read(SickToolbox::SickLMS5xx *sick_lms_5xx, int _nechoes, bool captureRSSI) {
    try {
	//unsigned int range_2_vals[SickLMS5xx::SICK_LMS_5XX_MAX_NUM_MEASUREMENTS];
	//sick_lms_5xx.SetSickScanFreqAndRes(SickLMS5xx::SICK_LMS_5XX_SCAN_FREQ_25,
	//SickLMS5xx::SICK_LMS_5XX_SCAN_RES_25);
	//sick_lms_5xx.SetSickScanDataFormat(SickLMS5xx::SICK_LMS_5XX_DIST_DOUBLE_PULSE,
	//				         SickLMS5xx::SICK_LMS_5XX_REFLECT_NONE);
	nechoes=_nechoes;
	assert(nechoes>=1 && nechoes<=MAXECHOES);
	if (sick_lms_5xx == NULL) {
	    // Fake a measurement
	    num_measurements=571;
	    for (int i=0;i<(int)num_measurements;i++) {
		for (int e=0;e<nechoes;e++) {
		    range[e][i]=i+e*100;
		    reflect[e][i]=100/(e+1);
		}
	    }
	    usleep(1000000/50);
	} else  {
	    const int NSTATUS=20;
	    unsigned int s[NSTATUS];
	    sick_lms_5xx->GetSickMeasurements(
					      range[0],
					      (nechoes>=2)?range[1]:NULL,
					      (nechoes>=3)?range[2]:NULL,
					      (nechoes>=4)?range[3]:NULL,
					      (nechoes>=5)?range[4]:NULL,
					      captureRSSI?reflect[0]:NULL,
					      (captureRSSI&&nechoes>=2)?reflect[1]:NULL,
					      (captureRSSI&&nechoes>=3)?reflect[2]:NULL,
					      (captureRSSI&&nechoes>=4)?reflect[3]:NULL,
					      (captureRSSI&&nechoes>=5)?reflect[4]:NULL,
					      num_measurements,
					      &s[0],NSTATUS);
	    //std::cout << "status=";
	    //for (int i=0;i<NSTATUS;i++)
	    //std::cout << s[i] << " ";
	    //std::cout << std::endl;
	    devNumber=s[1];
	    serialNumber=s[2];
	    devStatus=s[3]*256+s[4];
	    telegramCounter=s[5];
	    scanCounter=s[6];
	    scanCounterWraps=scanCounterWrapper.wrap(serialNumber,scanCounter);
	    scanTime=s[7];   // Time in usec of zero index since power-up (at -14deg zero-index)  -- wraps around after 72 minutes!
	    scanTimeWraps=scanTimeWrapper.wrap(serialNumber,scanTime);
	    transmitTime=s[8];
	    digitalInputs=s[9]*256+s[10];
	    digitalOutputs=s[11]*256+s[12];
	    // s[13] reserved
	    scanFrequency=s[14]/100.0;
	    measurementFrequency=s[15]*100;
	    encoderFlag=s[16];
	    if (encoderFlag!=0) {
		encoderPosition=s[17];
		encoderSpeed=s[18];
		outputChannels=s[19];
	    } else {
		outputChannels=s[17];
	    }
	    dbg("SickFrame.SickFrame",1) << "Unit " << serialNumber << " read scan " << scanCounter << std::endl;
	}
    }

    catch(const SickConfigException & sick_exception) {
	printf("%s\n",sick_exception.what());
    }

    catch(const SickIOException & sick_exception) {
	printf("%s\n",sick_exception.what());
    }

    catch(const SickTimeoutException & sick_exception) {
	printf("%s\n",sick_exception.what());
    }

    catch(...) {
	fprintf(stderr,"An Error Occurred!\n");
	throw;
    }

    gettimeofday(&acquired,0);  // Time of acquisition
}

// Read the next frame from the given file descriptor
// Returns unit id of frame read
int SickFrame::read(FILE *fd, int version) {
    int cid;   // CID is unit number with 1-origin
    int nread;

    if (version==1) {
	if (EOF==(nread=fscanf(fd,"%d %d %ld %d %d %d ",&cid,&scanCounter,&acquired.tv_sec,&acquired.tv_usec,&nechoes,&num_measurements))) {
	    return -1;
	}
	if (nread!=6)  {
	    std::cerr << "Error scanning input file, read " << nread << " entries when 6 were expected" << std::endl;
	    return -1;
	}
	scanTime=acquired.tv_sec*1000000+acquired.tv_usec;
	transmitTime=scanTime+1000;  // Assume 1msec delay in transmitting
	devNumber=0;
	serialNumber=cid;
	devStatus=0;
	telegramCounter=scanCounter;
	digitalInputs=0;
	digitalOutputs=0;
	scanFrequency=50.0;
	measurementFrequency=scanFrequency*180*3;
    } else if (version==2) {
	int id;
	float originX, originY;

	// Read origin, rotation and store in frame (not in sickio)  (but currently ignored since current settings apply to loaded file)
	if (EOF==(nread=fscanf(fd,"T %d %f %f %lf ",&id, &originX, &originY, &coordinateRotation)))
	    return -1;
	if (nread!=4)  {
	    std::cerr << "Error scanning input file, read " << nread << " entries when 4 were expected" << std::endl;
	    return -1;
	}
	origin.setX(originX); origin.setY(originY);
	dbg("SickFrame.read",1) << "id=" << id << ", origin=[" << origin << "], crot=" << coordinateRotation << std::endl;
	if (EOF==(nread=fscanf(fd,"P %d %d %ld %d %d %d ",&cid,&scanCounter,&acquired.tv_sec,&acquired.tv_usec,&nechoes,&num_measurements))) {
	    return -1;
	}
	if (nread!=6)  {
	    std::cerr << "Error scanning input file, read " << nread << " entries when 6 were expected" << std::endl;
	    return -1;
	}
	if (EOF==(nread=fscanf(fd,"%d %d %d %d %d %d %d %d ",&devNumber,&serialNumber,&devStatus,&telegramCounter,&scanTime,& transmitTime,&digitalInputs,&digitalOutputs)))
	    return -1;
	if (nread!=8)  {
	    std::cerr << "Error scanning input file, read " << nread << " entries when 8 were expected" << std::endl;
	    return -1;
	}
	int tmp1, tmp2;
	if (EOF==(nread=fscanf(fd,"%d %d ",&tmp1,&tmp2)))
	    return -1;
	if (nread!=2)  {
	    std::cerr << "Error scanning input file, read " << nread << " entries when 2 were expected" << std::endl;
	    return -1;
	}
	scanFrequency=tmp1/100.0;
	measurementFrequency=tmp2*100;
    } else {
	std::cerr << "Bad version (" << version << ") for SickFrame::read()" << std::endl;
	assert(false);
    }
	
    scanCounterWraps=scanCounterWrapper.wrap(serialNumber,scanCounter);
    scanTimeWraps=scanTimeWrapper.wrap(serialNumber,scanTime);

    assert(nechoes>=1 && nechoes<=SickIO::MAXECHOES);
    assert(num_measurements>0 && num_measurements<=SickToolbox::SickLMS5xx::SICK_LMS_5XX_MAX_NUM_MEASUREMENTS);

    for (int e=0;e<nechoes;e++) {
	int echo;
	nread=fscanf(fd,"D%d ",&echo);
	if (nread==EOF) return -1;
	assert(echo>=0 && echo<nechoes);
	for (int i=0;i<num_measurements;i++) {
	    nread=fscanf(fd,"%d ",&range[e][i]);
	    if (nread==EOF) return -1;
	}
    }
    for (int e=0;e<nechoes;e++) {
	int echo;
	nread=fscanf(fd,"R%d ",&echo);
	if (nread==EOF) return -1;
	assert(echo>=0 && echo<nechoes);
	for (int i=0;i<num_measurements;i++) {
	    nread=fscanf(fd,"%d ",&reflect[e][i]);
	    if (nread==EOF) return -1;
	}
    }
    return cid;
}

void SickFrame::write(FILE *fd, int cid, int version) const {
    if (version==1)
	fprintf(fd,"%d %d %ld %d %d %d\n",cid,scanCounter,acquired.tv_sec,acquired.tv_usec,nechoes,num_measurements);
    else if (version==2) {
	// More complete data
	fprintf(fd,"P %d %d %ld %d %d %d ",cid,scanCounter,acquired.tv_sec,acquired.tv_usec,nechoes,num_measurements);
	fprintf(fd,"%d %d %d %d %d %d %d %d ",devNumber,serialNumber,devStatus,telegramCounter,scanTime, transmitTime,digitalInputs, digitalOutputs);
	fprintf(fd,"%d %d\n", (int)(scanFrequency*100+0.5), (int)(measurementFrequency/100+0.5));
    } else {
	std::cerr << "Bad version (" << version << ") for SickFrame::write()" << std::endl;
	assert(false);
    }
	
    for (int e=0;e<nechoes;e++) {
	fprintf(fd,"D%d ",e);
	for (unsigned int i=0;i<num_measurements;i++)
	    fprintf(fd,"%d ",range[e][i]);
	fprintf(fd,"\n");
    }
    for (int e=0;e<nechoes;e++) {
	fprintf(fd,"R%d ",e);
	for (unsigned int i=0;i<num_measurements;i++)
	    fprintf(fd,"%d ",reflect[e][i]);
	fprintf(fd,"\n");
    }
}


void SickFrame::overlayFrame(const SickFrame &frame) {
    if (num_measurements==0)  {
	dbg("SickIO.overlay",1) << "Live capture not started" << std::endl;
	return;
    }
    assert(num_measurements==frame.num_measurements);
    assert(nechoes==frame.nechoes);


    // Overlay data - take closest range of overlay data and current data
    int cnt=0;
    for (int e=0;e<nechoes;e++)
	for (int i=0;i<num_measurements;i++) {
	    if (frame.range[e][i]<range[e][i]-10) {
		cnt++;
		range[e][i]=frame.range[e][i];
		reflect[e][i]=frame.reflect[e][i];
	    }
	}
    dbg("SickIO.overlay",4) << "Overlaid " << cnt << " points." << std::endl;
    // valid is driven by real-time acquisition
}

