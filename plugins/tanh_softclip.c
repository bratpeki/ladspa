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

#define POW10F( x ) expf(2.302585092994046f*(x))

typedef struct {

	LADSPA_Data* pThresh;
	LADSPA_Data* pParam;

	LADSPA_Data* pInput1;
	LADSPA_Data* pOutput1;
	LADSPA_Data* pInput2;  /* Stereo */
	LADSPA_Data* pOutput2; /* Stereo */

} PekiTanhSoftClip;

static LADSPA_Handle instantiatePekiTanhSoftClip(
	const LADSPA_Descriptor* Descriptor,
	unsigned long SampleRate
) {
	return malloc(sizeof(PekiTanhSoftClip));
}

static void connectPortToPekiTanhSoftClip(
	LADSPA_Handle Instance,
	unsigned long Port,
	LADSPA_Data* DataLocation
) {

	PekiTanhSoftClip* sPekiTanhSoftClip;

	sPekiTanhSoftClip = (PekiTanhSoftClip*)Instance;

	switch (Port) {

		case SC_THRESH:  sPekiTanhSoftClip->pThresh  = DataLocation; break;
		case SC_PARAM:   sPekiTanhSoftClip->pParam   = DataLocation; break;

		case SC_INPUT1:  sPekiTanhSoftClip->pInput1  = DataLocation; break;
		case SC_OUTPUT1: sPekiTanhSoftClip->pOutput1 = DataLocation; break;
		case SC_INPUT2:  sPekiTanhSoftClip->pInput2  = DataLocation; break; /* Stereo */
		case SC_OUTPUT2: sPekiTanhSoftClip->pOutput2 = DataLocation; break; /* Stereo */

	}

}

static void runMonoPekiTanhSoftClip(
	LADSPA_Handle Instance,
	unsigned long SampleCount
) {

	LADSPA_Data* dInput;
	LADSPA_Data* dOutput;
	LADSPA_Data dThresh;
	LADSPA_Data dParam;
	PekiTanhSoftClip* sPekiTanhSoftClip;
	unsigned long sampleIndex;

	sPekiTanhSoftClip = (PekiTanhSoftClip*)Instance;

	dInput = sPekiTanhSoftClip->pInput1;
	dOutput = sPekiTanhSoftClip->pOutput1;

	dParam = *(sPekiTanhSoftClip->pParam);

	dThresh = *(sPekiTanhSoftClip->pThresh);
	dThresh = POW10F(dThresh / 20.0f);

	for (sampleIndex = 0; sampleIndex < SampleCount; ++sampleIndex) {
		dOutput[sampleIndex] = tanhf( dParam * dInput[sampleIndex] / dThresh ) * dThresh;
	}

}

static void runStereoPekiTanhSoftClip(
	LADSPA_Handle Instance,
	unsigned long SampleCount
) {

	LADSPA_Data* dInput;
	LADSPA_Data* dOutput;
	LADSPA_Data dThresh;
	LADSPA_Data dParam;
	PekiTanhSoftClip* sPekiTanhSoftClip;
	unsigned long sampleIndex;

	sPekiTanhSoftClip = (PekiTanhSoftClip*)Instance;

	dThresh = *(sPekiTanhSoftClip->pThresh);
	dThresh = POW10F(dThresh / 20.0f);

	dParam = *(sPekiTanhSoftClip->pParam);

	dInput = sPekiTanhSoftClip->pInput1;
	dOutput = sPekiTanhSoftClip->pOutput1;
	for (sampleIndex = 0; sampleIndex < SampleCount; ++sampleIndex)
		dOutput[sampleIndex] = tanhf( dParam * dInput[sampleIndex] / dThresh ) * dThresh;

	dInput = sPekiTanhSoftClip->pInput2;
	dOutput = sPekiTanhSoftClip->pOutput2;
	for (sampleIndex = 0; sampleIndex < SampleCount; ++sampleIndex)
		dOutput[sampleIndex] = tanhf( dParam * dInput[sampleIndex] / dThresh ) * dThresh;

}

static void cleanupPekiTanhSoftClip( LADSPA_Handle Instance ) { free(Instance); }

LADSPA_Descriptor* descmPekiTanhSoftClip = NULL;
LADSPA_Descriptor* descsPekiTanhSoftClip = NULL;

ON_LOAD_ROUTINE {

	char ** pnPekiTanhSoftClip;
	LADSPA_PortDescriptor* pdPekiTanhSoftClip;
	LADSPA_PortRangeHint* prhPekiTanhSoftClip;

	descmPekiTanhSoftClip = (LADSPA_Descriptor*)malloc(sizeof(LADSPA_Descriptor));
	descsPekiTanhSoftClip = (LADSPA_Descriptor*)malloc(sizeof(LADSPA_Descriptor));

	if (descmPekiTanhSoftClip) {

		descmPekiTanhSoftClip->UniqueID = 2003;

		descmPekiTanhSoftClip->Label = strdup("peki_tanh_softclip_mono");
		descmPekiTanhSoftClip->Name = strdup("Peki's Tanh Soft Clipper (Mono)");
		descmPekiTanhSoftClip->Maker = strdup("Petar Katić (bratpeki)");
		descmPekiTanhSoftClip->Copyright = strdup("GPLv3");

		descmPekiTanhSoftClip->PortCount = 4;
		descmPekiTanhSoftClip->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;

		pdPekiTanhSoftClip = (LADSPA_PortDescriptor*)calloc(4, sizeof(LADSPA_PortDescriptor));
		pdPekiTanhSoftClip[SC_THRESH] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		pdPekiTanhSoftClip[SC_PARAM] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		pdPekiTanhSoftClip[SC_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdPekiTanhSoftClip[SC_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		descmPekiTanhSoftClip->PortDescriptors = (const LADSPA_PortDescriptor*)pdPekiTanhSoftClip;

		pnPekiTanhSoftClip = (char **)calloc(4, sizeof(char*));
		pnPekiTanhSoftClip[SC_THRESH] = strdup("Threshold (dB)");
		pnPekiTanhSoftClip[SC_PARAM] = strdup("Parameter");
		pnPekiTanhSoftClip[SC_INPUT1] = strdup("Input");
		pnPekiTanhSoftClip[SC_OUTPUT1] = strdup("Output");
		descmPekiTanhSoftClip->PortNames = (const char **)pnPekiTanhSoftClip;

		prhPekiTanhSoftClip = ((LADSPA_PortRangeHint*)calloc(4, sizeof(LADSPA_PortRangeHint)));
		prhPekiTanhSoftClip[SC_THRESH].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0
		);
		prhPekiTanhSoftClip[SC_THRESH].LowerBound = -20;
		prhPekiTanhSoftClip[SC_THRESH].UpperBound = 0;
		prhPekiTanhSoftClip[SC_PARAM].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MIDDLE
		);
		prhPekiTanhSoftClip[SC_PARAM].LowerBound = 2.5f;
		prhPekiTanhSoftClip[SC_PARAM].UpperBound = 7.5f;
		prhPekiTanhSoftClip[SC_INPUT1].HintDescriptor = 0;
		prhPekiTanhSoftClip[SC_OUTPUT1].HintDescriptor = 0;
		descmPekiTanhSoftClip->PortRangeHints = (const LADSPA_PortRangeHint*)prhPekiTanhSoftClip;

		descmPekiTanhSoftClip->instantiate = instantiatePekiTanhSoftClip;
		descmPekiTanhSoftClip->connect_port = connectPortToPekiTanhSoftClip;
		descmPekiTanhSoftClip->activate = NULL;
		descmPekiTanhSoftClip->run = runMonoPekiTanhSoftClip;
		descmPekiTanhSoftClip->run_adding = NULL;
		descmPekiTanhSoftClip->set_run_adding_gain = NULL;
		descmPekiTanhSoftClip->deactivate = NULL;
		descmPekiTanhSoftClip->cleanup = cleanupPekiTanhSoftClip;
	}

	if (descsPekiTanhSoftClip) {

		descsPekiTanhSoftClip->UniqueID = 2004;

		descsPekiTanhSoftClip->Label = strdup("peki_tanh_softclip_stereo");
		descsPekiTanhSoftClip->Name = strdup("Peki's Tanh Soft Clipper (Stereo)");
		descsPekiTanhSoftClip->Maker = strdup("Petar Katić (bratpeki)");
		descsPekiTanhSoftClip->Copyright = strdup("GPLv3");

		descsPekiTanhSoftClip->PortCount = 6;
		descsPekiTanhSoftClip->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;

		pdPekiTanhSoftClip = (LADSPA_PortDescriptor*)calloc(6, sizeof(LADSPA_PortDescriptor));
		pdPekiTanhSoftClip[SC_THRESH] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		pdPekiTanhSoftClip[SC_PARAM] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		pdPekiTanhSoftClip[SC_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdPekiTanhSoftClip[SC_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		pdPekiTanhSoftClip[SC_INPUT2] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdPekiTanhSoftClip[SC_OUTPUT2] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		descsPekiTanhSoftClip->PortDescriptors = (const LADSPA_PortDescriptor*)pdPekiTanhSoftClip;

		pnPekiTanhSoftClip = (char **)calloc(6, sizeof(char*));
		pnPekiTanhSoftClip[SC_THRESH] = strdup("Threshold (dB)");
		pnPekiTanhSoftClip[SC_PARAM] = strdup("Parameter");
		pnPekiTanhSoftClip[SC_INPUT1] = strdup("Input (Left)");
		pnPekiTanhSoftClip[SC_OUTPUT1] = strdup("Output (Left)");
		pnPekiTanhSoftClip[SC_INPUT2] = strdup("Input (Right)");
		pnPekiTanhSoftClip[SC_OUTPUT2] = strdup("Output (Right)");
		descsPekiTanhSoftClip->PortNames = (const char **)pnPekiTanhSoftClip;

		prhPekiTanhSoftClip = ((LADSPA_PortRangeHint*)calloc(6, sizeof(LADSPA_PortRangeHint)));
		prhPekiTanhSoftClip[SC_THRESH].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0
		);
		prhPekiTanhSoftClip[SC_THRESH].LowerBound = -20;
		prhPekiTanhSoftClip[SC_THRESH].UpperBound = 0;
		prhPekiTanhSoftClip[SC_PARAM].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MIDDLE
		);
		prhPekiTanhSoftClip[SC_PARAM].LowerBound = 2.5f;
		prhPekiTanhSoftClip[SC_PARAM].UpperBound = 7.5f;
		prhPekiTanhSoftClip[SC_INPUT1].HintDescriptor = 0;
		prhPekiTanhSoftClip[SC_OUTPUT1].HintDescriptor = 0;
		prhPekiTanhSoftClip[SC_INPUT2].HintDescriptor = 0;
		prhPekiTanhSoftClip[SC_OUTPUT2].HintDescriptor = 0;
		descsPekiTanhSoftClip->PortRangeHints = (const LADSPA_PortRangeHint*)prhPekiTanhSoftClip;

		descsPekiTanhSoftClip->instantiate = instantiatePekiTanhSoftClip;
		descsPekiTanhSoftClip->connect_port = connectPortToPekiTanhSoftClip;
		descsPekiTanhSoftClip->activate = NULL;
		descsPekiTanhSoftClip->run = runStereoPekiTanhSoftClip;
		descsPekiTanhSoftClip->run_adding = NULL;
		descsPekiTanhSoftClip->set_run_adding_gain = NULL;
		descsPekiTanhSoftClip->deactivate = NULL;
		descsPekiTanhSoftClip->cleanup = cleanupPekiTanhSoftClip;

	}

}

static void deleteDescriptor(LADSPA_Descriptor* descPekiTanhSoftClip) {

	unsigned long lIndex;

	if (descPekiTanhSoftClip) {

		free((char*)descPekiTanhSoftClip->Label);
		free((char*)descPekiTanhSoftClip->Name);
		free((char*)descPekiTanhSoftClip->Maker);
		free((char*)descPekiTanhSoftClip->Copyright);
		free((LADSPA_PortDescriptor*)descPekiTanhSoftClip->PortDescriptors);

		for (lIndex = 0; lIndex < descPekiTanhSoftClip->PortCount; lIndex++)
			free((char*)(descPekiTanhSoftClip->PortNames[lIndex]));

		free((char**)descPekiTanhSoftClip->PortNames);
		free((LADSPA_PortRangeHint*)descPekiTanhSoftClip->PortRangeHints);
		free(descPekiTanhSoftClip);

	}

}

ON_UNLOAD_ROUTINE {
	deleteDescriptor(descmPekiTanhSoftClip);
	deleteDescriptor(descsPekiTanhSoftClip);
}

const LADSPA_Descriptor* ladspa_descriptor(unsigned long Index) {

	switch (Index) {
		case 0: return descmPekiTanhSoftClip;
		case 1: return descsPekiTanhSoftClip;
		default: return NULL;
	}

}
