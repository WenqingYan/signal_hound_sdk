#include "sm_api.h"

#include <vector>

// SM200B/SM435B only.
// This example demonstrates setting up a single video triggered I/Q capture
//  using the 160MHz I/Q capture capabilities of the SM200B/SM435B.

void sm_example_segmented_iq_video_simple()
{
    // Number of I/Q samples in capture
    const int PRETRIGGER_LEN = 16384;
    const int POSTTRIGGER_LEN = 16384;
    const int CAPTURE_LEN = PRETRIGGER_LEN + POSTTRIGGER_LEN;

	int handle = -1;
    SmStatus status = smOpenDevice(&handle);

    if(status != smNoError) {
        // Unable to open device
        const char *errStr = smGetErrorString(status);
        return;
    }

    // Verify device has segmented I/Q capture capability
    SmDeviceType deviceType;
    smGetDeviceInfo(handle, &deviceType, 0);
    if(deviceType != smDeviceTypeSM200B && deviceType != smDeviceTypeSM435B) {
        // Invalid device type
        smCloseDevice(handle);
        return;
    }

    // Set device reference level, maximum expected input signal
    status = smSetRefLevel(handle, 0.0);

    // Configure the 160MHz capture
    status = smSetSegIQDataType(handle, smDataType32fc);
    status = smSetSegIQCenterFreq(handle, 2.45e9);
    // Setup an rising edge video trigger at -40 dBm
    status = smSetSegIQVideoTrigger(handle, -40.0, smTriggerEdgeRising);
    // Setup a single segment capture where the video trigger position
    // is at 50% (equal number of samples before and after the trigger)
    // 2 second timeout period.
    status = smSetSegIQSegmentCount(handle, 1);
    status = smSetSegIQSegment(handle, 0, smTriggerTypeVideo, PRETRIGGER_LEN, POSTTRIGGER_LEN, 2.0);

    status = smConfigure(handle, smModeIQSegmentedCapture);
    if(status != smNoError) {
        // Unable to configure device
        const char *errStr = smGetErrorString(status);
        return;
    }

    // 2 floats for each I/Q sample
	std::vector<float> buf(CAPTURE_LEN * 2);
    int64_t nsSinceEpoch = 0;
    SmBool timedOut;

    // Start the measurement, the analyzer will begin looking for a video
    //   trigger at this point.
    status = smSegIQCaptureStart(handle, 0);

    // Notify some other device to transmit/trigger if necessary.

    // Block until complete
    status = smSegIQCaptureWait(handle, 0);
    // Did the capture timeout?
    status = smSegIQCaptureTimeout(handle, 0, 0, &timedOut);
    // Read the timestamp
    status = smSegIQCaptureTime(handle, 0, 0, &nsSinceEpoch);
    // Read the I/Q data
    status = smSegIQCaptureRead(handle, 0, 0, (float*)buf.data(), 0, (int)buf.size() / 2);
    status = smSegIQCaptureFinish(handle, 0);

    smAbort(handle);
    smCloseDevice(handle);
}
