#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <ladspa.h>

#ifndef __cplusplus
# if __GNUC__
#  define ON_LOAD_ROUTINE   static void __attribute__ ((constructor)) init()
#  define ON_UNLOAD_ROUTINE static void __attribute__ ((destructor))  fini()
# else
#  define ON_LOAD_ROUTINE   void _init()
#  define ON_UNLOAD_ROUTINE void _fini()
# endif
#else
#endif

#define SC_THRESH 0
#define SC_PARAM 1
#define SC_INPUT1 2
#define SC_OUTPUT1 3
#define SC_INPUT2 4
#define SC_OUTPUT2 5

#define pow10f( x ) expf(2.302585092994046f*(x))

typedef struct {

	LADSPA_Data* pThresh;
	LADSPA_Data* pParam;

	LADSPA_Data* pInput1;
	LADSPA_Data* pOutput1;
	LADSPA_Data* pInput2;  /* Stereo */
	LADSPA_Data* pOutput2; /* Stereo */

} PekiSoftClip;

static LADSPA_Handle instantiatePekiSoftClip(
	const LADSPA_Descriptor* Descriptor,
	unsigned long SampleRate
) {
	return malloc(sizeof(PekiSoftClip));
}

static void connectPortToPekiSoftClip(
	LADSPA_Handle Instance,
	unsigned long Port,
	LADSPA_Data* DataLocation
) {

	PekiSoftClip* sPekiSoftClip;

	sPekiSoftClip = (PekiSoftClip*)Instance;

	switch (Port) {

		case SC_THRESH:  sPekiSoftClip->pThresh  = DataLocation; break;
		case SC_PARAM:   sPekiSoftClip->pParam   = DataLocation; break;

		case SC_INPUT1:  sPekiSoftClip->pInput1  = DataLocation; break;
		case SC_OUTPUT1: sPekiSoftClip->pOutput1 = DataLocation; break;
		case SC_INPUT2:  sPekiSoftClip->pInput2  = DataLocation; break; /* Stereo */
		case SC_OUTPUT2: sPekiSoftClip->pOutput2 = DataLocation; break; /* Stereo */

	}

}

static void runMonoPekiSoftClip(
	LADSPA_Handle Instance,
	unsigned long SampleCount
) {

	LADSPA_Data* dInput;
	LADSPA_Data* dOutput;
	LADSPA_Data dThresh;
	LADSPA_Data dParam;
	PekiSoftClip* sPekiSoftClip;
	unsigned long sampleIndex;

	sPekiSoftClip = (PekiSoftClip*)Instance;

	dInput = sPekiSoftClip->pInput1;
	dOutput = sPekiSoftClip->pOutput1;

	dParam = *(sPekiSoftClip->pParam);

	dThresh = *(sPekiSoftClip->pThresh);
	dThresh = pow10f(dThresh / 20.0f);

	for (sampleIndex = 0; sampleIndex < SampleCount; ++sampleIndex) {
		dOutput[sampleIndex] = tanhf( dParam * dInput[sampleIndex] / dThresh ) * dThresh;
	}

}

static void runStereoPekiSoftClip(
	LADSPA_Handle Instance,
	unsigned long SampleCount
) {

	LADSPA_Data* dInput;
	LADSPA_Data* dOutput;
	LADSPA_Data dThresh;
	LADSPA_Data dParam;
	PekiSoftClip* sPekiSoftClip;
	unsigned long sampleIndex;

	sPekiSoftClip = (PekiSoftClip*)Instance;

	dThresh = *(sPekiSoftClip->pThresh);
	dThresh = pow10f(dThresh / 20.0f);

	dParam = *(sPekiSoftClip->pParam);

	dInput = sPekiSoftClip->pInput1;
	dOutput = sPekiSoftClip->pOutput1;
	for (sampleIndex = 0; sampleIndex < SampleCount; ++sampleIndex)
		dOutput[sampleIndex] = tanhf( dParam * dInput[sampleIndex] / dThresh ) * dThresh;

	dInput = sPekiSoftClip->pInput2;
	dOutput = sPekiSoftClip->pOutput2;
	for (sampleIndex = 0; sampleIndex < SampleCount; ++sampleIndex)
		dOutput[sampleIndex] = tanhf( dParam * dInput[sampleIndex] / dThresh ) * dThresh;

}

static void cleanupPekiSoftClip( LADSPA_Handle Instance ) { free(Instance); }

LADSPA_Descriptor* descmPekiSoftClip = NULL;
LADSPA_Descriptor* descsPekiSoftClip = NULL;

ON_LOAD_ROUTINE {

	char ** pnPekiSoftClip;
	LADSPA_PortDescriptor* pdPekiSoftClip;
	LADSPA_PortRangeHint* prhPekiSoftClip;

	descmPekiSoftClip = (LADSPA_Descriptor*)malloc(sizeof(LADSPA_Descriptor));
	descsPekiSoftClip = (LADSPA_Descriptor*)malloc(sizeof(LADSPA_Descriptor));

	if (descmPekiSoftClip) {

		descmPekiSoftClip->UniqueID = 2003;

		descmPekiSoftClip->Label = strdup("peki_softclip_mono");
		descmPekiSoftClip->Name = strdup("Peki's Soft Clipper (Mono)");
		descmPekiSoftClip->Maker = strdup("Petar Katić (bratpeki)");
		descmPekiSoftClip->Copyright = strdup("GPLv3");

		descmPekiSoftClip->PortCount = 4;
		descmPekiSoftClip->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;

		pdPekiSoftClip = (LADSPA_PortDescriptor*)calloc(4, sizeof(LADSPA_PortDescriptor));
		pdPekiSoftClip[SC_THRESH] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		pdPekiSoftClip[SC_PARAM] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		pdPekiSoftClip[SC_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdPekiSoftClip[SC_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		descmPekiSoftClip->PortDescriptors = (const LADSPA_PortDescriptor*)pdPekiSoftClip;

		pnPekiSoftClip = (char **)calloc(4, sizeof(char*));
		pnPekiSoftClip[SC_THRESH] = strdup("Threshold (dB)");
		pnPekiSoftClip[SC_PARAM] = strdup("Parameter");
		pnPekiSoftClip[SC_INPUT1] = strdup("Input");
		pnPekiSoftClip[SC_OUTPUT1] = strdup("Output");
		descmPekiSoftClip->PortNames = (const char **)pnPekiSoftClip;

		prhPekiSoftClip = ((LADSPA_PortRangeHint*)calloc(4, sizeof(LADSPA_PortRangeHint)));
		prhPekiSoftClip[SC_THRESH].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0
		);
		prhPekiSoftClip[SC_THRESH].LowerBound = -20;
		prhPekiSoftClip[SC_THRESH].UpperBound = 0;
		prhPekiSoftClip[SC_PARAM].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MIDDLE
		);
		prhPekiSoftClip[SC_PARAM].LowerBound = 2.5f;
		prhPekiSoftClip[SC_PARAM].UpperBound = 7.5f;
		prhPekiSoftClip[SC_INPUT1].HintDescriptor = 0;
		prhPekiSoftClip[SC_OUTPUT1].HintDescriptor = 0;
		descmPekiSoftClip->PortRangeHints = (const LADSPA_PortRangeHint*)prhPekiSoftClip;

		descmPekiSoftClip->instantiate = instantiatePekiSoftClip;
		descmPekiSoftClip->connect_port = connectPortToPekiSoftClip;
		descmPekiSoftClip->activate = NULL;
		descmPekiSoftClip->run = runMonoPekiSoftClip;
		descmPekiSoftClip->run_adding = NULL;
		descmPekiSoftClip->set_run_adding_gain = NULL;
		descmPekiSoftClip->deactivate = NULL;
		descmPekiSoftClip->cleanup = cleanupPekiSoftClip;
	}

	if (descsPekiSoftClip) {

		descsPekiSoftClip->UniqueID = 2004;

		descsPekiSoftClip->Label = strdup("peki_softclip_stereo");
		descsPekiSoftClip->Name = strdup("Peki's Soft Clipper (Stereo)");
		descsPekiSoftClip->Maker = strdup("Petar Katić (bratpeki)");
		descsPekiSoftClip->Copyright = strdup("GPLv3");

		descsPekiSoftClip->PortCount = 6;
		descsPekiSoftClip->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;

		pdPekiSoftClip = (LADSPA_PortDescriptor*)calloc(6, sizeof(LADSPA_PortDescriptor));
		pdPekiSoftClip[SC_THRESH] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		pdPekiSoftClip[SC_PARAM] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		pdPekiSoftClip[SC_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdPekiSoftClip[SC_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		pdPekiSoftClip[SC_INPUT2] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdPekiSoftClip[SC_OUTPUT2] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		descsPekiSoftClip->PortDescriptors = (const LADSPA_PortDescriptor*)pdPekiSoftClip;

		pnPekiSoftClip = (char **)calloc(6, sizeof(char*));
		pnPekiSoftClip[SC_THRESH] = strdup("Threshold (dB)");
		pnPekiSoftClip[SC_PARAM] = strdup("Parameter");
		pnPekiSoftClip[SC_INPUT1] = strdup("Input (Left)");
		pnPekiSoftClip[SC_OUTPUT1] = strdup("Output (Left)");
		pnPekiSoftClip[SC_INPUT2] = strdup("Input (Right)");
		pnPekiSoftClip[SC_OUTPUT2] = strdup("Output (Right)");
		descsPekiSoftClip->PortNames = (const char **)pnPekiSoftClip;

		prhPekiSoftClip = ((LADSPA_PortRangeHint*)calloc(6, sizeof(LADSPA_PortRangeHint)));
		prhPekiSoftClip[SC_THRESH].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0
		);
		prhPekiSoftClip[SC_THRESH].LowerBound = -20;
		prhPekiSoftClip[SC_THRESH].UpperBound = 0;
		prhPekiSoftClip[SC_PARAM].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MIDDLE
		);
		prhPekiSoftClip[SC_PARAM].LowerBound = 2.5f;
		prhPekiSoftClip[SC_PARAM].UpperBound = 7.5f;
		prhPekiSoftClip[SC_INPUT1].HintDescriptor = 0;
		prhPekiSoftClip[SC_OUTPUT1].HintDescriptor = 0;
		prhPekiSoftClip[SC_INPUT2].HintDescriptor = 0;
		prhPekiSoftClip[SC_OUTPUT2].HintDescriptor = 0;
		descsPekiSoftClip->PortRangeHints = (const LADSPA_PortRangeHint*)prhPekiSoftClip;

		descsPekiSoftClip->instantiate = instantiatePekiSoftClip;
		descsPekiSoftClip->connect_port = connectPortToPekiSoftClip;
		descsPekiSoftClip->activate = NULL;
		descsPekiSoftClip->run = runStereoPekiSoftClip;
		descsPekiSoftClip->run_adding = NULL;
		descsPekiSoftClip->set_run_adding_gain = NULL;
		descsPekiSoftClip->deactivate = NULL;
		descsPekiSoftClip->cleanup = cleanupPekiSoftClip;

	}

}

static void deleteDescriptor(LADSPA_Descriptor* descPekiSoftClip) {

	unsigned long lIndex;

	if (descPekiSoftClip) {

		free((char*)descPekiSoftClip->Label);
		free((char*)descPekiSoftClip->Name);
		free((char*)descPekiSoftClip->Maker);
		free((char*)descPekiSoftClip->Copyright);
		free((LADSPA_PortDescriptor*)descPekiSoftClip->PortDescriptors);

		for (lIndex = 0; lIndex < descPekiSoftClip->PortCount; lIndex++)
			free((char*)(descPekiSoftClip->PortNames[lIndex]));

		free((char**)descPekiSoftClip->PortNames);
		free((LADSPA_PortRangeHint*)descPekiSoftClip->PortRangeHints);
		free(descPekiSoftClip);

	}

}

ON_UNLOAD_ROUTINE {
	deleteDescriptor(descmPekiSoftClip);
	deleteDescriptor(descsPekiSoftClip);
}

const LADSPA_Descriptor* ladspa_descriptor(unsigned long Index) {

	switch (Index) {
		case 0: return descmPekiSoftClip;
		case 1: return descsPekiSoftClip;
		default: return NULL;
	}

}
