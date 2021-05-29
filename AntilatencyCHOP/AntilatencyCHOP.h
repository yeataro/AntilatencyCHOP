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

#include "CHOP_CPlusPlusBase.h"

#include <Antilatency.InterfaceContract.LibraryLoader.h>
#include <Antilatency.DeviceNetwork.h>
#include <Antilatency.StorageClient.h>
#include <Antilatency.Alt.Tracking.h>
//#include <string>


//newline

/*

This example file implements a class that does 2 different things depending on
if a CHOP is connected to the CPlusPlus CHOPs input or not.
The example is timesliced, which is the more complex way of working.

If an input is connected the node will output the same number of channels as the
input and divide the first 'N' samples in the input channel by 2. 'N' being the current
timeslice size. This is noteworthy because if the input isn't changing then the output
will look wierd since depending on the timeslice size some number of the first samples
of the input will get used.

If no input is connected then the node will output a smooth sine wave at 120hz.
*/


// To get more help about these functions, look at CHOP_CPlusPlusBase.h
class AntilatencyCHOP : public CHOP_CPlusPlusBase
{
public:
	AntilatencyCHOP(const OP_NodeInfo* info);
	virtual ~AntilatencyCHOP();

	virtual void		getGeneralInfo(CHOP_GeneralInfo*, const OP_Inputs*, void* ) override;
	virtual bool		getOutputInfo(CHOP_OutputInfo*, const OP_Inputs*, void*) override;
	virtual void		getChannelName(int32_t index, OP_String *name, const OP_Inputs*, void* reserved) override;

	virtual void		execute(CHOP_Output*,
								const OP_Inputs*,
								void* reserved) override;


	virtual int32_t		getNumInfoCHOPChans(void* reserved1) override;
	virtual void		getInfoCHOPChan(int index,
										OP_InfoCHOPChan* chan,
										void* reserved1) override;

	virtual bool		getInfoDATSize(OP_InfoDATSize* infoSize, void* resereved1) override;
	virtual void		getInfoDATEntries(int32_t index,
											int32_t nEntries,
											OP_InfoDATEntries* entries,
											void* reserved1) override;
	//
	virtual void		getErrorString(OP_String *error, void* reserved1) override;
	virtual void		getInfoPopupString(OP_String *popup, void* reserved1) override;
	virtual void		getWarningString(OP_String *warning, void *reserved1);
	//
	virtual void		setupParameters(OP_ParameterManager* manager, void *reserved1) override;
	virtual void		pulsePressed(const char* name, void* reserved1);
	
	//virtual void		GetTrackingData(Antilatency::DeviceNetwork::NodeHandle* node) override;
	virtual void		setupDevice();

	virtual void		updateDevice();

	virtual void		updateEnv();

	virtual void		GetTrackingNode();

	virtual void		RunTrackingTask();

	virtual void		GetTrackingData();


private:

	// We don't need to store this pointer, but we do for the example.
	// The OP_NodeInfo class store information about the node that's using
	// this instance of the class (like its name).
	const OP_NodeInfo*	myNodeInfo;

	// In this example this value will be incremented each time the execute()
	// function is called, then passes back to the CHOP 
	int32_t				myExecuteCount;
	double				myOffset;


	//int32_t				DeviceConnect

	const char*			myError;
	const char*			myPopup;
	const char*			myWarning;

	//std::string			 ADNversion;

	const char*		 ENVcode;

	//Flag
	uint32_t				updateId = 0;
	bool					ANTIexec = false;

	Antilatency::DeviceNetwork::INetwork deviceNetwork;
	Antilatency::Alt::Tracking::ILibrary altTrackingLibrary;
	Antilatency::Alt::Tracking::ITrackingCotask trackingCotask;
	Antilatency::Alt::Tracking::IEnvironment environment;
	Antilatency::Math::floatP3Q placement;
	Antilatency::DeviceNetwork::NodeHandle trackingNode;
	Antilatency::Math::float3  Pos;
	Antilatency::Math::floatQ Rotate;
	Antilatency::Alt::Tracking::State *RawState;
};
