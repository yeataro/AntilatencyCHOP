/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

//multithread
#include <thread>

#include "AntilatencyCHOP.h"

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <assert.h>

//newline

#include <vector>
//#include <cout>
using namespace std;

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <Windows.h>


//Returns the first idle alt tracker node just for demonstration purposes
void
AntilatencyCHOP::GetTrackingNode() {
    auto result = Antilatency::DeviceNetwork::NodeHandle::Null;

    auto cotaskConstructor = altTrackingLibrary.createTrackingCotaskConstructor();

    auto nodes = cotaskConstructor.findSupportedNodes(deviceNetwork);
    if (!nodes.empty()) {
        for (auto node : nodes) {
            if (deviceNetwork.nodeGetStatus(node) == Antilatency::DeviceNetwork::NodeStatus::Idle) {
                result = node;
				//TBD:get multiple node here
                break;
            }
        }
    }
	trackingNode = result;
   // return result;
}




//Run tracking task on node and print tracking data
void
AntilatencyCHOP::RunTrackingTask() {

	//std::cout << "RunTrackingTask" << std::endl;

	auto cotaskConstructor = altTrackingLibrary.createTrackingCotaskConstructor();

	//std::cout << "auto cotaskConstructor = altTrackingLibrary.createTrackingCotaskConstructor(); " << std::endl;

	trackingCotask = cotaskConstructor.startTask(deviceNetwork, trackingNode, environment);

	myError = nullptr;
	myWarning = nullptr;
	myPopup = nullptr;

	//std::cout << "trackingCotask = cotaskConstructor.startTask(deviceNetwork, node, environment);" << std::endl;


}

void
AntilatencyCHOP::GetTrackingData() {
	if (trackingCotask != nullptr && !trackingCotask.isTaskFinished()) {
		//Get raw tracker state
		//auto state = trackingCotask.getState(Antilatency::Alt::Tracking::Constants::DefaultAngularVelocityAvgTime);
		//std::cout << "Raw position x: " << state.pose.position.x << ", y: " << state.pose.position.y << ", z: " << state.pose.position.z << std::endl;

		//Get extrapolated tracker state with placement correction
		auto extrapolatedState = trackingCotask.getExtrapolatedState(placement, 0.06f);

		Pos = extrapolatedState.pose.position;
		Rotate = extrapolatedState.pose.rotation;
	}
	else {
		trackingCotask = {};
	}
	Yield();
}



//newline

// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

DLLEXPORT
void
FillCHOPPluginInfo(CHOP_PluginInfo *info)
{
	// Always set this to CHOPCPlusPlusAPIVersion.
	info->apiVersion = CHOPCPlusPlusAPIVersion;

	// The opType is the unique name for this CHOP. It must start with a 
	// capital A-Z character, and all the following characters must lower case
	// or numbers (a-z, 0-9)
	info->customOPInfo.opType->setString("AntilatencyCHOP");

	// The opLabel is the text that will show up in the OP Create Dialog
	info->customOPInfo.opLabel->setString("AntilatencyCHOP");

	// Information about the author of this OP
	info->customOPInfo.authorName->setString("Yea Chen");
	info->customOPInfo.authorEmail->setString("yeataro@email.com");

	// This CHOP can work with 0 inputs
	info->customOPInfo.minInputs = 0;

	// It can accept up to 1 input though, which changes it's behavior
	info->customOPInfo.maxInputs = 1;
}

DLLEXPORT
CHOP_CPlusPlusBase*
CreateCHOPInstance(const OP_NodeInfo* info)
{
	// Return a new instance of your class every time this is called.
	// It will be called once per CHOP that is using the .dll
	return new AntilatencyCHOP(info);
}

DLLEXPORT
void
DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
{
	// Delete the instance here, this will be called when
	// Touch is shutting down, when the CHOP using that instance is deleted, or
	// if the CHOP loads a different DLL
	delete (AntilatencyCHOP*)instance;


}

};


void
AntilatencyCHOP::setupDevice() 
{

	myError = nullptr;

	auto adnLibrary = Antilatency::InterfaceContract::getLibraryInterface<Antilatency::DeviceNetwork::ILibrary>("AntilatencyDeviceNetwork");
	auto antilatencyStorageClient = Antilatency::InterfaceContract::getLibraryInterface<Antilatency::StorageClient::ILibrary>("AntilatencyStorageClient");
	altTrackingLibrary = Antilatency::InterfaceContract::getLibraryInterface<Antilatency::Alt::Tracking::ILibrary>("AntilatencyAltTracking");

	if (adnLibrary == nullptr) {
		//std::cout << "Failed to load AntilatencyDeviceNetwork library" << std::endl;
		myError = "Failed to load AntilatencyDeviceNetwork library";
		return;
	}

	if (antilatencyStorageClient == nullptr) {
		//std::cout << "Failed to load AntilatencyStorageClient library" << std::endl;
		myError = "Failed to load AntilatencyStorageClient library";
		return;
	}

	if (altTrackingLibrary == nullptr) {
		//std::cout << "Failed to load AntilatencyAltTracking library" << std::endl;
		myError = "Failed to load AntilatencyAltTracking library";
		return;
	}

	adnLibrary.setLogLevel(Antilatency::DeviceNetwork::LogLevel::Info);

	//std::cout << "ADN version: " << adnLibrary.getVersion() << std::endl;

	//Alt socket USB device ID
	Antilatency::DeviceNetwork::UsbDeviceType antilatencyUsbDeviceType;
	antilatencyUsbDeviceType.pid = 0x0000;
	antilatencyUsbDeviceType.vid = Antilatency::DeviceNetwork::UsbVendorId::Antilatency;

	//Alt socket USB device ID (deprecated)
	Antilatency::DeviceNetwork::UsbDeviceType antilatencyUsbDeviceTypeLegacy;
	antilatencyUsbDeviceTypeLegacy.pid = 0x0000;
	antilatencyUsbDeviceTypeLegacy.vid = Antilatency::DeviceNetwork::UsbVendorId::AntilatencyLegacy;

	deviceNetwork = adnLibrary.createNetwork(std::vector<Antilatency::DeviceNetwork::UsbDeviceType>{antilatencyUsbDeviceType, antilatencyUsbDeviceTypeLegacy});

	//Antilatency::DeviceNetwork::NodeHandle trackingNode;
	updateEnv();
	ANTIexec = true;
}

void
AntilatencyCHOP::updateDevice( )
{
	auto newUpdateId = deviceNetwork.getUpdateId();
	if (updateId != newUpdateId) {
		updateId = newUpdateId;

		//std::cout << "Factory update id has been incremented, searching for available tracking node..." << std::endl;
		//myPopup = "Factory update id has been incremented, searching for available tracking node...";
		GetTrackingNode();
		//trackingNode = GetTrackingNode();
		if (trackingNode != Antilatency::DeviceNetwork::NodeHandle::Null) {
			//Found tracking node
			auto nodeSerialNo = deviceNetwork.nodeGetStringProperty(deviceNetwork.nodeGetParent(trackingNode), Antilatency::DeviceNetwork::Interop::Constants::HardwareSerialNumberKey);
			auto nodeTag = deviceNetwork.nodeGetStringProperty(deviceNetwork.nodeGetParent(trackingNode), "Tag");
			std::cout << "Tracking node found, serial number: " << nodeSerialNo << std::endl;
			std::cout << "Tag: " << nodeTag << std::endl;
			//char Msg[80] = "Tracking node found, serial number: ";
			//strcat(Msg, nodeSerialNo.c_str());
			//myPopup = Msg;
			
			//tbd:output to info
			RunTrackingTask();
			//return;
		}
		else {
			//std::cout << "Tracking node not found." << std::endl;
			myWarning = "Tracking node not found, searching for available tracking node...";
			//return;

		}
	}
}

void
AntilatencyCHOP::updateEnv()
{
	auto antilatencyStorageClient = Antilatency::InterfaceContract::getLibraryInterface<Antilatency::StorageClient::ILibrary>("AntilatencyStorageClient");
	auto environmentCode = antilatencyStorageClient.getLocalStorage().read("environment", "default");
	auto placementCode = antilatencyStorageClient.getLocalStorage().read("placement", "default");
	myWarning = nullptr;
	environment = altTrackingLibrary.createEnvironment(environmentCode);
	placement = altTrackingLibrary.createPlacement(placementCode);
	
	//NOT A GOOD WAY
	if (strcmp(ENVcode,"AA") > 0){

	try {
		environment = altTrackingLibrary.createEnvironment(ENVcode);
		std::cout << environment << std::endl;
		if (environment == nullptr) {
			myError = "Could not create environment!";
			return;
			//continue;
		}
	}
	catch (const std::exception &ex) 
	{
		myWarning = "Can't use this environment code, use the default.";
		//std::cout << "Can't use that environmentCode, using default." << std::endl;
	}
	//std::cout << ENVcode << std::endl;

	}
	else {
		myWarning = "The environment code is too short or none, use the default";
	}
	auto markers = environment.getMarkers();
	//std::cout << "Environment markers count: " << markers.size() << std::endl;
	//tbd:output to info
	MarkersSize = markers.size();
	//Markers = markers;
	for (auto i = 0; i < markers.size(); ++i) {
		std::cout << "Marker " << i << ": {" << markers[i].x << ", " << markers[i].y << ", " << markers[i].z << "}" << std::endl;
		//tbd:output to info
	}
}



AntilatencyCHOP::AntilatencyCHOP(const OP_NodeInfo* info) : myNodeInfo(info), myError(nullptr)
{
	myExecuteCount = 0;
	myOffset = 0.0;
	//ADNversion = nullptr;

}

AntilatencyCHOP::~AntilatencyCHOP()
{

}

void
AntilatencyCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
	// This will cause the node to cook every frame
	ginfo->cookEveryFrameIfAsked = true;

	// Note: To disable timeslicing you'll need to turn this off, as well as ensure that
	// getOutputInfo() returns true, and likely also set the info->numSamples to how many
	// samples you want to generate for this CHOP. Otherwise it'll take on length of the
	// input CHOP, which may be timesliced.
	bool Timeslice = inputs->getParDouble("Timeslice");
	ginfo->timeslice = Timeslice;

	ginfo->inputMatchIndex = 0;
}

bool
AntilatencyCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
	// If there is an input connected, we are going to match it's channel names etc
	// otherwise we'll specify our own.

	/*
	if (inputs->getNumInputs() > 0)
	{
		return false;
	}
	else
	{
		
	*/
		info->numChannels = 7;
		// Since we are outputting a timeslice, the system will dictate
		// the numSamples and startIndex of the CHOP data
		info->numSamples = 1;
		//info->startIndex = 0;

		// For illustration we are going to output 120hz data
		double Frequency = inputs->getParDouble("Frequency");
		info->sampleRate = Frequency;
		return true;
	//}
}

void
AntilatencyCHOP::getChannelName(int32_t index, OP_String *name, const OP_Inputs* inputs, void* reserved1)
{
	//Change way to input Envcode
	ENVcode = inputs->getParString("Envcode");
	//std::cout << ENVcode << std::endl;
	switch (index) {
	case 0:
		name->setString("tx");
		break;
	case 1:
		name->setString("ty");
		break;
	case 2:
		name->setString("tz");
		break;
	case 3:
		name->setString("qx");
		break;
	case 4:
		name->setString("qy");
		break;
	case 5:
		name->setString("qz");
		break;
	case 6:
		name->setString("qw");
		break;
	}
}

void
AntilatencyCHOP::execute(CHOP_Output* output,
							  const OP_Inputs* inputs,
							  void* reserved)
{
	myExecuteCount++;
	//uint32_t updateId = 0;

	if (!ANTIexec) {
		setupDevice();
	}

		if (trackingCotask != nullptr && !trackingCotask.isTaskFinished()) {
			GetTrackingData();
		}
		else
		{
			updateDevice();
		}



		for (int j = 0; j < output->numSamples; j++)
		{
			
			output->channels[0][j] = Pos.x;
			output->channels[1][j] = Pos.y;
			output->channels[2][j] = Pos.z;
			output->channels[3][j] = Rotate.x;
			output->channels[4][j] = Rotate.y;
			output->channels[5][j] = Rotate.z;
			output->channels[6][j] = Rotate.w;
		}
	
	double rate = inputs->getParDouble("Frequency", 0);	

}

int32_t
AntilatencyCHOP::getNumInfoCHOPChans(void * reserved1)
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the CHOP. In this example we are just going to send one channel.
	return 2;
}

void
AntilatencyCHOP::getInfoCHOPChan(int32_t index,
										OP_InfoCHOPChan* chan,
										void* reserved1)
{
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name->setString("executeCount");
		chan->value = (float)myExecuteCount;
	}

	if (index == 1)
	{
		chan->name->setString("offset");
		chan->value = (float)myOffset;
	}
}

bool		
AntilatencyCHOP::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = 3;
	infoSize->cols = MarkersSize+1;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
AntilatencyCHOP::getInfoDATEntries(int32_t index,
										int32_t nEntries,
										OP_InfoDATEntries* entries, 
										void* reserved1)
{
	char tempBuffer[4096];

	//TBD:dont get here
	auto markers = environment.getMarkers();
	MarkersSize = markers.size();

	if (index == 0){
		entries->values[0]->setString("mark:tx");

		for (auto i = 0; i < markers.size(); ++i) {
			#ifdef _WIN32
			sprintf_s(tempBuffer, "%f\t", markers[i].x);
			#else // macOS
			snprintf(tempBuffer, sizeof(tempBuffer), "%f\t", markers[i].x);
			#endif
			int j = i + 1;
			entries->values[j]->setString(tempBuffer);
		}
		
		
	}

	if (index == 1) {
		entries->values[0]->setString("mark:ty");

		for (auto i = 0; i < markers.size(); ++i) {
#ifdef _WIN32
			sprintf_s(tempBuffer, "%f\t", markers[i].y);
#else // macOS
			snprintf(tempBuffer, sizeof(tempBuffer), "%f\t", markers[i].y);
#endif
			int j = i + 1;
			entries->values[j]->setString(tempBuffer);
		}
	}

	if (index == 2) {
		entries->values[0]->setString("mark:tz");

		for (auto i = 0; i < markers.size(); ++i) {
#ifdef _WIN32
			sprintf_s(tempBuffer, "%f\t", markers[i].z);
#else // macOS
			snprintf(tempBuffer, sizeof(tempBuffer), "%f\t", markers[i].z);
#endif
			int j = i + 1;
			entries->values[j]->setString(tempBuffer);
		}

	}


	/*
	if (index == 0)
	{
		// Set the value for the first column
		entries->values[0]->setString("executeCount");

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%d", myExecuteCount);
#else // macOS
        snprintf(tempBuffer, sizeof(tempBuffer), "%d", myExecuteCount);
#endif
		entries->values[1]->setString(tempBuffer);
	}

	if (index == 1)
	{
		// Set the value for the first column
		entries->values[0]->setString("offset");

		// Set the value for the second column
#ifdef _WIN32
        sprintf_s(tempBuffer, "%g", myOffset);
#else // macOS
        snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString( tempBuffer);
	}
	*/
}




void
AntilatencyCHOP::getErrorString(OP_String *error, void* reserved1)
{
	error->setString(myError);
}

void
AntilatencyCHOP::getInfoPopupString(OP_String *popup, void* reserved1)
{
	popup->setString(myPopup);
}

void
AntilatencyCHOP::getWarningString(OP_String *warning, void *reserved1)
{
	warning->setString(myWarning);
}





void
AntilatencyCHOP::setupParameters(OP_ParameterManager* manager, void *reserved1)
{
	// Frequency
	{
		OP_NumericParameter	np;

		np.name = "Frequency";
		np.label = "Frequency";
		np.defaultValues[0] = 60.0;
		np.minSliders[0] = 1.0;
		np.maxSliders[0] = 2000.0;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	//Time Slice
	{
		OP_NumericParameter	tp;

		tp.name = "Timeslice";
		tp.label = "Time Slice";
		tp.defaultValues[0] =false;

		OP_ParAppendResult res = manager->appendToggle(tp);
		assert(res == OP_ParAppendResult::Success);
		
	}
	
	// Environment Code
	{
		OP_StringParameter	sp;

		sp.name = "Envcode";
		sp.label = "Environment Code";
		sp.defaultValue = nullptr;

		OP_ParAppendResult res = manager->appendString(sp);
		assert(res == OP_ParAppendResult::Success);
	}


		// pulse
	{
		OP_NumericParameter	np;

		np.name = "Initialize";
		np.label = "Initialize";
		
		OP_ParAppendResult res = manager->appendPulse(np);
		assert(res == OP_ParAppendResult::Success);
	}

}

void 
AntilatencyCHOP::pulsePressed(const char* name, void* reserved1)
{
	if (!strcmp(name, "Initialize"))
	{
		myExecuteCount = 0;
		trackingCotask = {};
		updateEnv();
		RunTrackingTask();
		//setupDevice();
	}
	//myExecuteCount = 100;
}



